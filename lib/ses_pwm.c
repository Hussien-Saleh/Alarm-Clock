#include "ses_pwm.h"


void pwm_init(void){

    PRR0 &= ~(1<<PRTIM0);                       // Enable Timer0
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0);    // Set OC0B on compare match, inverting fast-pwm mode

    TCCR0A |= (0x03 << WGM00);                  // Configure fast-pwm mode

    TCCR0B &= ~(1<< FOC0B);                     // Ensure force output compare bit is cleared
    TCCR0B |= (1<< CS00);                       // No prescaler mode

    TCNT0  = 0x00;                              // Clear timer/counter0 register

}
void pwm_setDutyCycle(uint8_t dutyCycle){
    OCR0B = 255-dutyCycle;
}