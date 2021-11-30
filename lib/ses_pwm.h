#ifndef SES_PWM_H_  
#define SES_PWM_H_

#include "ses_common.h"

#define MOTOR_CONTROL_PIN   5

/* initialized Timer0 for Fast PWM mode */
void pwm_init(void);

/* set motor duty cycle 
@param dutyCycle, the value of the duty cycle
 */
void pwm_setDutyCycle(uint8_t dutyCycle);

#endif