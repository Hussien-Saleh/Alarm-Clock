#ifndef SES_ALARM_CLOCK_H
#define SES_ALARM_CLOCK_H

/*INCLUDES *******************************************************************/

#include <ses_common.h>
#include "ses_scheduler.h"
#include "ses_lcd.h"
#include "ses_led.h"
#include "ses_button.h"

#define HOUR_TO_MILLI       360000	//To convert hours to milliseconds
#define MINUTE_TO_MILLI     60000	//To convert minutes to milliseconds
#define SECOND_TO_MILLI     1000	//To convert seconds to milliseconds


#define TRANSITION(newState) (fsm->state = newState, RET_TRANSITION)

/* TYPES **********************************************************/

typedef struct fsm_s Fsm; //< typedef for alarm clock state machine
typedef struct event_s Event; //< event type for alarm clock fsm

/** return values */
enum {
	RET_HANDLED, //< event was handled
	RET_IGNORED, //< event was ignored; not used in this implementation
	RET_TRANSITION //< event was handled and a state transition occurred
};

/* possible event signals */
enum {
	ENTRY,
	EXIT,
	ALARM_CHECK_STATE,
	ALARM_TIMEOUT,
	ROTARY_PRESSED,
	JOYSTICK_PRESSED
};

/* enum to specify the color of the led */
typedef enum {
	RED,
	GREEN,
	YELLOW
} color;

typedef uint8_t fsmReturnStatus; //< typedef to be used with above enum

/** typedef for state event handler functions */
typedef fsmReturnStatus (*State)(Fsm *, const Event *);

struct time_t {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t milli;
};

struct fsm_s {
	State state; //< current state, pointer to event handler
	bool isAlarmEnabled; //< flag for the alarm status
	struct time_t timeSet; //< multi-purpose var for system time and alarm time
	struct time_t alarmSet;	 //< var for the alarm
};

struct event_s {
	uint8_t signal; //< identifies the type of event
};


/* FUNCTION PROTOTYPES *******************************************************/

/**
 * Updates the LCD with the time HH:MM being set
 * 
 * @param fsm	FSM pointer
 */
void setClkTimeLCD(Fsm* fsm);

/**
 * Updates the LCD with the alarm HH:MM being set
 * 
 * @param fsm	FSM pointer
 */
void setAlarmTimeLCD(Fsm* fsm);

/**
 * Updates the LCD with the current time during normal mode operation
 */
void updateClkTimeLCD(void * param);

/**
 * Dispatches a pressed joystick button event signal to the FSM
 */
static void joystickPressedDispatch(void * param);

/**
 * Dispatches a pressed rotary button event signal to the FSM
 */
static void rotaryPressedDispatch(void * param);

/**
 * Dispatches an alarm check state event signal to the FSM
 */
static void alarmStateDispatch(void * param);

/**
 * Dispatches an alarmTimeout event signal to the FSM
 */
static void alarmTimeout(void * param);

/**
 * Set Clock Hours
 *
 * @param	fsm		FSM Pointer
 * @param	e		Event
 */
fsmReturnStatus setClkHours(Fsm* fsm, const Event* e);

/**
 * Set Clock Minutes
 *
 * @param	fsm		FSM Pointer
 * @param	e		Event
 */
fsmReturnStatus setClkMinutes(Fsm* fsm, const Event* e);

/**
 * Normal clock mode
 *
 * @param	fsm		FSM Pointer
 * @param	e		Event
 */
fsmReturnStatus normalClkMode(Fsm* fsm, const Event* e);

/**
 * Set Alarm Hours
 *
 * @param	fsm		FSM Pointer
 * @param	e		Event
 */
fsmReturnStatus setAlarmHours(Fsm* fsm, const Event* e);

/**
 * Set Alarm Minutes
 *
 * @param	fsm		FSM Pointer
 * @param	e		Event
 */
fsmReturnStatus setAlarmMinutes(Fsm* fsm, const Event* e);

/**
 * Alarm is running
 *
 * @param	fsm		FSM Pointer
 * @param	e		Event
 */
fsmReturnStatus runAlarm(Fsm* fsm, const Event* e);

/**
 * Check if alarm time has been reached
 *
 * @param 	fsm		FSM Pointer
 */
bool checkAlarmState(Fsm* fsm);

/**
 * Dispatches an event to the FSM
 * 
 * @param fsm	FSM pointer
 * @param event	Event to be dispatched
*/
inline static void fsm_dispatch(Fsm * fsm, const Event * event);

/**
 * Initializes FSM and set the first initial state
 * 
 * @param fsm	FSM pointer
 * @param init	Initial state
 */
inline static void fsm_init(Fsm * fsm, State init);

/**
 * Converts from human readable time to machine time in milliseconds
 * 
 * @param time	Human readable time
 */
static systemTime_t toSystemTime(struct time_t time);

/**
 * Converts from machine time in milliseconds to human readable time
 * 
 * @param systemTime	machine time in milliseconds
 */
static struct time_t fromSystemTime(systemTime_t systemTime);

/**
 * Set up task1 for scheduler
 */
void task1Config();

/**
 * Set up task2 for scheduler
 */
void task2Config();

/**
 * Set up task3 for scheduler
 */
void task3Config();

/**
 * Set up task4 for scheduler
 */
void task4Config();

/**
 * Set up task5 for scheduler
 */
void task5Config();

#endif /* SES_ALARM_CLOCK_H */
