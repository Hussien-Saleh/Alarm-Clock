#include "ses_common.h"
#include "ses_button.h"
#include "ses_timer.h"

#define BUTTON_ENCODER_PORT       	PORTB
#define BUTTON_ENCODER_PIN         	6
#define BUTTON_JOYSTICK_PORT        PORTB
#define BUTTON_JOYSTICK_PIN      	7


#define BUTTON_NUM_DEBOUNCE_CHECKS    5
#define BUTTON_DEBOUNCE_PERIOD       // TODO
#define BUTTON_DEBOUNCE_POS_JOYSTICK  0x01
#define BUTTON_DEBOUNCE_POS_ROTARY    0x02


static volatile pButtonCallback rotaryCallback = NULL;
static volatile pButtonCallback joystickCallback = NULL;


void button_init(bool debouncing){

    /* Configuration of the buttons as input pins */

    DDR_REGISTER(BUTTON_ENCODER_PORT) &= ~(1<<BUTTON_ENCODER_PIN);
    DDR_REGISTER(BUTTON_JOYSTICK_PORT) &= ~(1<<BUTTON_JOYSTICK_PIN);

    /* Activation of the internal pull-up resistors for eacch of the buttons */
    BUTTON_ENCODER_PORT  |=(1<<BUTTON_ENCODER_PIN);
    BUTTON_JOYSTICK_PORT |=(1<<BUTTON_JOYSTICK_PIN);

    if(debouncing) {
        // Initilalize Debouncing and timer1
        pTimerCallback button_check = &button_checkState;
        timer1_setCallback(button_check);
        timer1_start();
    }
    else {
        // direct interrupt initialization
        PCICR |= (1 << PCIE0); 
        PCMSK0 |= (1 << BUTTON_ENCODER_PIN);    //Enable pin change interrupt
        PCMSK0 |= (1 << BUTTON_JOYSTICK_PIN);   
    }
}

 bool button_isJoystickPressed(void){

    if (!(PIN_REGISTER(BUTTON_JOYSTICK_PORT) & (1 << BUTTON_JOYSTICK_PIN)))
        return true;
    else 
        return false;
}

bool button_isRotaryPressed(void){

    if (!(PIN_REGISTER(BUTTON_ENCODER_PORT) & (1 << BUTTON_ENCODER_PIN)))
        return true;
    else 
        return false;
}

void button_setRotaryButtonCallback(pButtonCallback callback){

    if (callback != NULL)
        rotaryCallback = callback;
}

void button_setJoystickButtonCallback(pButtonCallback callback){
    
    if (callback != NULL)
        joystickCallback = callback;

}

 void button_checkState() {

    static uint8_t state[BUTTON_NUM_DEBOUNCE_CHECKS] = {};
    static uint8_t index = 0;
    static uint8_t debouncedState = 0;
    uint8_t lastDebouncedState = debouncedState;

    // each bit in every state byte represents one button
    state[index] = 0;
    if(button_isJoystickPressed()) {
        state[index] |= BUTTON_DEBOUNCE_POS_JOYSTICK;
    }
    if(button_isRotaryPressed()) {
        state[index] |= BUTTON_DEBOUNCE_POS_ROTARY;
    }

    index++;
    if (index == BUTTON_NUM_DEBOUNCE_CHECKS) {
        index = 0; 
    }

    // init compare value and compare with ALL reads, only if
    // we read BUTTON_NUM_DEBOUNCE_CHECKS consistent "1's" in the state
    // array, the button at this position is considered pressed

    uint8_t j = 0xFF;
    for(uint8_t i = 0; i < BUTTON_NUM_DEBOUNCE_CHECKS; i++) {
        j = j & state[i];
    }
    debouncedState = j;
    
    // ensure that the button is pressed
    if(debouncedState != 0 && lastDebouncedState == 0){

        // execute the appropriate callback
        if(debouncedState & BUTTON_DEBOUNCE_POS_JOYSTICK){
            joystickCallback();
        }
        if(debouncedState & BUTTON_DEBOUNCE_POS_ROTARY){
            rotaryCallback();
        }
        
    }

}
 


ISR(PCINT0_vect){

    if(button_isJoystickPressed()){
        if((joystickCallback != NULL) && ((PCMSK0 & (1<<BUTTON_JOYSTICK_PIN)))){
            joystickCallback();
        }

    }
    else if(button_isRotaryPressed()){
        if((rotaryCallback != NULL) && ((PCMSK0 & (1<<BUTTON_ENCODER_PIN)))){ 
            rotaryCallback();
        }
    }  
}