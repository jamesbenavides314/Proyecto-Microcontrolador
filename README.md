PROCESSOR 18F4550
#include <xc.inc>

; CONFIGURACIÓN DEL MICROCONTROLADOR
CONFIG  PLLDIV = 1            ; No prescaler del PLL
CONFIG  CPUDIV = OSC1_PLL2    ; Postscaler del sistema  
CONFIG  USBDIV = 2            ; División de reloj USB
CONFIG  FOSC = INTOSCIO_EC    ; Oscilador interno
CONFIG  WDT = OFF             ; Watchdog Timer deshabilitado
CONFIG  MCLRE = OFF           ; MCLARE deshabilitado, se usa como entrada digital
CONFIG  LVP = OFF             ; Low Voltage Programming deshabilitado

; VARIABLES EN MEMORIA
PSECT udata_acs
Contador1:      DS 1          ; 1 byte
Contador2:      DS 1          ; 1 byte  
Contador3:      DS 1          ; 1 byte
ContadorSecuencia: DS 1       ; 1 byte

PSECT code
ORG 0x00
    GOTO inicio

inicio:
    MOVLW 0b11110110          ; Configura oscilador interno a 8MHz
    MOVWF OSCCON        
    
    CLRF TRISB          ; Configurar PORTB como salida
    CLRF LATB           ; Apagar todos los LEDs
    CLRF ContadorSecuencia ; Inicializar contador de secuencia

loop_principal:
    ; Verificar qué secuencia ejecutar
    MOVF ContadorSecuencia, W
    XORLW 0
    BZ secuencia1
    MOVF ContadorSecuencia, W
    XORLW 1
    BZ secuencia2
    GOTO secuencia3

secuencia1:
    ; Secuencia 1: Encendido secuencial (0->1->2->3)
    MOVLW 0b00000001    ; RB0
    CALL ejecutar_led
    MOVLW 0b00000010    ; RB1
    CALL ejecutar_led
    MOVLW 0b00000100    ; RB2
    CALL ejecutar_led
    MOVLW 0b00001000    ; RB3
    CALL ejecutar_led
    GOTO cambiar_secuencia

secuencia2:
    ; Secuencia 2: Encendido desde extremos hacia centro (0y3->1y2)
    MOVLW 0b00001001    ; RB0 y RB3
    CALL ejecutar_led
    MOVLW 0b00000110    ; RB1 y RB2
    CALL ejecutar_led
    GOTO cambiar_secuencia

secuencia3:
    ; Secuencia 3: Parpadeo alterno (0y2, luego 1y3)
    MOVLW 0b00000101    ; RB0 y RB2
    CALL ejecutar_led
    MOVLW 0b00001010    ; RB1 y RB3
    CALL ejecutar_led

cambiar_secuencia:
    ; Cambiar a la siguiente secuencia
    INCF ContadorSecuencia, F
    MOVF ContadorSecuencia, W
    XORLW 3             ; Si llegamos a 3, volver a 0
    BNZ loop_principal
    CLRF ContadorSecuencia
    GOTO loop_principal

; SUBRUTINA PARA EJECUTAR PATRÓN DE LED
ejecutar_led:
    MOVWF LATB          ; Encender LEDs según patrón
    CALL retraso5s      ; Mantener encendido por 5 segundos
    CLRF LATB           ; Apagar todos los LEDs
    CALL retraso2s      ; Mantener apagado por 2 segundos
    RETURN

; RETARDO DE 5 SEGUNDOS
retraso5s:
    MOVLW 50            ; 50 x 100ms = 5000ms
    MOVWF Contador1
bucle5s:
    CALL retraso100ms
    DECFSZ Contador1, F 
    GOTO bucle5s
    RETURN 

; RETARDO DE 2 SEGUNDOS
retraso2s:
    MOVLW 20            ; 20 x 100ms = 2000ms
    MOVWF Contador1
bucle2s:
    CALL retraso100ms
    DECFSZ Contador1, F 
    GOTO bucle2s
    RETURN 

; RETARDO DE 100ms (BASADO EN 8MHz)
retraso100ms:
    MOVLW 200           ; Ajustado para mejor precisión
    MOVWF Contador2
bucle_100ms_ext:
    MOVLW 200
    MOVWF Contador3
bucle_100ms_int:
    NOP
    NOP
    NOP
    NOP
    DECFSZ Contador3, F
    GOTO bucle_100ms_int
    DECFSZ Contador2, F
    GOTO bucle_100ms_ext
    RETURN

END
