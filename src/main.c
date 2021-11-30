/* INCLUDES ******************************************************************/

#include "../include/alarmClock.h"

/* VARIABLES *****************************************************************/

static Fsm theFsm;

//Declare tasks
static taskDescriptor task1; //Toggle Green LED every second
static taskDescriptor task2; //Compare the alarm time with current time every second
static taskDescriptor task3; //Update the display with the current time
static taskDescriptor task4; //Turn off the alarm after 5 seconds
static taskDescriptor task5; //Toggle Red LED at 4Hz

/* MAIN **********************************************************************/

int main(void) {

	//Initialize peripherials
	lcd_init();
	led_redInit();
	led_greenInit();
	led_yellowInit();

	button_init(true);

	//Configure Tasks
	task1Config(); 
	task2Config();
	task3Config();
	task4Config();
	task5Config();

	//Set button callbacks
	button_setJoystickButtonCallback(&joystickPressedDispatch);
	button_setRotaryButtonCallback(&rotaryPressedDispatch);

	//Initialize and run FSM & Scheduler
	fsm_init(&theFsm, setClkHours);
	scheduler_init();
	scheduler_run();
	
}

/* FUNCTION DEFINITION *******************************************************/

void task1Config(){
	//Toggle Green LED every second
	task1.task = (task_t) &led_greenToggle;
	task1.param = NULL;
	task1.expire = 1000;
	task1.period = 1000;
}

void task2Config(){
	//Compare the alarm time with current time every second
	task2.task = &alarmStateDispatch;
	task2.param = NULL;
	task2.expire = 1000;
	task2.period = 1000;
}

void task3Config(){
	//Update the display with the current time
	task3.task = &updateClkTimeLCD;
	task3.param = NULL;
	task3.expire = 1000;
	task3.period = 1000;
}

void task4Config(){
	//Turn off the alarm after 5 seconds
	task4.task = &alarmTimeout;
	task4.param = NULL;
	task4.expire = 5000;
	task4.period = 0;
}

void task5Config(){
	//Toggle Red LED at 4Hz
	task5.task = (task_t) &led_redToggle;
	task5.param = NULL;
	task5.expire = 250;
	task5.period = 250;
}

void setClkTimeLCD(Fsm* fsm){

	lcd_clear();
	lcd_setCursor(4,0);
	fprintf(lcdout, "%02d:%02d", fsm->timeSet.hour, fsm->timeSet.minute);

	//Prompt user to set time
	lcd_setCursor(0,1);
	fprintf(lcdout, "Set the time please");
}

void setAlarmTimeLCD(Fsm* fsm){

	lcd_clear();
	lcd_setCursor(4,0);
	fprintf(lcdout, "%02d:%02d", fsm->alarmSet.hour, fsm->alarmSet.minute);

	//Prompt user to set alarm
	lcd_setCursor(0,1);
	fprintf(lcdout, "Set the alarm please");
}

void updateClkTimeLCD(void * param){
	//Print current time
	struct time_t clockTime = fromSystemTime(scheduler_getTime());
	lcd_clear();
	lcd_setCursor(4,0);
	fprintf(lcdout, "%02d:%02d:%02d", clockTime.hour, clockTime.minute, clockTime.second);
}

static void joystickPressedDispatch(void * param) {
	Event e = {.signal = JOYSTICK_PRESSED};
	fsm_dispatch(&theFsm, &e);
}

static void rotaryPressedDispatch(void * param) {
	Event e = {.signal = ROTARY_PRESSED};
	fsm_dispatch(&theFsm, &e);
}

static void alarmStateDispatch(void * param) {
	Event e = {.signal = ALARM_CHECK_STATE};
	fsm_dispatch(&theFsm, &e);
}

static void alarmTimeout(void * param) {
	Event e = {.signal = ALARM_TIMEOUT};
	fsm_dispatch(&theFsm, &e);
}


