#include <inttypes.h>
/****************************************************************************
 Module
     ES_Events.h
 Description
     header file with definitions for the event structure 
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:46 jec      moved event enum to config file, changed prefixes to ES
 10/23/11 22:01 jec      customized for Remote Lock problem
 10/17/11 08:47 jec      changed event prefixes to EF_ to match new framework
 08/09/06 14:30 jec      started coding
*****************************************************************************/

#ifndef ES_Events_H
#define ES_Events_H

//#include "stdint.h"
#include <inttypes.h>

typedef struct ES_Event_t {
    ES_EventTyp_t EventType;    // what kind of event?
    uint16_t   EventParam;      // parameter value for use w/ this event
}ES_Event;

#define INIT_EVENT  (ES_Event){ES_INIT,0x0000}
#define ENTRY_EVENT (ES_Event){ES_ENTRY,0x0000}
#define EXIT_EVENT  (ES_Event){ES_EXIT,0x0000}
#define NO_EVENT (ES_Event){ES_NO_EVENT,0x0000}

#endif /* ES_Events_H */

#include "stdint.h"
/****************************************************************************
 Module
         ES_Timers.h

 Revision
         1.0.1

 Description
         Header File for the ME218 Timer Module

 Notes


 History
 When           Who	What/Why
 -------------- ---	--------
 01/15/12 16:43 jec  converted for Gen2 of the Events & Services Framework
 10/21/11 18:33 jec  Begin conversion for use with the new Event Framework
 09/01/05 12:29 jec  Converted rates and return values to enumerated constants
 06/15/04 09:58 jec  converted all prefixes to EF_Timer
 05/27/04 15:04 jec  revised TMR_RATE macros for the S12
 11/24/00 13:17 jec  revised TMR_RATE macros for the HC12
 02/20/99 14:11 jec  added #defines for TMR_ERR, TMR_ACTIVE, TMR_EXPIRED
 02/24/97 14:28 jec  Began Coding
****************************************************************************/

#ifndef ES_Timers_H
#define ES_Timers_H



typedef enum { ES_Timer_ERR           = -1,
               ES_Timer_ACTIVE        =  1,
               ES_Timer_OK            =  0,
               ES_Timer_NOT_ACTIVE    =  0
} ES_TimerReturn_t;


/**
 * @Function ES_Timer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module
 * @author Max Dunne, 2011.11.15 */
void             ES_Timer_Init(void);

/**
 * @Function ES_Timer_InitTimer(uint8_t Num, uint32_t NewTime)
 * @param Num -  the number of the timer to start
 * @param NewTime - the number of tick to be counted
 * @return ERROR or SUCCESS
 * @brief  sets the NewTime into the chosen timer and clears any previous event flag
 * and sets the timer actice to begin counting.
 * @author Max Dunne 2011.11.15 */
ES_TimerReturn_t ES_Timer_InitTimer(uint8_t Num, uint32_t NewTime);

/**
 * @Function ES_Timer_SetTimer(uint8_t Num, uint32_t NewTime)
 * @param Num - the number of the timer to set.
 * @param NewTime -  the number of milliseconds to be counted
 * @return ERROR or SUCCESS
 * @brief  sets the time for a timer, but does not make it active.
 * @author Max Dunne  2011.11.15 */
ES_TimerReturn_t ES_Timer_SetTimer(uint8_t Num, uint32_t NewTime);

/**
 * @Function ES_Timer_StartTimer(uint8_t Num)
 * @param Num - the number of the timer to start
 * @return ERROR or SUCCESS
 * @brief  simply sets the active flag in TMR_ActiveFlags to resart a stopped timer.
 * @author Max Dunne, 2011.11.15 */
ES_TimerReturn_t ES_Timer_StartTimer(uint8_t Num);

/**
 * @Function ES_Timer_StopTimer(unsigned char Num)
 * @param Num - the number of the timer to stop.
 * @return ERROR or SUCCESS
 * @brief  simply clears the bit in TimerActiveFlags associated with this timer. This
 * will cause it to stop counting.
 * @author Max Dunne 2011.11.15 */
