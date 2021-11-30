/* INCLUDES ******************************************************************/
#include "ses_timer.h"
#include "ses_led.h"

/* DEFINES & MACROS **********************************************************/
#define TIMER1_CYC_FOR_5MILLISEC       0x270F  

/* FCPU=16MHZ, Prescaler=64, Tclock = Prescaler/FCPU)
To generate interrupt every 1ms, Ttimer; time it takes match compare match= 1ms
Ttimer = (Tclock * (OCR+1) */
#define TIMER2_CYC_FOR_1MILLISEC	   0xF9  

#define TIMER5_CYC_FOR_1SEC	           0xF423  

static volatile pTimerCallback timer2 = NULL;
static volatile pTimerCallback timer1 = NULL;
static volatile pTimerCallback timer5 = NULL;


/*FUNCTION DEFINITION ********************************************************/
void timer2_setCallback(pTimerCallback cb) {
	timer2 = cb;
}

void timer2_start() {

	/* initialize counter */
	TCNT2 = 0x00;  
	/* CTC mode, Clear timer on compare match mode */
	TCCR2A |= (1 << WGM21);				
	/* prescaler F_CPU/64 */
	TCCR2B |= (1 << CS22);				
	/* generate interrupt every 1ms */
	OCR2A |= TIMER5_CYC_FOR_1SEC;  
	/* clear compare A flag interrupt */
	TIFR2 |= (1 << OCF2A); 		
	/* enable compare A match interrupt */	
	TIMSK2 |= (1 << OCIE2A);			
}


void timer2_stop() {
	TCCR2B &= ~(0x07 << CS20);	// no clock source		
    //TIMSK2 &= ~(1 << OCIE2A);
}

void timer1_setCallback(pTimerCallback cb) {
	 timer1 = cb;
}


void timer1_start() {
	
	/* initialize counter */
	TCNT1 = 0x00;  
	/* Setting to CTC mode, prescaler F_CPU/8 */
	TCCR1B |= ((1 << WGM12) | (1<< CS11)); 	

	/* set value to interrupt every 5ms */
	OCR1AH |= ((TIMER1_CYC_FOR_5MILLISEC & 0xFF00) >> 8); 
	OCR1AL |= (TIMER1_CYC_FOR_5MILLISEC & 0xFF);  		  
	/* Clear Compare A flag interrupt */
	TIFR1 |= (1 << OCF1A); 	
	/* Enable Compare A match interrupt */	
	TIMSK1 |= (1 << OCIE1A);	
}


void timer1_stop() {
	/* Disable Compare A match interrupt */
	//TIMSK1 &= ~(1 << OCIE1A); 
	TCCR1B &= ~(0x07 << CS10);	// no clock source		

}

void timer5_start(){

	TCNT5 = 0x00;
	TCCR5B |= (1 << WGM52);					// CTC Mode
	TCCR5B |= (1 << CS50) | (1 << CS51);	// Set prescaler to 64
	
	OCR5A = TIMER5_CYC_FOR_1SEC; 			// Set OCR5A to a value for 1 second
	TIFR5 |= (1 << OCF5A); 		// Clear Compare A flag interrupt
	TIMSK5 |= (1 << OCIE5A);	// Enable Compare A match interrupt
}

void timer5_stop(){
	TCCR5B &= ~(0x07 << CS50);	// no clock source	
}

void timer5_setCallback(pTimerCallback cb) {
	 timer5 = cb;
}

uint16_t timer5_getCount(){
    return TCNT5;
}

ISR (TIMER5_COMPA_vect){
	
	if(timer5 != NULL){ 
		
		timer5();
		motorFreq = 0;
		arrIndex = (arrIndex + 1) % NO_OF_SAMPLES;  // Increment the index
		motorFreqArr[arrIndex] = motorFreq;
	}

}

ISR(TIMER1_COMPA_vect) {

	if (timer1 != NULL){
		timer1();
	}
}

ISR(TIMER2_COMPA_vect) {

	if (timer2 != NULL){
		timer2();
	}
}