inline static void fsm_dispatch(Fsm * fsm, const Event * event){
	static Event entryEvent = {.signal = ENTRY};
	static Event exitEvent = {.signal = EXIT};
	State s = fsm->state;
	fsmReturnStatus r = fsm->state(fsm, event);
	if (r == RET_TRANSITION) {
		s(fsm, &exitEvent); //< call exit action of last state
		fsm->state(fsm, &entryEvent); //< call entry action of new state
	}
}

inline static void fsm_init(Fsm * fsm, State init){
	//... other initialization
	Event entryEvent = {.signal = ENTRY};
 	fsm->state = init;
	fsm->state(fsm, &entryEvent);
}

fsmReturnStatus setClkHours(Fsm* fsm, const Event* e) {

	switch(e->signal) {

		case ENTRY:

		//Reset the time and alarm
		fsm->timeSet.hour = 0;
		fsm->timeSet.minute = 0;
		fsm->timeSet.second = 0;
		fsm->timeSet.milli = 0;
		fsm->isAlarmEnabled = false;

		//Display time to be set on LCD
		setClkTimeLCD(fsm);

			return RET_HANDLED;

		case ROTARY_PRESSED:

			//Increment the hours
			fsm->timeSet.hour++;
			if (fsm->timeSet.hour >= 24) {
				fsm->timeSet.hour = 0;
			}

			//Display time to be set on LCD
			setClkTimeLCD(fsm);
			return RET_HANDLED;

		case JOYSTICK_PRESSED:
			//Transition to the next setClkMinutes state
			return TRANSITION(setClkMinutes); 

		default:
			return RET_IGNORED;
		}
}

fsmReturnStatus setClkMinutes(Fsm* fsm, const Event* e) {
	switch(e->signal) {
		case ENTRY:
			//Update LCD
			setClkTimeLCD(fsm);
			return RET_HANDLED;

		case EXIT:
			//Set the scheduler time to the time given by the user
			scheduler_setTime(toSystemTime(fsm->timeSet));
			return RET_HANDLED;

		case ROTARY_PRESSED:
			//Increment the minutes and update the display
			fsm->timeSet.minute++;
			if (fsm->timeSet.minute >= 60) {
				fsm->timeSet.minute = 0;
			}
			setClkTimeLCD(fsm);
			return RET_HANDLED;

		case JOYSTICK_PRESSED:
			//Transition to normalClkMode state
			return TRANSITION(normalClkMode);

		default:
			return RET_IGNORED;
	}
}

fsmReturnStatus normalClkMode(Fsm* fsm, const Event* e) {

	switch(e->signal) {
		case ENTRY:

			//Toggle Green LED
			scheduler_add(&task1);

			//Display time on the lcd
			scheduler_add(&task3);

			//Turn off Red LED
			led_redOff();

			return RET_HANDLED;

		case ROTARY_PRESSED:

			//Enable/Disable (Toggle) the Alarm
			if(!(fsm->isAlarmEnabled)) {

				fsm->isAlarmEnabled = true;
				led_yellowOn();
				scheduler_add(&task2);
			} else {

				fsm->isAlarmEnabled = false;
				led_yellowOff();
				scheduler_remove(&task2);

			}
			return RET_HANDLED;

		case JOYSTICK_PRESSED:

			scheduler_remove(&task3);

			if(fsm->isAlarmEnabled) {
				fsm->isAlarmEnabled = false;
				scheduler_remove(&task2);
				led_yellowOff();
			}
			// Transition to setting alarm hours
			return TRANSITION(setAlarmHours);

		case ALARM_CHECK_STATE:

			//Check if alarm is enabled and if alarm time has been reached
			if (checkAlarmState(fsm) && fsm->isAlarmEnabled) {

				return TRANSITION(runAlarm);

			} 
			else{
				return RET_IGNORED;
			}

		default:
			return RET_IGNORED;
	}
}

