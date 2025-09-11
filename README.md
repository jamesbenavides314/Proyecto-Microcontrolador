#include "p18f4550.inc"

; CONFIGURACIÓN DEL MICROCONTROLADOR
CONFIG  PLLDIV = 1            ; No prescaler del PLL
CONFIG  CPUDIV = OSC1_PLL2    ; Postscaler del sistema
CONFIG  USBDIV = 2            ; División de reloj USB
CONFIG  FOSC = INTOSCIO_EC    ; Oscilador interno
CONFIG  WDT = OFF             ; Watchdog Timer deshabilitado
CONFIG MCLRE = OFF        ; MCLARE deshabilitado, se usa como entrada digital
CONFIG  LVP = OFF             ; Low Voltage Programming deshabilitado

; VARIABLES EN MEMORIA
Contador1 EQU 0x20
Contador2 EQU 0x21
Contador3 EQU 0x22

ORG 0x00
    GOTO inicio

inicio:
    MOVLW 0x72          ; Configura oscilador interno a 8MHz
    MOVWF OSCCON        

    
    CLRF TRISB          ; Configurar PORTB como salida
    
    CLRF LATB           ; Apagar todos los LEDs

loop:
    BSF LATB, 0         ; Encender LED en RB0
    CALL retraso3s      ; Mantener encendido por 3s
    BCF LATB, 0         ; Apagar LED en RB0

    BSF LATB, 1         ; Encender LED en RB1
    CALL retraso3s      ; Mantener encendido por 3s
    BCF LATB, 1         ; Apagar LED en RB1

    BSF LATB, 2         ; Encender LED en RB2
    CALL retraso3s      ; Mantener encendido por 3s
    BCF LATB, 2         ; Apagar LED en RB2

    GOTO loop           ; Repetir en bucle

; RETARDO DE 3 SEGUNDOS
retraso3s:
    MOVLW D'30'         
    MOVWF Contador1
bucle1:
    CALL retraso100ms
    DECFSZ Contador1, F 
    GOTO bucle1
    RETURN 

; RETARDO DE 100ms (BASADO EN 8MHz)
retraso100ms:
    MOVLW D'250'         
    MOVWF Contador2
bucle2:
    MOVLW D'250'
    MOVWF Contador3
bucle3:
    NOP
    NOP
    DECFSZ Contador3, F
    GOTO bucle3
    DECFSZ Contador2, F
    GOTO bucle2
    RETURN

END
