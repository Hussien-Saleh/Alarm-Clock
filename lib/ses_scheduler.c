/*INCLUDES ************************************************************/
#include "ses_timer.h"
#include "ses_scheduler.h"
#include "util/atomic.h"


/* PRIVATE VARIABLES **************************************************/
/** list of scheduled tasks */
static taskDescriptor* taskList = NULL;
static volatile systemTime_t currTime = 0;


/*FUNCTION DEFINITION *************************************************/
static void scheduler_update(void) {
	
	// set a pointer to the head of the linked tasklist
	taskDescriptor* tempCurrent = taskList;

	// traverse linked tasklist and update the expire time
	while(tempCurrent != NULL){

		if(tempCurrent->expire == 0){
			tempCurrent->execute = 1;	// mark expired tasks for execution
			
			// reset expiration time of periodic tasks
			if(tempCurrent->period != 0){
				tempCurrent->expire = tempCurrent->period; 
			}
		}

		(tempCurrent->expire)--;		// decrease expiry time by 1
		tempCurrent = tempCurrent->next;
	}
	// set the pointer to the head of the linked tasklist (NULL)
	tempCurrent = taskList;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		currTime++;
	}

}

void scheduler_init() {

	// set timer callback
	pTimerCallback scheduleUpdate = &scheduler_update;
	timer2_setCallback(scheduleUpdate);

	//enable global interrupts
	sei(); 

	//start timer
	timer2_start();
}

void scheduler_run() {
	
	task_t currTask = NULL; 	//task to execute
	void* paramTask = NULL; 	//parameter to pass
	bool executeTask = false; 	//execute flag
	
	while(1){

		//point to the beginning of linked list
		taskDescriptor* tempCurrent = taskList;

		///traverse the linked list
		while(tempCurrent != NULL){

			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				if (tempCurrent->execute) {

					currTask = tempCurrent->task;		//store the task
					paramTask = tempCurrent->param;		//store the parameter
					executeTask = true;					//set the execute flag
					tempCurrent->execute = 0;			//reset the flag for the task
					
					//if the task is not periodic, remove it
					if (tempCurrent->period == 0) {
						scheduler_remove(tempCurrent);
					}
				}
			
				tempCurrent = tempCurrent->next;
			}	

			// execute the task
			if(executeTask) {
				currTask(paramTask);
				executeTask = false;
			}
		}
	}
}

bool scheduler_add(taskDescriptor * toAdd) {
	//check if invalid task is added to the linkedList (NULL)
	if(toAdd == NULL){
		return false;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		//check if empty taskList
		if(taskList == NULL){
	  		//add the task
	  		taskList = toAdd; 
	  		toAdd->next = NULL;

		}else{

			//point to the beginning of linked list
			taskDescriptor* tempCurrent = taskList;

			//check if already scheduled in first node
			if(tempCurrent == toAdd){
				return false;    		
			}

			//traverse the linked list
			while(tempCurrent->next != NULL){

      			// check if the task is already scheduled
				if(tempCurrent->next == toAdd){
					return false;
				}
				tempCurrent = tempCurrent->next;		
			}

			//add the task
			tempCurrent->next = toAdd; 					
			toAdd->next = NULL;
		}
	}
	return true;
}

void scheduler_remove(taskDescriptor * toRemove) {
	//point to the beginning of linked list
	taskDescriptor* tempCurrent = taskList;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		//check if to be removed is first node
		if(taskList == toRemove){
			taskList = toRemove->next;

		}else{

			//traverse the linked list
			while(tempCurrent->next != NULL){

				//check if next task is to be removed
				if(tempCurrent->next == toRemove){
					tempCurrent->next = toRemove->next; //remove task
					break;
				}

				tempCurrent = tempCurrent->next;		
			}

		}
	}
}


systemTime_t scheduler_getTime(){
	return currTime;
}

void scheduler_setTime(systemTime_t time){

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        currTime = time;
    }
}

