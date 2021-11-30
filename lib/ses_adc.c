#include "ses_common.h"
#include "ses_adc.h"
 
#define SENSORS_PORT        PORTF
#define TEMP_SENSOR_PIN     2
#define LIGHT_SENSOR_PIN    4
#define JOYSTICK_PIN        5
#define MICROPHONE_PIN      0

#define ADC_VREF_SRC        0xC0   // 1.6V

#define ADC_TEMP_RAW_MIN    481
#define ADC_TEMP_RAW_MAX    256
#define ADC_TEMP_FACTOR     73
#define ADC_TEMP_MIN        (ADC_TEMP_FACTOR*200)
#define ADC_TEMP_MAX        (ADC_TEMP_FACTOR*400)        



void adc_init(void){

    /* configure of the temperature sensor, light sensor and joystick pins as inputs */
    DDR_REGISTER(SENSORS_PORT) &= ~(1 << TEMP_SENSOR_PIN);
    DDR_REGISTER(SENSORS_PORT) &= ~(1 << LIGHT_SENSOR_PIN);
    DDR_REGISTER(SENSORS_PORT) &= ~(1 << JOYSTICK_PIN);
    /* configure microphone pins (0,1) as input */
    DDR_REGISTER(SENSORS_PORT) &= ~(0x03 << MICROPHONE_PIN);

    /* deactivate the internal pull-up resistors */
    SENSORS_PORT &= ~(1 << TEMP_SENSOR_PIN);
    SENSORS_PORT &= ~(1 << LIGHT_SENSOR_PIN);
    SENSORS_PORT &= ~(1 << JOYSTICK_PIN);
    SENSORS_PORT &= ~(0x03 << MICROPHONE_PIN);

    /* disable ADC module */
    ADCSRA &= ~(1<<ADEN);
    /* disable power reduction mode for ADC module*/
    PRR0 &= ~(1<<PRADC);
    /* configure the voltage reference 1.6V */
    ADMUX |= ADC_VREF_SRC;
    /* set ADC result right adjusted */
    ADMUX &= ~(1<<ADLAR);
    /* clear MUX5 analog channel and gain selection bit*/
    ADCSRB &= ~(1<<MUX5);
    /* configure the prescaler */
    ADCSRA |= ADC_PRESCALE;
    /* disable auto triggering */
    ADCSRA &= ~(1<<ADATE);
    /* enable ADC module */
    ADCSRA |= (1<<ADEN);

}

uint16_t adc_read(uint8_t adc_channel){

    if (adc_channel < ADC_NUM){
        /* clear MUX4:0 bits before setting the required channel */
        ADMUX &= ~(0x1F<<MUX0); 
        /* set the required channel in ADMUX  */
        ADMUX |= (adc_channel & 0x1F);
        /* start conversion */
        ADCSRA |= (1<<ADSC); 
        /* wait until conversion completes */
        while ((ADCSRA & (1<<ADSC)));
    
	    return ADC; 
        
    }else {
        return ADC_INVALID_CHANNEL;
    }
}

int16_t adc_getTemperature(){
    
    int16_t adc = adc_read(ADC_TEMP_CH);
    int16_t slope = (ADC_TEMP_MAX - ADC_TEMP_MIN) / (ADC_TEMP_RAW_MAX - ADC_TEMP_RAW_MIN);
    int16_t offset = ADC_TEMP_MAX - (ADC_TEMP_RAW_MAX * slope);
    return (adc * slope + offset) / ADC_TEMP_FACTOR;

}


uint8_t adc_getJoystickDirection(){


}