fsmReturnStatus setAlarmHours(Fsm* fsm, const Event* e) {
	switch(e->signal) {
		case ENTRY:
			//Reset alarmSet
			fsm->alarmSet.hour = 0;
			fsm->alarmSet.minute = 0;
			fsm->alarmSet.second = 0;
			fsm->alarmSet.milli = 0;

			//Update LCD with alarm
			scheduler_remove(&task3);
			setAlarmTimeLCD(fsm);
			return RET_HANDLED;

		case ROTARY_PRESSED:
			//Increment the alarm hours
			fsm->alarmSet.hour++;
			if (fsm->alarmSet.hour >= 24) {
				fsm->alarmSet.hour = 0;
			}

			//Update the display
			setAlarmTimeLCD(fsm);
			return RET_HANDLED;

		case JOYSTICK_PRESSED:
			//Transition to next state to set minutes
			return TRANSITION(setAlarmMinutes);

		default:
			return RET_IGNORED;
	}
}

fsmReturnStatus setAlarmMinutes(Fsm* fsm, const Event* e) {
	switch (e->signal) {
		case ROTARY_PRESSED:
			//Increment the alarm minutes 
			fsm->alarmSet.minute++;
			if (fsm->alarmSet.minute >= 60) {
				fsm->alarmSet.minute = 0;
			}

			//Update the display
			setAlarmTimeLCD(fsm);
			return RET_HANDLED;

		case JOYSTICK_PRESSED:
			// Transition to normal operation mode
			return TRANSITION(normalClkMode);

		default:
			return RET_IGNORED;
	}
}

fsmReturnStatus runAlarm(Fsm* fsm, const Event* e) {
	switch (e->signal) {
		case ENTRY:
			// Set alarm time as 5 sec
			scheduler_add(&task4);

			// Toggle red LED at 4Hz
			scheduler_add(&task5);
			return RET_HANDLED;

		case EXIT:

			// Disable the alarm
			scheduler_remove(&task2);
			scheduler_remove(&task4);
			fsm->isAlarmEnabled = false;

			//Turn of LEDs
			scheduler_remove(&task5);
			led_yellowOff();
			led_redOff();
			return RET_HANDLED;

		case ALARM_TIMEOUT:
			// Transition to normal mode of operation
			scheduler_remove(&task4);
			return TRANSITION(normalClkMode);

		case JOYSTICK_PRESSED:
			// Transition to normal mode of operation
			scheduler_remove(&task4);
			return TRANSITION(normalClkMode);

		case ROTARY_PRESSED:
			// Transition to normal mode of operation
			scheduler_remove(&task4);
			return TRANSITION(normalClkMode);

		default:
			return RET_IGNORED;
	}
}

bool checkAlarmState(Fsm* fsm) {
	
	//Compare scheduler (clock) time with set alarm time
	struct time_t currentTime = fromSystemTime(scheduler_getTime());
	if(currentTime.hour == fsm->alarmSet.hour && currentTime.minute == fsm->alarmSet.minute) {
		return true; 	// Return true if alarm time reached
	}
	return false;
}

static systemTime_t toSystemTime(struct time_t time){

	systemTime_t t = ((time.hour)*HOUR_TO_MILLI + (time.minute)*MINUTE_TO_MILLI + (time.second)*SECOND_TO_MILLI + (time.milli));
	
	return t;
}

static struct time_t fromSystemTime(systemTime_t systemTime){
	struct time_t t;
	t.hour =  (systemTime/(HOUR_TO_MILLI));
	systemTime =  (systemTime - (HOUR_TO_MILLI)*t.hour);

	t.minute =  (systemTime/(MINUTE_TO_MILLI));
	systemTime =  (systemTime - (MINUTE_TO_MILLI)*t.minute);

	t.second =  (systemTime/(SECOND_TO_MILLI));
	systemTime =  (systemTime - (SECOND_TO_MILLI)*t.second);

	t.milli = systemTime;

	return t;
}


