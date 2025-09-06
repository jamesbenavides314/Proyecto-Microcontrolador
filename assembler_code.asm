; Configuración de bits de configuración (Fuses)
CONFIG  FOSC = INTOSCIO_EC   ; Usa el oscilador interno a 8 MHz
CONFIG  WDT = OFF            ; Deshabilitar el Watchdog Timer
CONFIG  LVP = OFF            ; Deshabilitar la programación en bajo voltaje
CONFIG  PBADEN = OFF         ; Configurar los pines de PORTB como digitales
CONFIG  MCLRE = OFF          ; Pin MCLR deshabilitado
    
#include <xc.inc>

; Sección para el vector de reinicio
PSECT  resetVec, class=CODE, reloc=2, abs
ORG 0X00
    GOTO Inicio
    

; Sección de código principal
PSECT  main_code, class=CODE, reloc=2  

Inicio:
    MOVLW   0b11110110  ;Poner 8MHz en el oscilador
    MOVWF   OSCCON
    
    CLRF    TRISB           ; Configurar PORTB como salida (0 = salida, 1 = entrada)
    CLRF    LATB            ; Apagar todos los pines de PORTB (LED apagado inicialmente)

Loop:
    BSF     LATB,0          ;Enciende el led
    CALL    Retardo_5       ;5s encendido
    
    BCF     LATB,0          ;Apaga el led
    CALL    Retardo_2       ;2s encendido
    
    GOTO    Loop            ; Repetir el proceso de parpadeo en bucle infinito

Retardo_5:
    MOVLW   0b11111010         ; Función para el retraso de 5000 ms
    MOVWF   Rcount1
    MOVLW   0b11011100
    MOVWF   Rcount2
    MOVLW   0b00101101
    MOVWF   Rcount3
    CALL    Rloop
    RETURN
 
Retardo_2:
    MOVLW   0b11111010         ; Función para el retraso de 2000 ms
    MOVWF   Rcount1
    MOVLW   0b11011100
    MOVWF   Rcount2
    MOVLW   0b00010010
    MOVWF   Rcount3
    CALL    Rloop
    RETURN  
    
Rloop:
    MOVF    Rcount3, W
    MOVWF   C3    
Loop3:
    MOVF    Rcount2, W
    MOVWF   C2
Loop2:
    MOVF    Rcount1, W
    MOVWF   C1 
Loop1:
    DECFSZ  C1,F
    GOTO    Loop1
    
    DECFSZ  C2,F
    GOTO    Loop2
    
    DECFSZ  C3,F
    GOTO    Loop3
    
    RETURN
   
PSECT udata                     ; Sección de datos sin inicializar (variables en RAM)
C1:                DS 1         
C2:                DS 1         ; Reserva 1 byte de memoria para el contador interno
C3:                DS 1
Rcount1:           DS 1
Rcount2:           DS 1
Rcount3:           DS 1
    
END
