/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id$
*/

#include "wiring_private.h"

#ifndef TIMER0_CLK_SRC
#if F_CPU < 250000L
#define TIMER0_CLK_SRC 1	/* CS0 :1 */
#elif F_CPU < 2000000L
#define TIMER0_CLK_SRC 2	/* CS0 | CS1 :8*/
#else
#define TIMER0_CLK_SRC 3	/* CS0 | CS1 :64*/
#endif
#endif

#if TIMER0_CLK_SRC == 1
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(1 * 256))
#elif TIMER0_CLK_SRC == 2
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(8 * 256))
#elif TIMER0_CLK_SRC == 3
// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
#elif TIMER0_CLK_SRC == 4
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(256 * 256))
#elif TIMER0_CLK_SRC == 5
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(1024 * 256))
#else 
#error "TIMER0_CLK_SRC has illigal value"
#endif

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
// Mic: shift by only 2, for better precision, also changed isr
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 2)
#define FRACT_MAX ((1000 >> 2) - FRACT_INC)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

//#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#ifndef TIMER0_OVF_vect
ISR(TIM0_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	} else {
	  f += FRACT_INC;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

// the timing fuctions are decleared weak to allow the user 
// to use a own clock scheme

unsigned long millis(void) __attribute__((weak));
unsigned long millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}

unsigned long micros(void) __attribute__((weak));
unsigned long micros() {
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
#if defined(TCNT0)
	t = TCNT0;
#elif defined(TCNT0L)
	t = TCNT0L;
#else
	#error TIMER 0 not defined
#endif

  
#ifdef TIFR0
	if ((TIFR0 & _BV(TOV0)) && (t < 255))
		m++;
#else
	if ((TIFR & _BV(TOV0)) && (t < 255))
		m++;
#endif

	SREG = oldSREG;
	
#if TIMER0_CLK_SRC == 1
	return ((m << 8) + t) * (1 / clockCyclesPerMicrosecond());
#elif TIMER0_CLK_SRC == 2
	return ((m << 8) + t) * (8 / clockCyclesPerMicrosecond());
#elif TIMER0_CLK_SRC == 3
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
#elif TIMER0_CLK_SRC == 4
	return ((m << 8) + t) * (256 / clockCyclesPerMicrosecond());
#elif TIMER0_CLK_SRC == 5
	return ((m << 8) + t) * (1024 / clockCyclesPerMicrosecond());
#else 
#error "TIMER0_CLK_SRC has illigal value"
#endif
}

void delay(unsigned long) __attribute__((weak));
void delay(unsigned long ms)
{
	uint16_t start = (uint16_t)micros();

	while (ms > 0) {
		yield();
		if (((uint16_t)micros() - start) >= 1000) {
			ms--;
			start += 1000;
		}
	}
}

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayMicroseconds(unsigned int us) __attribute__((weak));
void delayMicroseconds(unsigned int us)
{
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);
#if F_CPU >= 20000000L
	// for the 20 MHz clock on rare Arduino boards

	// for a one-microsecond delay, simply wait 2 cycle and return. The overhead
	// of the function call yields a delay of exactly a one microsecond.
	__asm__ __volatile__ (
		"nop" "\n\t"
		"nop"); //just waiting 2 cycle
	if (--us == 0)
		return;

	// the following loop takes a 1/5 of a microsecond (4 cycles)
	// per iteration, so execute it five times for each microsecond of
	// delay requested.
	us = (us<<2) + us; // x5 us

	// account for the time taken in the preceeding commands.
	us -= 2;

#elif F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;
#elif F_CPU >= 12000000L
	// for the 12 MHz clock on spare boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay 
	if (--us == 0)
		return;

	// the following loop takes a third of a microsecond (4 cycles)
	// per iteration, so execute it three times for each microsecond of
	// delay requested.
	us = (us << 1) + us; // x3

	// account for the time taken in the preceeding commands.
	us -= 2;
#else
	// for the 8 MHz internal clock on the ATmega168

	// for a one- or two-microsecond delay, simply return.  the overhead of
	// the function calls takes more than two microseconds.  can't just
	// subtract two, since us is unsigned; we'd overflow.
	if (--us == 0)
		return;
	if (--us == 0)
		return;

	// the following loop takes half of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1;
    
	// partially compensate for the time taken by the preceeding commands.
	// we can't subtract any more than this or we'd overflow w/ small delays.
	us--;
#endif

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
}