ES_TimerReturn_t ES_Timer_StopTimer(uint8_t Num);

/**
 * Function: ES_Timer_GetTime(void)
 * @param None
 * @return FreeRunningTimer - the current value of the module variable FreeRunningTimer
 * @remark Provides the ability to grab a snapshot time as an alternative to using
 * the library timers. Can be used to determine how long between 2 events.
 * @author Max Dunne, 2011.11.15  */
uint32_t         ES_Timer_GetTime(void);

#endif   /* ES_Timers_H */
/*------------------------------ End of file ------------------------------*/



/****************************************************************************
 Module
     ES_CheckEvents.h
 Description
     header file for use with the data structures to define the event checking
     functions and the function to loop through the array calling the checkers
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 12:00 jec      new header for local types
 10/16/11 17:17 jec      started coding
*****************************************************************************/

#ifndef ES_CheckEvents_H
#define ES_CheckEvents_H

#include <inttypes.h>

typedef uint8_t CheckFunc( void );

typedef CheckFunc (*pCheckFunc);

uint8_t ES_CheckUserEvents( void );


#endif  // ES_CheckEvents_H
#ifndef ES_General_H
#define ES_General_H

#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

#endif//ES_General_H
/* 
 * File:   ES_KeyboardInput.h
 * Author: mdunne
 *
 * Created on September 9, 2013, 6:47 PM
 */

#ifndef ES_KEYBOARDINPUT_H
#define	ES_KEYBOARDINPUT_H




/**
 * @Function InitKeyboardInput(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief this initializes the keyboard input system which
 *        Returns TRUE if successful, FALSE otherwise
 * @author Max Dunne , 2013.09.26 */
uint8_t InitKeyboardInput(uint8_t Priority);


/**
 * @Function PostKeyboardInput(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief Used to post events to keyboard input
 *        Returns TRUE if successful, FALSE otherwise
* @author Max Dunne , 2013.09.26 */
uint8_t PostKeyboardInput(ES_Event ThisEvent);


/**
 * @Function RunKeyboardInput(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return ES_NO_EVENT
 * @brief Keyboard input only accepts the ES_KEYINPUT event and will always return
 * ES_NO_EVENT. it parses strings of the form EVENTNUM->EVENTPARAMHEX or
 * EVENTNUM and passes them to the state machine defined by
 * POSTFUNCTION_FOR_KEYBOARD_INPUT.
 * @note WARNING: you must have created the EventNames Array to use this module
* @author Max Dunne , 2013.09.26 */
ES_Event RunKeyboardInput(ES_Event ThisEvent);


/**
 * @Function KeyboardInput_PrintEvents(void)
 * @param None
 * @return None
 * @brief  Lists out all Events in the EventNames array.
 * @author Max Dunne, 2013.09.26 */
void KeyboardInput_PrintEvents(void);



#endif	/* ES_KEYBOARDINPUT_H */


/****************************************************************************
 Module
     ES_LookupTables.h
 Description
     Extern declarations for a set of constant lookup tables that are used in
     multiple places in the framework and beyond. 
     
 Notes
     As a rule, I don't approve of global variables for a host of reasons.
     In this case I decided to make them global in the interests of
     efficiency. These tables will be references very often in the timer
     functions (as often as 8 times/ms) and on every pass through the event
     scheduler/dispatcher. As a result I decided to simply make them global.
     Since they are constant, they are not subject to the multiple access
     point issues associated with modifiable global variables.

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 13:03 jec      started coding
*****************************************************************************/
/*
  this table is used to go from a bit number (0-7) to the mask used to clear
  that bit in a byte. Technically, this is not necessary (as you could always
  complement the SetMask) but it will save the complement operation every 
  time it is used to clear a bit. If we clear a bit with it in more than 8
  places, then it is a win on code size and speed.
*/
extern uint8_t const BitNum2ClrMask[];

/*
  this table is used to go from a bit number (0-7) to the mask used to set
  that bit in a byte.
*/
extern uint8_t const BitNum2SetMask[];

