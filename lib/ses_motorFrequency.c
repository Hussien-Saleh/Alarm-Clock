#include "ses_motorFrequency.h"
#include "ses_led.h"
#include "ses_timer.h"
#include "util/atomic.h"

static volatile int noOfPulses = 0;            // Number of rising edges
volatile uint16_t motorFreq = 0;               // Motor frequency
volatile uint16_t motorFreqArr[NO_OF_SAMPLES]; // Array of motor frequencies
volatile uint16_t arrIndex = 0;                // Array index


void motorFrequency_init(){
  
    /******Enable External Interrupt INT0******/
    DDR_REGISTER(PORTD) &= ~(1 << PD0);     // Set pin as input
    EICRA |= (0x03 << ISC00);               // Rising edge interrupt
    EIMSK |= (1 << INT0);                   // Enable External Interrupt 
	EIFR |= (1<<INTF0);                     // Clear the external interrupt flag

    led_yellowInit();
    led_greenInit();

}

uint16_t motorFrequency_getRecent(){
    return motorFreq;
}

uint16_t motorFrequency_getMedian(){

    uint16_t tempMax=0;

    // Sort data in array in ascending order and return the median

    for(uint8_t i=0 ; i < NO_OF_SAMPLES - 1; i++){

        for(uint8_t j=i+1; j < NO_OF_SAMPLES; j++){
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                if(motorFreqArr[i] > motorFreqArr[j]){
                
                    tempMax =  motorFreqArr[i];
                    motorFreqArr[i] = motorFreqArr[j]; 
                    motorFreqArr[j] = tempMax;
                
                }
            }
        }
    }

    return motorFreqArr[((NO_OF_SAMPLES - 1) / 2)];

}

ISR(INT0_vect){

    noOfPulses++;
    led_yellowToggle();
    led_greenOff();

    // If beginning of revolution detected
    if(noOfPulses == 1){
        timer5_start();
    }

    // If one revolution completed (6 edges)
    if(noOfPulses == 7){

        noOfPulses = 0;                                     // Reset pulse counter
        motorFreq = (uint16_t) (TIMER5_FREQ/ (timer5_getCount() + 1));      // Calculate motor frequency = timerFrequency/Number of timer ticks
        timer5_stop();                                      // Stop the timer

        arrIndex = (arrIndex + 1) % NO_OF_SAMPLES;          // Increment the array index
		motorFreqArr[arrIndex] = motorFreq;                 // Update and store value to array
    }

}