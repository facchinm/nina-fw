/*
 * updi_io.cpp
 *
 * Created: 18-11-2017 10:36:54
 *  Author: JMR_2
 */ 

// Includes
#include "updi_io.h"
#include "esp32-hal-timer.h"
#include "Arduino.h"

#define BIT_RATE 225000U // (max 225000 min 160000)
#define F_CPU 	 80000000UL
#define BIT_TIME (F_CPU/BIT_RATE)

hw_timer_t * timer = NULL;
static volatile int pinStatus = HIGH;

// Functions
/* Sends regular characters through the UPDI link */
int UPDI_io::put(char c) {
	/* Wait for end of stop bits */
	wait_for_bit();
	//start_timer();
	/* Enable TX output */
	pinMode(22, OUTPUT);
	setup_bit_low();
	/* Calculate parity */
	uint8_t parity;		//get_parity(c);
	parity = 0;
	/* If we can be sure an overflow has happened by now due to instruction latency, */
	/* no more wait is needed and we only need to clear overflow flag */
	//wait_for_bit();
	/* Send data bits and calculate parity */
	for (uint8_t mask = 1; mask; mask <<= 1) {
		// Check bit, transmit high or low bit accordingly and update parity bit
		parity = (c & mask) ? (setup_bit_high(), ~parity) : (setup_bit_low(), parity);
		wait_for_bit();
	}
	/* Send parity bit */
	parity ? setup_bit_high() : setup_bit_low();
	wait_for_bit();
	/* Send stop bits */
	setup_bit_high();
	wait_for_bit();
	wait_for_bit();
	wait_for_bit();
	/* Ready for RX input, but high due to pull-up */
	pinMode(22, INPUT_PULLUP);
	return c;
	//return EOF;
}

/* Sends special sequences through the UPDI link */
int UPDI_io::put(ctrl c)
{
	/* This nested function expects the timer output to just have gone low */
	/* It waits for 12 minimum baud bit times (break character) then goes high */
	auto break_pulse = [] {
		for (uint8_t i = 0; i < 11; i++) wait_for_bit();
		setup_bit_high();
		wait_for_bit();
		pinMode(22, INPUT_PULLUP);
	};

	//stop_timer();
	//start_timer();
	/* Enable TX output */
	pinMode(22, OUTPUT);
	/* Send falling edge */
	setup_bit_low();
	switch (c) {
		case double_break:
			break_pulse();
			setup_bit_low();
			wait_for_bit();
			pinMode(22, OUTPUT);
		case single_break:
			break_pulse();
			wait_for_bit();	
			break;
		case enable:
		/*
			TCCR0A = 0;
			DDRD |= (1 << DDD6);
			PORTD &= ~(1 << DDD6);
			__builtin_avr_nops(5);
			PORTD |= (1 << DDD6);
			DDRD &= ~(1 << DDD6);
			setup_bit_high();
			break;
		*/
		default:
			break;
	}
	//start_timer();
	return 0;
}

int UPDI_io::get() {
	//stop_timer();
	/* Wait for middle of start bit */
	//OCR0A = BIT_TIME / 2 - 1;
	//TCNT0 = 12;		// overhead time; needs to be calibrated
	/* Make sure overflow flag is reset */
	//TIFR0 = (1 << OCF0A);
	
	/* Must disable pull-up, because the UPDI UART just sends very short output pulses at the beginning of each bit time. */
	/* If pull up is enabled, there will be a drift to high state that results in erroneous input sampling. */
	/* As a side effect, random electrical fluctuations of the input prevent an infinite wait loop */
	/* in case no target is connected. */
	pinMode(22, INPUT);
	/* Wait for start bit */
	while (digitalRead(22) == HIGH);

	//start_timer();
	wait_for_bit();
	/* Setup sampling time */
	/* Sample bits */
	uint8_t c = 0;
	for (uint8_t i = 0; i < 8; i++) {
		wait_for_bit();
		/* Take sample */
		c = (c >> 1) | ((uint8_t) ((digitalRead(22)) << 1));		// The cast is to prevent promotion to 16 bit
	}
	/* To Do Sample Parity */
	wait_for_bit();
	wait_for_bit();
	wait_for_bit();
	
	/* Return as soon as high parity or stop bits start */
	while (digitalRead(22) == LOW);
	/* Re-enable pull up */
	pinMode(22, INPUT_PULLUP);
	return c;
}

void IRAM_ATTR onTimer(){
	digitalWrite(22, pinStatus);

}

void UPDI_io::init(void)
{
	timer = timerBegin(0, 80000000UL / F_CPU, true);
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, BIT_TIME, true);
	timerSetCountUp(timer, true);
	timerSetAutoReload(timer, true);
	setup_bit_high();
	start_timer();
}

inline void UPDI_io::setup_bit_low() {
	/* OC0A will go low on match with OCR0A */
	/* Also, set CTC mode - reset timer on match with OCR0A */
	//pinStatus = LOW;
	digitalWrite(22, LOW);
}

inline void UPDI_io::setup_bit_high() {
	/* OC0A will go high on match with OCR0A */
	/* Also, set CTC mode - reset timer on match with OCR0A */
	//pinStatus = HIGH;
	digitalWrite(22, HIGH);
}

inline void UPDI_io::wait_for_bit() {
	/* Wait for compare match */
	delayMicroseconds(BIT_TIME);
}

inline void UPDI_io::stop_timer() {
	timerStop(timer);
}

inline void UPDI_io::start_timer() {
	timerAlarmWrite(timer, BIT_TIME, true);
	timerStart(timer);
}

/*
inline uint8_t UPDI_io::get_parity(uint8_t c) {
	asm(
	"mov	r0, %0	\n"
	"swap	r0		\n"
	"eor	%0, r0	\n"
	"mov	r0, %0	\n"
	"lsr	r0		\n"
	"lsr	r0		\n"
	"eor	%0, r0	\n"
	"mov	r0, %0	\n"
	"lsr	r0		\n"
	"eor	%0, r0	\n"
	:"+r" (c)
	:
	:"r0"
	);
	return c & 0x01;
}
*/
