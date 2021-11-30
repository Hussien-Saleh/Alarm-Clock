## Alarm Clock based on Event-Driven Finite-State Machine Model

#### Functionality:

- When the microcontroller is powered up, the actual time is not known. 
- The user has to set the time manually using the buttons.
- At this stage, the display shows an uninitialized clock using the format HH:MM. The second display line may show a request for the user to set time. 
- First, the hour has to be set by repeatedly pressing the Rotary Button. After pressing the Joystick Button, the minutes have to be set via the Rotary Button. 
- Pressing the Joystick Button again updates the system time and starts the clock. 
- In this normal operation mode the time is shown in the format HH:MM:SS. In this state, the user can press the Rotary Button to enable the alarm or to disable the alarm.
- If the Joystick Button is pressed, the alarm time can be set. In this mode line 1 shows the alarm time instead of the current system time using the format HH:MM. 
- If the alarm is enabled and the actual time matches the alarm time, the red LED shall toggle with 4 Hz. 
-  he alarm shall stop, if any button is pressed or 5 seconds have passed. 
- The alarm must only be triggered if the clock is in its normal operating mode, i.e., it must not be triggered while the alarm time is being modified.

**In addition, the LEDs are used for the following functionality:**

- The **green LED** blinks synchronously with the counter of the seconds. 
- The **yellow LED** is on, if and only if the alarm is enabled.
- The **red LED** is flashing with 4Hz during alarm, and it is off otherwise.

![alt text](https://github.com/Hussien-Saleh/Alarm-Clock/blob/master/fsm.png )

<a href="url"><img src="https://github.com/Hussien-Saleh/Alarm-Clock/blob/master/fsm.png " align="centered" height="750" width="750" ></a>

#### Alarm Clock Implementation:

The described alarm clock is implemented using finite state machine (FSM) as an exclusively event-based (i.e., not synchronous) one!
The FSM is implemented with pointers to functions. A state is represented by a pointer to an event handler (i.e., a function), which takes a pointer to the FSM itself and the event that occurred.
The return value is used by the FSM “core” to decide about entry and exit actions.
A structure contains possible events that may trigger state changes is implemented. The type of the event is determined by an event handler (e.g., Joystick Button pressed).
an event dispatcher is used to provide entry and exit actions as well.
The TRANSITION macro is used exclusively to change states to make sure that state changes are always the last operations within an event handler.

The operations regarding the state machine follow a “run-to-completion” semantic (must not be interrupted by another operation on the FSM). This is accomplished by running the FSM completely within task context.
The code can easily be extended to a more generic event processor, which could be used to implement arbitrary state machines. It is inspired heavily by the material presented in “Practical UML Statecharts — Event-Driven Programming for Embedded Systems” by Miro Samek. 
Note also that usually such an event processor would use an event-queue to store incoming events. Howevern, The FSM in in this context uses a task scheduler to queue the events for the alarm clock.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

## Task Scheduler
 
Instead of using multiple hardware timers, a task scheduler is implemented using one hardware timer to manage mutliple tasks and allow the execution of them in parallel after a given time period in a synchronous context (not inside the interrupt service routine) since it avoids executing multiple time-consuming functions inside an interrupt service routine which is disadvantageous because it blocks other interrupts.

In order to provide a synchronous execution of tasks, the scheduler periodically polls for executable tasks inside the scheduler_run() function.
The function scheduler_init() initializes the scheduler and Timer2. 

All tasks are represented by a **linked list data structure**. Each task is described by a function pointer to the function to execute (taskDescriptor.task). 
A task is scheduled for execution after a fixed time period given by taskDescriptor.expire. The scheduler also uses this variable to count down the milliseconds until the task should be executed. 
Tasks can be scheduled for single execution (taskDescriptor.period==0) or periodic execution (taskDescriptor.period>0). 
“Single execution tasks” are removed after execution; periodic tasks remain in the task list and are repeated depending on their period.

**To schedule a task**, these parameters have to be set in the task descriptor and call the scheduler_add() function. A pointer is provided to the taskDescriptor as a call parameter. 
Additionally, the function to be executed takes avoid* parameter to enable the passing of parameters to a task (taskDescriptor.param). Added tasks can later be removed from the scheduler with the function scheduler_remove().

Within the scheduler, the scheduled tasks should be organized as a singly linked list, which is initially empty: The scheduler does not reserve memory for the task descriptors. 
Instead, the module using the scheduler has to provide the memory to store the taskDescriptor. Thereby, the scheduler is not restricted to a fixed number of tasks. Hence, The task descriptor variable is declared in the main program module and a pointer is passed to this variable to the scheduler_add() function.

The callback for the Timer2 interrupt is used to update the scheduler every 1 ms by decreasing the expiry time of all tasks by 1 ms and mark expired tasks for execution. Additionally, the expiration time of periodic tasks should be reset here to the period.

The function scheduler_run() executes the scheduler in a superloop and is usually called from the main() function. It executes the next task marked for execution (if any), resets its execute flag and/or removes it from the list.
Any interrupt service routine that calls a scheduler function may interleave with the execution of a scheduler function from the main-loop, i.e., from a task. This can lead to an inconsistency of memory if shared variables are accessed, which can cause unexpected behavior. Therefore, the access to the list of taskDescriptors has to be restricted by disabling the interrupts in critical sections. 

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

## ATmega128RFA1 Microcontroller Configuration and Device drivers - Lib

### LED driver
Driver used for using different LEDs is implemented. It contains modular functionality for leds.
LEDs are active low (a logical high will turn them off)

### Button Driver 
Driver used for reading the states and digital input signals of buttons.
The button of the rotary encoder is connected to pin 6 of port B. The joystick button is connected to pin 7 of port B. 

All buttons trigger pin change external interrupts where it is extended with button callback functions that are executed when a valid callback was set.

### Button Debouncing
The driver also handles the button mechanical bouncing effects with a function that is polling the button state within a timer interrupt.
The function is meant to be called by a timer interrupt, so it was set as callback during the initialization. It is extended so that the corresponding button callback is called as soon as the debounced state indicates a (debounced) button press (not release).
The methodology were used for two buttons only. However, it could be used to debounce up to eight buttons in an efficient way.
A macro BUTTON_NUM_DEBOUNCE_CHECKS is defined based on “The Art of Designing Embedded Systems” by Jack Ganssle, inside the button driver with a value of 5, which introduces a delay of up to 30 ms. This is fast enough to feel instantaneous for a human eye and provides reliable debouncing.
 
### ADC driver 
Driver is implemented to convert analog input voltages to digital values.
On the board, the following peripherals are connected to the ADC:
- A **temperature sensor** at pin 2 of port F.
- A **light sensor** at pin 4 of port F.
- A **joystick** at pin 5 of port F.
- A **microphone** is connected differential to the pins 0 and 1 of port F 

All components output an analog voltage between 0 V and 1.6 V. 

### Hardware Timer Driver
Instead of busy-waiting with the **_delay_ms** function, the timing is done without blocking other operations. 
The 8-bit Timer/Counter2 is used to invoke callback asynchronous operations with different modes (Compare match, PWM).

