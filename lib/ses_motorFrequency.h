#ifndef SES_MOTOR_FREQUENCY_H_
#define SES_MOTOR_FREQUENCY_H_

#include "ses_common.h"


#define NO_OF_SAMPLES	19
#define TIMER5_FREQ   250000

/* FUNCTION PROTOTYPES *******************************************************/

/**initializes external interrupt for motor frequency calculation
 */
void motorFrequency_init();

/**get latest motor frequency
 */
uint16_t motorFrequency_getRecent();

/**get median of N number of motor frequencies measured
 */
uint16_t motorFrequency_getMedian();




#endif