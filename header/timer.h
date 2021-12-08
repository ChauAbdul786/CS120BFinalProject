#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; 

unsigned long _avr_timer_M = 1; //Start count from here, down to 0. Default 1 ms. 
unsigned long _avr_timer_cntcurr = 0; //Current internal count of 1ms ticks.

void TimerOn(){
	TCCR1B = 0x0B; // Bit 3 = 0: CTC (Clear Timer on Compare) mode.
					// B2B1B0 = 011: Pre-scalar / 64.
					// So, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s.
					// Thus, TCNT1 register will count at 125,000 ticks/s.

	OCR1A = 125; // Timer interrupt will be generated when TCNT1 == OCR1A.
				 // We want a 1 ms tick. 0.001s * 125,000 ticks/s = 125.
				 // So when TCNT1 register equals 125, 
				 // 1 ms has passed. Thus, we compare to 125.

	TIMSK1 = 0x02; // bit1: OCR1A -- enables compare match interrupt.

	TCNT1 = 0; //Initialize avr counter

	_avr_timer_cntcurr = _avr_timer_M; //Timer ISR will be called every _avr_timer_cntcurr ms. 

	//Enable global interrupts
	SREG |= 0x80; //0x80" 1000 0000
}

void TimerOff(){
	TCCR1B = 0x00; // Timer off
}

void TimerISR(){
	TimerFlag = 1;
}

// In our approach, C programmer does not touch this ISr, but rather TimerISR()
ISR(TIMER1_COMPA_vect){
	//CPU automatically calls when TCNT1 == OCR1 (Every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; //Count down to 0 rather than up to TOP
	if(_avr_timer_cntcurr == 0){ //Results in a more efficient compare
		TimerISR(); //Call the ISR that the use uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}