void init()
{
	// this needs to be called before setup() or some functions won't
	// work there
	sei();
	
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
#if defined(TCCR0A) && defined(WGM01)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);
#elif defined(TCCR0) && defined(WGM01)
	// ATmega32 uses Timer0 is fast PWM for output
	sbi(TCCR0, WGM01);
	sbi(TCCR0, WGM00);
#endif  

	// set timer 0 prescale factor to 64
#if defined(TCCR0B) 
	// this combination is for the standard 168/328/1280/2560
#define TCCR0_CLK_SRC TCCR0B
#elif defined(TCCR0A)
	// this combination is for the __AVR_ATmega645__ series
#define TCCR0_CLK_SRC TCCR0A
#elif defined(TCCR0)
	// this combination is for the standard atmega8
#define TCCR0_CLK_SRC TCCR0
#else
	#error Timer 0 prescale factor 64 not set correctly
#endif
#if (TIMER0_CLK_SRC & 4) && defined(CS02)
	sbi(TCCR0_CLK_SRC, CS02);
#endif
#if (TIMER0_CLK_SRC & 2) && defined(CS01)
	sbi(TCCR0_CLK_SRC, CS01);
#endif
#if (TIMER0_CLK_SRC & 1) && defined(CS00)
	sbi(TCCR0_CLK_SRC, CS00);
#endif

	// enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
	sbi(TIMSK, TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
	sbi(TIMSK0, TOIE0);
#else
	#error	Timer 0 overflow interrupt not set correctly
#endif

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle
#if defined(TCCR1B)
#if defined(TCCR1B) && defined(CS11) && defined(CS10)
	TCCR1B = 0;
#endif
	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
#if F_CPU >= 8000000L
	sbi(TCCR1B, CS10);
#endif
#elif defined(TCCR1) && defined(CS11) && defined(CS10)
	sbi(TCCR1, CS11);
#if F_CPU >= 8000000L
	sbi(TCCR1, CS10);
#endif
#endif
	// put timer 1 in 8-bit phase correct pwm mode
#if defined(TCCR1A) && defined(WGM10)
	sbi(TCCR1A, WGM10);
#elif defined(TCCR1)
	#warning this needs to be finished
#endif

	// set timer 2 prescale factor to 64
#if defined(TCCR2) && defined(CS22)
	sbi(TCCR2, CS22);
#elif defined(TCCR2B) && defined(CS22)
	sbi(TCCR2B, CS22);
#else
	#warning Timer 2 not finished (may not be present on this CPU)
#endif

	// configure timer 2 for phase correct pwm (8-bit)
#if defined(TCCR2) && defined(WGM20)
	sbi(TCCR2, WGM20);
#elif defined(TCCR2A) && defined(WGM20)
	sbi(TCCR2A, WGM20);
#else
	#warning Timer 2 not finished (may not be present on this CPU)
#endif

#if defined(TCCR3B) && defined(CS31) && defined(WGM30)
	sbi(TCCR3B, CS31);		// set timer 3 prescale factor to 64
	sbi(TCCR3B, CS30);
	sbi(TCCR3A, WGM30);		// put timer 3 in 8-bit phase correct pwm mode
#endif

#if defined(TCCR4A) && defined(TCCR4B) && defined(TCCR4D) /* beginning of timer4 block for 32U4 and similar */
	sbi(TCCR4B, CS42);		// set timer4 prescale factor to 64
	sbi(TCCR4B, CS41);
	sbi(TCCR4B, CS40);
	sbi(TCCR4D, WGM40);		// put timer 4 in phase- and frequency-correct PWM mode	
	sbi(TCCR4A, PWM4A);		// enable PWM mode for comparator OCR4A
	sbi(TCCR4C, PWM4D);		// enable PWM mode for comparator OCR4D
#else /* beginning of timer4 block for ATMEGA1280 and ATMEGA2560 */
#if defined(TCCR4B) && defined(CS41) && defined(WGM40)
	sbi(TCCR4B, CS41);		// set timer 4 prescale factor to 64
	sbi(TCCR4B, CS40);
	sbi(TCCR4A, WGM40);		// put timer 4 in 8-bit phase correct pwm mode
#endif
#endif /* end timer4 block for ATMEGA1280/2560 and similar */	

#if defined(TCCR5B) && defined(CS51) && defined(WGM50)
	sbi(TCCR5B, CS51);		// set timer 5 prescale factor to 64
	sbi(TCCR5B, CS50);
	sbi(TCCR5A, WGM50);		// put timer 5 in 8-bit phase correct pwm mode
#endif

#if defined(ADCSRA)
	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

	// enable a2d conversions
	sbi(ADCSRA, ADEN);
#endif

	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
#if defined(UCSRB)
	UCSRB = 0;
#elif defined(UCSR0B)
	UCSR0B = 0;
#endif
}

/**
 * Empty yield() hook.
 *
 * This function is intended to be used by library writers to build
 * libraries or sketches that supports cooperative threads.
 *
 * Its defined as a weak symbol and it can be redefined to implement a
 * real cooperative scheduler.
 */
static void __empty() {
	// Empty
}
void yield(void) __attribute__ ((weak, alias("__empty")));