/*
  this table is used to go from an unsigned 8bit value to the most significant
  bit that is set in that byte. It is used in the determination of priorities
  from the Ready variable and in determining active timers in 
  the timer interrupt response. Index into the array with (ByteVal-1) to get 
  the correct MS Bit num.
*/
extern uint8_t const Byte2MSBitNum[255];

#ifndef PORT_H
#define PORT_H

// these macros provide the wrappers for critical regions, where ints will be off
// but the state of the interrupt enable prior to entry will be restored.
extern unsigned char _CCR_temp;

#define EnterCritical()     
#define ExitCritical()      


#endif

/****************************************************************************
 Module
     EF_PostList.h
 Description
     header file for use with the module to post events to lists of state
     machines
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:57 jec      modified includes to match Events & Services
 10/16/11 12:28 jec      started coding
*****************************************************************************/
#ifndef ES_PostList_H
#define ES_PostList_H


#include <inttypes.h>

typedef uint8_t PostFunc_t( ES_Event );

typedef PostFunc_t (*pPostFunc);

uint8_t ES_PostList00( ES_Event);
uint8_t ES_PostList01( ES_Event);
uint8_t ES_PostList02( ES_Event);
uint8_t ES_PostList03( ES_Event);
uint8_t ES_PostList04( ES_Event);
uint8_t ES_PostList05( ES_Event);
uint8_t ES_PostList06( ES_Event);
uint8_t ES_PostList07( ES_Event);

#endif // ES_PostList_H
#ifndef ES_PRIOR_TABLES_H
#define ES_PRIOR_TABLES_H

uint8_t GetMSBitNum( uint8_t Value );


uint8_t GetClearMask( uint8_t BitNum );


uint8_t GetSetMask( uint8_t BitNum );
#endif

/****************************************************************************
 Module
     ES_Queue.h
 Description
     header file for use with the Queue functions of the Events  & Services 
     Framework
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 09:36 jec      converted to use new types from ES_Types.h
 10/17/11 07:49 jec      new header to match the rest of the framework
 08/09/11 09:30 jec      started coding
*****************************************************************************/
#ifndef ES_Queue_H
#define ES_Queue_H


#include <inttypes.h>
/* prototypes for public functions */

uint8_t ES_InitQueue( ES_Event * pBlock, unsigned char BlockSize );
uint8_t ES_EnQueueFIFO( ES_Event * pBlock, ES_Event Event2Add );
uint8_t ES_DeQueue( ES_Event * pBlock, ES_Event * pReturnEvent );
//void EF_FlushQueue( unsigned char * pBlock );
uint8_t ES_IsQueueEmpty( ES_Event * pBlock );

#endif /*ES_Queue_H */


/****************************************************************************
 Module
     ES_ServiceHeaders.h
 Description
     This file serves to keep the clutter down in ES_Framework.h
 Notes
 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 10:35 jec      started coding
*****************************************************************************/


#ifdef DEV
#include SERV_0_HEADER
#endif

#if NUM_SERVICES > 1
#ifdef DEV
#include SERV_1_HEADER
#endif
#endif


#if NUM_SERVICES > 2
#include SERV_2_HEADER
#endif

#if NUM_SERVICES > 3
#include SERV_3_HEADER
#endif

#if NUM_SERVICES > 4
#include SERV_4_HEADER
#endif

#if NUM_SERVICES > 5
#include SERV_5_HEADER
#endif

#if NUM_SERVICES > 6
#include SERV_6_HEADER
#endif

#if NUM_SERVICES > 7
#include SERV_7_HEADER
#endif



#ifndef ES_TattleTale_H
#define ES_TattleTale_H

#include <inttypes.h>


// Public Function Prototypes



#ifdef USE_TATTLETALE

/**
 * @Function ES_AddTattlePoint(const char * FunctionName, const char * StateName, ES_Event ThisEvent)
 * @param FunctionName - name of the function called, auto generated
 * @param StateName - Current State Name, grabbed from the the StateNames array
 * @param ThisEvent - Event passed to the function
 * @return None.
 * @brief saves pointers for current call and checks to see if a recursive call is
 * occuring
 * @note  PRIVATE FUNCTION: Do Not Call this function
 * @author Max Dunne, 2013.09.26 */
