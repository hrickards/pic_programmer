;; Author: Harry Rickards
;; Make output lEDs on port B blink

; Compiler directives
		ERRORLEVEL	-302			; ignore operand not in bank 0 messages
		LIST		p=16f627a		; select processor

; Configuration
		INCLUDE		"p16f627a.inc"	; include config code for processor
		__CONFIG(_LVP_OFF & _INTOSC_OSC_NOCLKOUT) ; disable low-voltage programming and use internal clock

; Variable definitions
		UDATA
sPORTB	RES			1				; shadow copy of PORTB
DC1		RES			1				; delay loop counters
DC2		RES			1				;

; Main program
RESET: 
		CODE 0x0000					; Reset vector

START:
		MOVLW		0x07			; loads literal val 7 into w
		MOVWF		CMCON			; comparator input & output multiplexers
		BANKSEL		TRISA			; select tristate A
		MOVLW		0xFF			; load num ff into w
		MOVWF		TRISA			; make port A input by writing w to TRISA
		CLRF		TRISB			; clear reg B to make port B output
		BANKSEL		PORTA			;

		MOVLW		b'10101010'		; initial state of LEDs
		MOVWF		sPORTB
		MOVWF		PORTB

; Main loop that flashes LED
MAIN:
		MOVF		PORTB, W		; get shadow copy of PORTB
		XORLW		0xFF			; toggle bits corresponding to all LEDs
		MOVWF		sPORTB			; in shadow register
		MOVWF		PORTB			; and write to B

		; delay
		MOVLW		.255			; run outer loop 256 times
		MOVWF		DC2
		CLRF		DC1				; run inner looop 256 (from clearing) times
INNER:	NOP							; inner loop = 256*4 -1 = 1023 cycles
		DECFSZ		DC1, F			; decrement counter & skip next if zero
		GOTO		INNER
		DECFSZ		DC2, F			; decrement counter & skip next if zero
		GOTO		INNER

		GOTO		MAIN			; repeat forever

		END							; end program