void ES_AddTattlePoint(const char * FunctionName, const char * StateName, ES_Event ThisEvent);


/**
 * @Function ES_CheckTail(const char *FunctionName)
 * @param FunctionName - name of the function called, auto generated
 * @return None.
 * @brief checks to see if system is indeed at the end of a trace and calls
 * keyboard dump if so
 * @note  PRIVATE FUNCTION: Do Not Call this function
 * @author Max Dunne, 2013.09.26 */
void ES_CheckTail(const char *FunctionName);



/**
 * @Function ES_Tattle()
 * @param None.
 * @return None.
 * @brief called at the beginning of all state machines
 * @author Max Dunne, 2013.09.26 */
#define ES_Tattle() ES_AddTattlePoint(__FUNCTION__,StateNames[CurrentState],ThisEvent)

/**
 * @Function ES_Tail()
 * @param None.
 * @return None.
 * @brief called at the end of top level state machines
 * @author Max Dunne, 2013.09.26 */
#define ES_Tail() ES_CheckTail(__FUNCTION__)
#else
#define ES_Tattle()
#define ES_Tail()
#endif


#endif 


/****************************************************************************
 
  Header file for template service 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ServTemplate_H
#define ServTemplate_H

#include <inttypes.h>



#define TIMERS_USED 2


// Public Function Prototypes
/**
 * @Function InitTimerService(uint8_t Priority)
 * @param uint8_t - the priorty of this service
 * @return uint8_t - FALSE if error in initialization, TRUE otherwise
 * @brief  Saves away the priority, and does any  other required initialization for
 * this service
 * @author Max Dunne   2013.01.04 */
uint8_t InitTimerService ( uint8_t Priority );

/**
 * @Function PostTimerService(ES_Event ThisEvent)
 * @param ThisEvent - the event to post to the queue
 * @return FALSE or TRUE
 * @brief  Posts an event to the timers queue
 * @author Max Dunne   2013.01.04 */
uint8_t PostTimerService( ES_Event ThisEvent );


/**
 * @Function RunTimerService(ES_Event ThisEvent)
 * @param ES_Event - the event to process
 * @return ES_NO_EVENT or ES_ERROR
 * @brief  accepts the timer events and updates the state arrays
 * @author Max Dunne   2013.01.04 */
ES_Event RunTimerService( ES_Event ThisEvent );



/**
 * @Function IsTimerActive(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ERROR or TRUE or FALSE
 * @brief  used to determine if a timer is currently active.
 * @author Max Dunne   2013.01.04 */
int8_t IsTimerActive(unsigned char Num);

/**
 * @Function IsTimerExpired(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ERROR or TRUE or FALSE
 * @brief  used to determine if a timer is currently expired.
 * @author Max Dunne   2013.01.04 */
int8_t IsTimerExpired(unsigned char Num);

/**
 * @Function IsTimerStopped(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ERROR or TRUE or FALSE
 * @brief  used to determine if a timer is currently stopped.
 * @author Max Dunne   2013.01.04 */
int8_t IsTimerStopped(unsigned char Num);

/**
 * @Function GetUserTimerState(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ES_EventTyp_t current state of timer
 * @brief  used to get the current timer state.
 * @author Max Dunne   2013.01.04 */
ES_EventTyp_t GetUserTimerState(unsigned char Num);


#endif /* ServTemplate_H */


/****************************************************************************
 Module
     ES_Framework.h
 Description
     header file for use with the top level functions of the ES Event Framework
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 9/14/14        MaxL    condensing into 3 files
 10/17/06 07:41 jec      started coding
*****************************************************************************/

#ifndef ES_Framework_H
#define ES_Framework_H

#include <inttypes.h>


#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

typedef enum {
              Success = 0,
              FailedPost = 1,
              FailedRun,
              FailedPointer,
              FailedIndex,
              FailedInit
} ES_Return_t;

ES_Return_t ES_Initialize( void );


ES_Return_t ES_Run( void );
uint8_t ES_PostAll( ES_Event ThisEvent );
uint8_t ES_PostToService( uint8_t WhichService, ES_Event ThisEvent);



#endif   // ES_Framework_H

