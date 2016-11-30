#include <ES_Configure.h>
#include "ES_Framework.h"
/****************************************************************************
 Module
     ES_Queue.c
 Description
     Implements a FIFO circular buffer of EF_Event in a block of memory
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 09:34 jec      converted to use the new C99 types from types.h
 08/09/11 18:16 jec      started coding
*****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include <BOARD.h>

/*----------------------------- Module Defines ----------------------------*/
// QueueSize is max number of entries in the queue
// CurrentIndex is the 'read-from' index,
// actually CurrentIndex + sizeof(EF_Queue_t)
// entries are made to CurrentIndex + NumEntries + sizeof(ES_Queue_t)
typedef struct {  unsigned char QueueSize;
                  unsigned char CurrentIndex;
                  unsigned char NumEntries;
} ES_Queue_t;

typedef ES_Queue_t * pQueue_t;

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
   ES_InitQueue
 Parameters
   EF_Event * pBlock : pointer to the block of memory to use for the Queue
   unsigned char BlockSize: size of the block pointed to by pBlock
 Returns
   max number of entries in the created queue
 Description
   Initializes a queue structure at the beginning of the block of memory
 Notes
   you should pass it a block that is at least sizeof(ES_Queue_t) larger than 
   the number of entries that you want in the queue. Since the size of an 
   ES_Event (at 4 bytes; 2 enum, 2 param) is greater than the 
   sizeof(ES_Queue_t), you only need to declare an array of ES_Event
   with 1 more element than you need for the actual queue.
 Author
   J. Edward Carryer, 08/09/11, 18:40
****************************************************************************/
uint8_t ES_InitQueue( ES_Event * pBlock, unsigned char BlockSize )
{
   pQueue_t pThisQueue;
   // initialize the Queue by setting up initial values for elements
   pThisQueue = (pQueue_t)pBlock;
   // use all but the structure overhead as the Queue
   pThisQueue->QueueSize = BlockSize - 1;
   pThisQueue->CurrentIndex = 0;
   pThisQueue->NumEntries = 0;
   return(pThisQueue->QueueSize);
}

/****************************************************************************
 Function
   ES_EnQueueFIFO
 Parameters
   ES_Event * pBlock : pointer to the block of memory in use as the Queue
   ES_Event Event2Add : event to be added to the Queue
 Returns
   uint8_t : TRUE if the add was successful, FALSE if not
 Description
   if it will fit, adds Event2Add to the Queue
 Notes

  Author
   J. Edward Carryer, 08/09/11, 18:59
****************************************************************************/
uint8_t ES_EnQueueFIFO( ES_Event * pBlock, ES_Event Event2Add )
{
   pQueue_t pThisQueue;
   pThisQueue = (pQueue_t)pBlock;
   // index will go from 0 to QueueSize-1 so use '<'
   if ( pThisQueue->NumEntries < pThisQueue->QueueSize)
   {  // save the new event, use % to create circular buffer in block
      // 1+ to step past the Queue struct at the beginning of the
      // block
      //EnterCritical();   // save interrupt state, turn ints off
      pBlock[ 1 + ((pThisQueue->CurrentIndex + pThisQueue->NumEntries)
               % pThisQueue->QueueSize)] = Event2Add;
      pThisQueue->NumEntries++;          // inc number of entries
      //ExitCritical();  // restore saved interrupt state
      
      return(TRUE);
   }else
      return(FALSE);
}


/****************************************************************************
 Function
   ES_DeQueue
 Parameters
   unsigned char * pBlock : pointer to the block of memory in use as the Queue
   ES_Event * pReturnEvent : used to return the event pulled from the queue
 Returns
   The number of entries remaining in the Queue
 Description
   pulls next available entry from Queue, EF_NO_EVENT if Queue was empty and
   copies it to *pReturnEvent.
 Notes

 Author
   J. Edward Carryer, 08/09/11, 19:11
****************************************************************************/
uint8_t ES_DeQueue( ES_Event * pBlock, ES_Event * pReturnEvent )
{
   pQueue_t pThisQueue;
   uint8_t NumLeft;

   pThisQueue = (pQueue_t)pBlock;
   if ( pThisQueue->NumEntries > 0)
   {
      EnterCritical();   // save interrupt state, turn ints off
      *pReturnEvent = pBlock[ 1 + pThisQueue->CurrentIndex ];
      // inc the index
      pThisQueue->CurrentIndex++;
      // this way we only do the modulo operation when we really need to
      if (pThisQueue->CurrentIndex >=  pThisQueue->QueueSize)
         pThisQueue->CurrentIndex = (unsigned char)(pThisQueue->CurrentIndex % pThisQueue->QueueSize);
      //dec number of elements since we took 1 out
      NumLeft = --pThisQueue->NumEntries; 
      ExitCritical();  // restore saved interrupt state
   }else { // no items left in the queue
      (*pReturnEvent).EventType = ES_NO_EVENT;
      (*pReturnEvent).EventParam = 0;
      NumLeft = 0;
   }
   return NumLeft;
}

/****************************************************************************
 Function
   ES_IsQueueEmpty
 Parameters
   unsigned char * pBlock : pointer to the block of memory in use as the Queue
 Returns
   uint8_t : TRUE if Queue is empty
 Description
   see above
 Notes

 Author
   J. Edward Carryer, 08/10/11, 13:29
****************************************************************************/
uint8_t ES_IsQueueEmpty( ES_Event * pBlock )
{
   pQueue_t pThisQueue;

   pThisQueue = (pQueue_t)pBlock;
   return(pThisQueue->NumEntries == 0);
}

#if 0
/****************************************************************************
 Function
   QueueFlushQueue
 Parameters
   unsigned char * pBlock : pointer to the block of memory in use as the Queue
 Returns
   nothing
 Description
   flushes the Queue by reinitializing the indecies
 Notes

 Author
   J. Edward Carryer, 08/12/06, 19:24
****************************************************************************/
void QueueFlushQueue( unsigned char * pBlock )
{
   pQueue_t pThisQueue;
   // doing this with a Queue structure is not strictly necessary
   // but makes it clearer what is going on.
   pThisQueue = (pQueue_t)pBlock;
   pThisQueue->CurrentIndex = 0;
   pThisQueue->NumEntries = 0;
   return;
}


#endif
/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

























































//#define TEST
/****************************************************************************
 Module
     ES_Timers.c

 Description
     This is a module implementing  8 16 bit timers all using the RTI
     timebase

 Notes
     Everything is done in terms of RTI Ticks, which can change from
     application to application.

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:42 jec      added some more error checking to start & init
                         funcs to prevent starting a timer with no
                         service attached or with a time of 0
 01/15/12 16:46 jec      convert to Gen2 of Events & Services framework
 10/21/11 18:26 jec      begin conversion to work with the new Event Framework
 09/01/05 13:16 jec      converted the return types and parameters to use the
                         enumerated constants from the new header.
 08/31/05 10:23 jec      converted several return value tests in the test harness
                         to use symbolic values.
 06/15/04 09:56 jec      converted all external prefixes to TMRS12 to be sure
                         that we don't have any conflicts with the old libs
 05/28/04 13:53 jec      converted for 9S12C32 processor
 12/11/02 14:53 dos      converted for ICC11V6, unadorned char needs to be
                         called out as signed char, default is now unsigned
                         for a plain char.
 11/24/99 14:45 rmo		 updated to compile under ICC11v5.
 02/24/97 17:13 jec      added new function TMR_SetTimer. This will allow one
                                     function to set up the time, while another function
                                 actually initiates the timing.
 02/24/97 13:34 jec      Began Coding
 ****************************************************************************/

/*----------------------------- Include Files -----------------------------*/

#include <xc.h>
#include <peripheral/timer.h>
/*--------------------------- External Variables --------------------------*/

/*----------------------------- Module Defines ----------------------------*/
#define BITS_PER_BYTE 8
#define F_CPU 80000000L
#define F_PB F_CPU/2
#define TIMER_FREQUENCY 1000

#define NUM_TIMERS 16
#define TranslatePin(x) (1<<x)
/*------------------------------ Module Types -----------------------------*/



/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
static unsigned int TMR_TimerArray[NUM_TIMERS];

// make this one const to get it put into flash, since it will never change


static unsigned int TMR_ActiveFlags;
static uint32_t FreeRunningTimer; /* this is used by the default RTI routine */

static pPostFunc const Timer2PostFunc[NUM_TIMERS] = {TIMER0_RESP_FUNC,
    TIMER1_RESP_FUNC,
    TIMER2_RESP_FUNC,
    TIMER3_RESP_FUNC,
    TIMER4_RESP_FUNC,
    TIMER5_RESP_FUNC,
    TIMER6_RESP_FUNC,
    TIMER7_RESP_FUNC,
    TIMER8_RESP_FUNC,
    TIMER9_RESP_FUNC,
    TIMER10_RESP_FUNC,
    TIMER11_RESP_FUNC,
    TIMER12_RESP_FUNC,
    TIMER13_RESP_FUNC,
    TIMER14_RESP_FUNC,
    TIMER15_RESP_FUNC};




/*------------------------------ Module Code ------------------------------*/

/**
 * @Function ES_Timer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module
 * @author Max Dunne, 2011.11.15 */
 void ES_Timer_Init(void) {
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, F_PB / TIMER_FREQUENCY);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_3);

    mT1IntEnable(1);
}

/**
 * @Function ES_Timer_SetTimer(uint8_t Num, uint32_t NewTime)
 * @param Num - the number of the timer to set.
 * @param NewTime -  the number of milliseconds to be counted
 * @return ERROR or SUCCESS
 * @brief  sets the time for a timer, but does not make it active.
 * @author Max Dunne  2011.11.15 */
ES_TimerReturn_t ES_Timer_SetTimer(uint8_t Num, uint32_t NewTime) {
    static ES_Event NewEvent;
    // tried to set a timer that doesn't exist
    if ((Num >= NUM_TIMERS) || (Timer2PostFunc[Num] == TIMER_UNUSED) || (NewTime == 0)) {
        return ES_Timer_ERR;
    }
    TMR_TimerArray[Num] = NewTime;
    return ES_Timer_OK;
}

/**
 * @Function ES_Timer_StartTimer(uint8_t Num)
 * @param Num - the number of the timer to start
 * @return ERROR or SUCCESS
 * @brief  simply sets the active flag in TMR_ActiveFlags to resart a stopped timer.
 * @author Max Dunne, 2011.11.15 */
ES_TimerReturn_t ES_Timer_StartTimer(uint8_t Num) {
    static ES_Event NewEvent;
    // tried to set a timer that doesn't exist
    if ((Num >= NUM_TIMERS) || (TMR_TimerArray[Num] == 0)) {
        return ES_Timer_ERR;
    }
    TMR_ActiveFlags |= TranslatePin(Num); /* set timer as active */
    NewEvent.EventType = ES_TIMERACTIVE;
    NewEvent.EventParam = Num;
    // post the timeout event to the right Service
    Timer2PostFunc[Num](NewEvent);
    return ES_Timer_OK;
}

/**
 * @Function ES_Timer_StopTimer(unsigned char Num)
 * @param Num - the number of the timer to stop.
 * @return ERROR or SUCCESS
 * @brief  simply clears the bit in TimerActiveFlags associated with this timer. This 
 * will cause it to stop counting.
 * @author Max Dunne 2011.11.15 */
ES_TimerReturn_t ES_Timer_StopTimer(unsigned char Num) {
    static ES_Event NewEvent;
    if ((Num >= NUM_TIMERS) || (Timer2PostFunc[Num] == TIMER_UNUSED) || !(TMR_ActiveFlags & TranslatePin(Num))) {
        return ES_Timer_ERR; // tried to set a timer that doesn't exist
    }
    TMR_ActiveFlags &= ~(TranslatePin(Num)); // set timer as inactive
    NewEvent.EventType = ES_TIMERSTOPPED;
    NewEvent.EventParam = Num;
    // post the timeout event to the right Service
    Timer2PostFunc[Num](NewEvent);
    return ES_Timer_OK;
}

/**
 * @Function ES_Timer_InitTimer(uint8_t Num, uint32_t NewTime)
 * @param Num -  the number of the timer to start
 * @param NewTime - the number of tick to be counted
 * @return ERROR or SUCCESS
 * @brief  sets the NewTime into the chosen timer and clears any previous event flag 
 * and sets the timer actice to begin counting.
 * @author Max Dunne 2011.11.15 */
ES_TimerReturn_t ES_Timer_InitTimer(uint8_t Num, uint32_t NewTime) {
    static ES_Event NewEvent;
    if ((Num >= NUM_TIMERS) || (Timer2PostFunc[Num] == TIMER_UNUSED) || (NewTime == 0)) {
        return ES_Timer_ERR;
    }
    TMR_TimerArray[Num] = NewTime;
    TMR_ActiveFlags |= TranslatePin(Num); /* set timer as active */
    NewEvent.EventType = ES_TIMERACTIVE;
    NewEvent.EventParam = Num;
    // post the timeout event to the right Service
    Timer2PostFunc[Num](NewEvent);
    return ES_Timer_OK;
}

/**
 * Function: ES_Timer_GetTime(void)
 * @param None
 * @return FreeRunningTimer - the current value of the module variable FreeRunningTimer
 * @remark Provides the ability to grab a snapshot time as an alternative to using
 * the library timers. Can be used to determine how long between 2 events.
 * @author Max Dunne, 2011.11.15  */
uint32_t ES_Timer_GetTime(void) {
    return (FreeRunningTimer);
}

/****************************************************************************
 Function
     ES_Timer_RTI_Resp
 Parameters
     None.
 Returns
     None.
 Description
     This is the new RTI response routine to support the timer module.
     It will increment time, to maintain the functionality of the
     GetTime() timer and it will check through the active timers,
     decrementing each active timers count, if the count goes to 0, it
     will post an event to the corresponding SM and clear the active flag to
     prevent further counting.
 Notes
     None.
 Author
     J. Edward Carryer, 02/24/97 15:06
 ****************************************************************************/
void __ISR(_TIMER_1_VECTOR, ipl3auto) Timer1IntHandler(void) {
    static ES_Event NewEvent;
    uint8_t CurTimer = 0;
    mT1ClearIntFlag();
#ifdef USE_KEYBOARD_INPUT
    return;
#endif
    ++FreeRunningTimer; // keep the GetTime() timer running 
    if (TMR_ActiveFlags != 0) {
        for (CurTimer = 0; CurTimer < NUM_TIMERS; CurTimer++) {
            if ((TMR_ActiveFlags & (TranslatePin(CurTimer))) != 0) {
                if (--TMR_TimerArray[CurTimer] == 0) {
                    NewEvent.EventType = ES_TIMEOUT;
                    NewEvent.EventParam = CurTimer;
                    // post the timeout event to the right Service
                    Timer2PostFunc[CurTimer](NewEvent);
                    // and stop counting
                    TMR_ActiveFlags &= ~(TranslatePin(CurTimer));
                }

            }
        }

    }
}
/*------------------------------- Footnotes -------------------------------*/
#ifdef TEST

#include <termio.h>
#include <stdio.h>
#include <timerS12.h>


#define TIME_OUT_DELAY 1221
signed char Message[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x0d, 0};

void main(void) {
    int p;
    printf("\rStarting, should delay for 10 seconds\r");
    EF_Timer_Init(EF_Timer_RATE_8MS);
    EF_Timer_InitTimer(0, TIME_OUT_DELAY); /* TIME_OUT_DELAY  = 10s w/ 8.19mS interval */
    EF_Timer_InitTimer(1, TIME_OUT_DELAY);
    EF_Timer_InitTimer(2, TIME_OUT_DELAY);
    EF_Timer_InitTimer(3, TIME_OUT_DELAY);
    EF_Timer_InitTimer(4, TIME_OUT_DELAY);
    EF_Timer_InitTimer(5, TIME_OUT_DELAY);
    EF_Timer_InitTimer(6, TIME_OUT_DELAY);
    EF_Timer_InitTimer(7, TIME_OUT_DELAY);
    while (EF_Timer_IsTimerExpired(0) != EF_Timer_EXPIRED)
        ;
    printf("Timed Out\r");

    EF_Timer_InitTimer(7, TIME_OUT_DELAY);
    for (p = 0; p < 10000; p++); /* kill some time */
    EF_Timer_StopTimer(7);
    if (EF_Timer_IsTimerActive(7) != EF_Timer_NOT_ACTIVE)
        printf("Timer Stop Failed\r");
    else
        printf("Timer Stop Succeded, restarting timeout\r");
    EF_Timer_StartTimer(7);
    while (EF_Timer_IsTimerExpired(7) != EF_Timer_EXPIRED)
        ;

    printf("Timed Out Again\r");

    DisableInterrupts;
}
#endif
/*------------------------------ End of file ------------------------------*/


/****************************************************************************
 Module
     EF_PostList.c
 Description
     source file for the module to post events to lists of state
     machines
 Notes
     
 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 15:55 jec      re-coded for Gen2 with conditional declarations
 10/16/11 12:32 jec      started coding
*****************************************************************************/
/*----------------------------- Include Files -----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
static uint8_t PostToList(  PostFunc_t *const*FuncList, unsigned char ListSize, ES_Event NewEvent);

/*---------------------------- Module Variables ---------------------------*/
// Fill in these arrays with the lists of posting funcitons for the state
// machines that will have common events delivered to them.

#if NUM_DIST_LISTS > 0
static PostFunc_t * const DistList00[] = {DIST_LIST0 };
// the endif for NUM_DIST_LISTS > 0 is at the end of the file
#if NUM_DIST_LISTS > 1
static PostFunc_t * const DistList01[] = {DIST_LIST1 };
#endif
#if NUM_DIST_LISTS > 2
static PostFunc_t * const DistList02[] = {DIST_LIST2 };
#endif
#if NUM_DIST_LISTS > 3
static PostFunc_t * const DistList03[] = {DIST_LIST3 };
#endif
#if NUM_DIST_LISTS > 4
static PostFunc_t * const DistList04[] = {DIST_LIST4 };
#endif
#if NUM_DIST_LISTS > 5
static PostFunc_t * const DistList05[] = {DIST_LIST5 };
#endif
#if NUM_DIST_LISTS > 6
static PostFunc_t * const DistList06[] = {DIST_LIST6 };
#endif
#if NUM_DIST_LISTS > 7
static PostFunc_t * const DistList07[] = {DIST_LIST7 };
#endif

/*------------------------------ Module Code ------------------------------*/

// Each of these list-specific functions is a wrapper that calls the generic
// function to walk through the list, calling the listed posting functions

/****************************************************************************
 Function
   PostListxx
 Parameters
   ES_Event NewEvent : the new event to be passed to each of the state machine
   posting functions in list xx
 Returns
   TRUE if all the post functions succeeded, FALSE if any failed
 Description
   Posts NewEvent to all of the state machines listed in the list
 Notes
   
 Author
   J. Edward Carryer, 10/24/11, 07:48
****************************************************************************/
uint8_t ES_PostList00( ES_Event NewEvent) {
  return PostToList( DistList00, ARRAY_SIZE(DistList00), NewEvent);
}

#if NUM_DIST_LISTS > 1
uint8_t ES_PostList01( ES_Event NewEvent) {
  return PostToList( DistList01, ARRAY_SIZE(DistList01), NewEvent);
}
#endif

#if NUM_DIST_LISTS > 2
uint8_t ES_PostList02( ES_Event NewEvent) {
  return PostToList( DistList02, ARRAY_SIZE(DistList02), NewEvent);
}
#endif

#if NUM_DIST_LISTS > 3
uint8_t ES_PostList03( ES_Event NewEvent) {
  return PostToList( DistList03, ARRAY_SIZE(DistList03), NewEvent);
}
#endif

#if NUM_DIST_LISTS > 4
uint8_t ES_PostList04( ES_Event NewEvent) {
  return PostToList( DistList04, ARRAY_SIZE(DistList04), NewEvent);
}
#endif

#if NUM_DIST_LISTS > 5
uint8_t ES_PostList05( ES_Event NewEvent) {
  return PostToList( DistList05, ARRAY_SIZE(DistList05), NewEvent);
}
#endif

#if NUM_DIST_LISTS > 6
uint8_t ES_PostList06( ES_Event NewEvent) {
  return PostToList( DistList06, ARRAY_SIZE(DistList06), NewEvent);
}
#endif

#if NUM_DIST_LISTS > 7
uint8_t ES_PostList07( ES_Event NewEvent) {
  return PostToList( DistList07, ARRAY_SIZE(DistList07), NewEvent);
}
#endif

// Implementations for private functions
/****************************************************************************
 Function
   PostToList
 Parameters
   PostFunc *const*List : pointer to the list of posting functions
   unsigned char ListSize : number of elements in the list array 
   EF_Event NewEvent : the new event to be passed to each of the state machine
   posting functions in the list
 Returns
   TRUE if all the post functions succeeded, FALSE if any failed
 Description
   Posts NewEvent to all of the state machines listed in the list
 Notes
   
 Author
   J. Edward Carryer, 10/24/11, 07:52
****************************************************************************/
static uint8_t PostToList( PostFunc_t *const*List, unsigned char ListSize, ES_Event NewEvent){
  unsigned char i;
  // loop through the list executing the post functions
  for ( i=0; i< ListSize; i++) {
    if ( List[i](NewEvent) != TRUE )
      break; // this is a failed post
  }
  if ( i != ListSize ) // if no failures, i = ListSize
    return (FALSE);
  else
    return(TRUE);
}
#endif /* NUM_DIST_LISTS > 0*/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

/****************************************************************************
 Module
     ES_Framework.c
 Description
     source file for the core functions of the Events & Services framework
 Notes
     
 History
 When           Who     What/Why
 -------------- ---     --------
 * 9/14/14      maxl    condesning into 3 files
 01/30/12 19:31 jec      moved call to ES_InitTimers into the ES_Initialize
                         this rewuired adding a parameter to ES_Initialize.
 01/15/12 12:15 jec      major revision for Gen2
 10/17/11 12:24 jec      started coding
 *****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include <stdio.h>
#include <BOARD.h>
//#include <termio.h>

// Include the header files for the state machines.
// This gets you the prototypes for the public state machine functions.

#include "serial.h"


/*----------------------------- Module Defines ----------------------------*/
typedef uint8_t InitFunc_t(uint8_t Priority);
typedef ES_Event RunFunc_t(ES_Event ThisEvent);

typedef InitFunc_t * pInitFunc;
typedef RunFunc_t * pRunFunc;

#define NULL_INIT_FUNC ((pInitFunc)0)

typedef struct {
    InitFunc_t *InitFunc; // Service Initialization function
    RunFunc_t *RunFunc; // Service Run function
} ES_ServDesc_t;

typedef struct {
    ES_Event *pMem; // pointer to the memory
    uint8_t Size; // how big is it
} ES_QueueDesc_t;

/*---------------------------- Module Functions ---------------------------*/
static uint8_t CheckSystemEvents(void);

/*---------------------------- Module Variables ---------------------------*/
/****************************************************************************/
// You fill in this array with the names of the service init & run functions
// for each service that you use.
// The order is: InitFunction, RunFunction
// The first enry, at index 0, is the lowest priority, with increasing 
// priority with higher indices

static ES_ServDesc_t const ServDescList[] = {
    {SERV_0_INIT, SERV_0_RUN} /* lowest priority  always present */
#if NUM_SERVICES > 1
    ,
    {SERV_1_INIT, SERV_1_RUN}
#endif
#if NUM_SERVICES > 2
    ,
    {SERV_2_INIT, SERV_2_RUN}
#endif
#if NUM_SERVICES > 3
    ,
    {SERV_3_INIT, SERV_3_RUN}
#endif
#if NUM_SERVICES > 4
    ,
    {SERV_4_INIT, SERV_4_RUN}
#endif
#if NUM_SERVICES > 5
    ,
    {SERV_5_INIT, SERV_5_RUN}
#endif
#if NUM_SERVICES > 6
    ,
    {SERV_6_INIT, SERV_6_RUN}
#endif
#if NUM_SERVICES > 7
    ,
    {SERV_7_INIT, SERV_7_RUN}
#endif

};

/****************************************************************************/
// Initialize this variable with the name of the posting function that you
// want executed when a new keystroke is detected.

static pPostFunc const pPostKeyFunc = POST_KEY_FUNC;

/****************************************************************************/
// The queues for the services

static ES_Event Queue0[SERV_0_QUEUE_SIZE + 1];
#if NUM_SERVICES > 1
static ES_Event Queue1[SERV_1_QUEUE_SIZE + 1];
#endif
#if NUM_SERVICES > 2
static ES_Event Queue2[SERV_2_QUEUE_SIZE + 1];
#endif
#if NUM_SERVICES > 3
static ES_Event Queue3[SERV_3_QUEUE_SIZE + 1];
#endif
#if NUM_SERVICES > 4
static ES_Event Queue4[SERV_4_QUEUE_SIZE + 1];
#endif
#if NUM_SERVICES > 5
static ES_Event Queue5[SERV_5_QUEUE_SIZE + 1];
#endif
#if NUM_SERVICES > 6
static ES_Event Queue6[SERV_6_QUEUE_SIZE + 1];
#endif
#if NUM_SERVICES > 7
static ES_Event Queue7[SERV_7_QUEUE_SIZE + 1];
#endif

/****************************************************************************/
// array of queue descriptors for posting by priority level

static ES_QueueDesc_t const EventQueues[NUM_SERVICES] = {
    { Queue0, ARRAY_SIZE(Queue0)}
#if NUM_SERVICES > 1
    ,
    { Queue1, ARRAY_SIZE(Queue1)}
#endif
#if NUM_SERVICES > 2
    ,
    { Queue2, ARRAY_SIZE(Queue2)}
#endif
#if NUM_SERVICES > 3
    ,
    { Queue3, ARRAY_SIZE(Queue3)}
#endif
#if NUM_SERVICES > 4
    ,
    { Queue4, ARRAY_SIZE(Queue4)}
#endif
#if NUM_SERVICES > 5
    ,
    { Queue5, ARRAY_SIZE(Queue5)}
#endif
#if NUM_SERVICES > 6
    ,
    { Queue6, ARRAY_SIZE(Queue6)}
#endif
#if NUM_SERVICES > 7
    ,
    { Queue7, ARRAY_SIZE(Queue7)}
#endif
};

/****************************************************************************/
// Variable used to keep track of which queues have events in them

uint8_t Ready;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
   ES_Initialize
 Parameters
   None
 Returns
   ES_Return_t : FailedPointer if any of the function pointers are NULL
                 FailedInit if any of the initialization functions failed
 Description
   Initialize all the services and tests for NULL pointers in the array
 Notes

 Author
   J. Edward Carryer, 10/23/11,
 ****************************************************************************/
ES_Return_t ES_Initialize(void) {
    unsigned char i;
    ES_Timer_Init(); // start up the timer subsystem
    // loop through the list testing for NULL pointers and
    for (i = 0; i < ARRAY_SIZE(ServDescList); i++) {
        if ((ServDescList[i].InitFunc == (pInitFunc) 0) ||
                (ServDescList[i].RunFunc == (pRunFunc) 0))
            return FailedPointer; // protect against NULL pointers
        // and initializing the event queues (must happen before running inits)
        ES_InitQueue(EventQueues[i].pMem, EventQueues[i].Size);
        // executing the init functions
        if (ServDescList[i].InitFunc(i) != TRUE)
            return FailedInit; // this is a failed initialization
    }
    return Success;
}

/****************************************************************************
 Function
   ES_Run
 Parameters
   None
 *
 Returns
   ES_Return_t : FailedRun is any of the run functions failed during execution
 Description
   This is the main framework function. It searches through the state
   machines to find one with a non-empty queue and then executes the
   state machine to process the event in its queue.
   while all the queues are empty, it searches for system generated or
   user generated events.
 Notes
   this function only returns in case of an error
 Author
   J. Edward Carryer, 10/23/11,
   M. Dunne, 2013.09.18
 ****************************************************************************/
ES_Return_t ES_Run(void) {
    // make these static to improve speed
    uint8_t HighestPrior;
    static ES_Event ThisEvent;
    uint8_t CurService;
    uint8_t CurServiceMask;

    while (1) { // stay here unless we detect an error condition

        // loop through the list executing the run functions for services
        // with a non-empty queue
        while (Ready != 0) {
            //if (IsTransmitEmpty()) {
                 //printf("handling queues: %X\r\n", Ready);
            //}
            for (CurService = 0; CurService < NUM_SERVICES; CurService++) {
                CurServiceMask = 1 << CurService;
                //printf("handling queue: %X: %X: %X\r\n", CurService,Ready,Ready & CurServiceMask);
                if (Ready & CurServiceMask) {
                    if (ES_DeQueue(EventQueues[CurService].pMem, &ThisEvent) == 0) {
                        Ready &= ~CurServiceMask; // mark queue as now empty
                    }
                    if (ServDescList[CurService].RunFunc(ThisEvent).EventType == ES_ERROR) {
                        return FailedRun;
                    }
                }
            }
        }
        // all the queues are empty, so look for new system or user detected events
        if (CheckSystemEvents() == FALSE)
#ifndef USE_KEYBOARD_INPUT
            ES_CheckUserEvents();
#else
            ;
#endif

    }
}

/****************************************************************************
 Function
   ES_PostAll
 Parameters
   ES_Event : The Event to be posted
 Returns
   uint8_t : FALSE if any of the post functions failed during execution
 Description
   posts to all of the services' queues 
 Notes

 Author
   J. Edward Carryer, 01/15/12,
 ****************************************************************************/
uint8_t ES_PostAll(ES_Event ThisEvent) {

    unsigned char i;
    // loop through the list executing the post functions
    for (i = 0; i < ARRAY_SIZE(EventQueues); i++) {
        if (ES_EnQueueFIFO(EventQueues[i].pMem, ThisEvent) != TRUE) {
            break; // this is a failed post
        } else {
            Ready |= (1 << i); // show queue as non-empty
        }
    }
    if (i == ARRAY_SIZE(EventQueues)) { // if no failures
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/****************************************************************************
 Function
   ES_PostToService
 Parameters
   uint8_t : Which service to post to (index into ServDescList)
   ES_Event : The Event to be posted
 Returns
   uint8_t : FALSE if the post function failed during execution
 Description
   posts to one of the services' queues
 Notes
   used by the timer library to associate a timer with a state machine
 Author
   J. Edward Carryer, 01/16/12,
 ****************************************************************************/
uint8_t ES_PostToService(uint8_t WhichService, ES_Event TheEvent) {
    if ((WhichService < ARRAY_SIZE(EventQueues)) &&
            (ES_EnQueueFIFO(EventQueues[WhichService].pMem, TheEvent) ==
            TRUE)) {
        Ready |= (1 << WhichService); // show queue as non-empty
        return TRUE;
    } else
        return FALSE;
}


//*********************************
// private functions
//*********************************

/****************************************************************************
 Function
   CheckSystemEvents
 Parameters
   None
 Returns
   uint8_t : TRUE if a system event was detected
 Description
   check for system generated events and uses pPostKeyFunc to post to one
   of the state machine's queues
 Notes
   currently only tests for incoming keystrokes
 Author
   J. Edward Carryer, 10/23/11, 
 ****************************************************************************/
static uint8_t CheckSystemEvents(void) {

    //  if ( kbhit() != 0 ) // new key waiting?
    //  {
    //    ES_Event ThisEvent;
    //    ThisEvent.EventType = ES_NEW_KEY;
    //    ThisEvent.EventParam = getchar();
    //    (*pPostKeyFunc)( ThisEvent );
    //    return TRUE;
    //  }
#ifdef USE_KEYBOARD_INPUT
    if (!IsReceiveEmpty()) {
        ES_Event ThisEvent;
        ThisEvent.EventType = ES_KEYINPUT;
        ThisEvent.EventParam = GetChar();
        PostKeyboardInput(ThisEvent);
        return TRUE;
    }
#endif
    return FALSE;
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

/****************************************************************************
 Module
   TemplateService.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
 ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */

#include <BOARD.h>
#include "serial.h"
#include <stdio.h>
#include <peripheral/timer.h>

#ifdef USE_TATTLETALE

/*----------------------------- Module Defines ----------------------------*/


#define TATTLE_POINTS 30

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */

void ES_TattleTaleDump(void);


typedef struct {
    //int16_t superState;
    //int16_t subStates[TATTLE_SUBSTATE_DEPTH];
    const char* FunctionName;
    const char *StateName;
    ES_Event Event;


} TattleDataPoint;

static TattleDataPoint TattleData[TATTLE_POINTS];

static uint8_t tattleCount = 0;
static uint8_t tattleDepth = 0;
static uint8_t tattleRecursiveCall = FALSE;
static uint8_t tattleRecursiveCount = 0;
/*------------------------------ Module Code ------------------------------*/





/***************************************************************************
 private functions
 ***************************************************************************/

/**
 * @Function ES_TattleTaleDump(void)
 * @param None.
 * @return None.
 * @brief Dumps all events caught by tattle and tail in one trace.
 * @note  PRIVATE FUNCTION: do not call this function
 * @author Max Dunne, 2013.09.26 */
void ES_TattleTaleDump(void)
{
    uint8_t curDataPoint = 0;
    tattleDepth = 0;
    T1CONCLR = _T1CON_ON_MASK;
    for (curDataPoint = 0; curDataPoint < tattleCount; curDataPoint++) {
#ifdef SUPPRESS_EXIT_ENTRY_IN_TATTLE
        if ((TattleData[curDataPoint].Event.EventType != ES_ENTRY) || (TattleData[curDataPoint].Event.EventType != ES_EXIT)) {
#endif
            printf("%s[%s(%s,%X)]", TattleData[curDataPoint].FunctionName, TattleData[curDataPoint].StateName,          \
                        EventNames[TattleData[curDataPoint].Event.EventType], TattleData[curDataPoint].Event.EventParam);
            if (curDataPoint < (tattleCount - 1)) {
                printf("->");
            } else {
                printf(";");
            }
            while (!IsTransmitEmpty());
#ifdef SUPPRESS_EXIT_ENTRY_IN_TATTLE
        }
#endif
    }
    printf("\n");
    tattleCount = 0;
    T1CONSET = _T1CON_ON_MASK;
}

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
void ES_AddTattlePoint(const char * FunctionName, const char * StateName, ES_Event ThisEvent)
{
    if (tattleCount < TATTLE_POINTS) {
        TattleData[tattleCount].FunctionName = FunctionName;
        TattleData[tattleCount].StateName = StateName;
        TattleData[tattleCount].Event = ThisEvent;
        tattleCount++;
    }
    if ((ThisEvent.EventType == ES_EXIT) && (FunctionName == TattleData[0].FunctionName)) {
        tattleRecursiveCall = TRUE;
    }
}



/**
 * @Function ES_CheckTail(const char *FunctionName)
 * @param FunctionName - name of the function called, auto generated
 * @return None.
 * @brief checks to see if system is indeed at the end of a trace and calls
 * keyboard dump if so
 * @note  PRIVATE FUNCTION: Do Not Call this function
 * @author Max Dunne, 2013.09.26 */
void ES_CheckTail(const char *FunctionName)
{
    if (TattleData[0].FunctionName == FunctionName) {
        if ((!tattleRecursiveCall) && (TattleData[tattleCount - 1].Event.EventType != ES_EXIT)) {
            ES_TattleTaleDump();
            return;
        }
        if (tattleRecursiveCall) {
            tattleRecursiveCount++;
        }
        if (tattleRecursiveCount > 2) {
            tattleRecursiveCount = 0;
            tattleRecursiveCall = FALSE;
            ES_TattleTaleDump();
        }
    }
}
#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


/****************************************************************************
 Module
     ES_CheckEvents.c
 Description
     source file for the module to call the User event checking routines
 Notes
     Users should only modify the contents of the ES_EventList array.
 History
 When           Who     What/Why
 -------------- ---     --------
 10/16/11 12:32 jec      started coding
*****************************************************************************/

#include <BOARD.h>

// Include the header files for the module(s) with your event checkers. 
// This gets you the prototypes for the event checking functions.

#include EVENT_CHECK_HEADER

// Fill in this array with the names of your event checking functions

static CheckFunc * const ES_EventList[]={EVENT_CHECK_LIST };


// Implementation for public functions

/****************************************************************************
 Function
   ES_CheckUserEvents
 Parameters
   None
 Returns
   TRUE if any of the user event checkers returned TRUE, FALSE otherwise
 Description
   loop through the EF_EventList array executing the event checking functions
 Notes
   
 Author
   J. Edward Carryer, 10/25/11, 08:55
****************************************************************************/
uint8_t ES_CheckUserEvents( void ) 
{
  unsigned char i;
  // loop through the array executing the event checking functions
  for ( i=0; i< ARRAY_SIZE(ES_EventList); i++) {
    if ( ES_EventList[i]() == TRUE )
      break; // found a new event, so process it first
  }
  if ( i == ARRAY_SIZE(ES_EventList) ) // if no new events
    return (FALSE);
  else
    return(TRUE);
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

/****************************************************************************
 Module
   TemplateService.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
 ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */
#include <BOARD.h>

#ifdef TIMER_SERVICE_TEST
#include <stdio.h>

static enum {
    init, expired, runstop
} timerServiceTestingState;
#endif

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static ES_EventTyp_t UserTimerStates[TIMERS_USED];


/*------------------------------ Module Code ------------------------------*/

/**
 * @Function InitTimerService(uint8_t Priority)
 * @param uint8_t - the priorty of this service
 * @return uint8_t - FALSE if error in initialization, TRUE otherwise
 * @brief  Saves away the priority, and does any  other required initialization for 
 * this service
 * @author Max Dunne   2013.01.04 */
uint8_t InitTimerService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }

}

/**
 * @Function PostTimerService(ES_Event ThisEvent)
 * @param ThisEvent - the event to post to the queue
 * @return FALSE or TRUE
 * @brief  Posts an event to the timers queue
 * @author Max Dunne   2013.01.04 */
uint8_t PostTimerService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTimerService(ES_Event ThisEvent)
 * @param ES_Event - the event to process
 * @return ES_NO_EVENT or ES_ERROR 
 * @brief  accepts the timer events and updates the state arrays
 * @author Max Dunne   2013.01.04 */
ES_Event RunTimerService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (ThisEvent.EventType) {
        case ES_INIT:
            break;
        case ES_TIMEOUT:
        case ES_TIMERACTIVE:
        case ES_TIMERSTOPPED:
            UserTimerStates[ThisEvent.EventParam] = ThisEvent.EventType;
            break;
        default:
            //ReturnEvent.EventType = ES_ERROR;
            break;
    }

#ifdef TIMER_SERVICE_TEST
    {
        uint8_t i;
        switch (timerServiceTestingState) {
            case init:
                if (ThisEvent.EventType == ES_INIT) {
                    printf("Timer Module INITED succesfully\r\n");
                    break;
                }
                printf("Timer %d had event %d at time %d\r\n", ThisEvent.EventParam, ThisEvent.EventType, ES_Timer_GetTime());
                if ((ThisEvent.EventParam == (TIMERS_USED - 1)) && (ThisEvent.EventType == ES_TIMERACTIVE)) {
                    timerServiceTestingState = expired;
                    printf("Testing timer user functions [expired][stopped][active]{state}\r\n");
                }
                break;

            case expired:
                for (i = 0; i < TIMERS_USED; i++) {
                    printf("(%d):[%d,%d,%d]{%d} ", i, IsTimerExpired(i), IsTimerStopped(i), IsTimerActive(i), GetUserTimerState(i));
                }
                printf("\r\n");
                if ((ThisEvent.EventParam == (TIMERS_USED - 1)) && (ThisEvent.EventType == ES_TIMEOUT)) {
                    timerServiceTestingState = runstop;
                    ES_Timer_InitTimer(0, 500);
                    for (i = 1; i < TIMERS_USED; i++) {
                        ES_Timer_SetTimer(i, 1000);
                        ES_Timer_StartTimer(i);
                    }
                }
                break;

            case runstop:
                printf("Timer %d had event %d at time %d\r\n", ThisEvent.EventParam, ThisEvent.EventType, ES_Timer_GetTime());
                if ((ThisEvent.EventParam == (0)) && (ThisEvent.EventType == ES_TIMEOUT)) {
                    for (i = 1; i < TIMERS_USED; i++) {
                        ES_Timer_StopTimer(i);
                    }
                }
                if ((ThisEvent.EventParam == (TIMERS_USED - 1)) && (ThisEvent.EventType == ES_TIMERSTOPPED)) {
                    printf("Testing of User Timer Functions is complete.\r\n");
                }

                break;
            default:
                ReturnEvent.EventType = ES_ERROR;
                break;
        }

    }
#endif
    //ES_Tail();
    return ReturnEvent;
}

/**
 * @Function IsTimerExpired(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ERROR or TRUE or FALSE 
 * @brief  used to determine if a timer is currently expired.
 * @author Max Dunne   2013.01.04 */
int8_t IsTimerExpired(unsigned char Num) {
    if (Num >= TIMERS_USED) {
        return ERROR;
    }
    if (UserTimerStates[Num] == ES_TIMEOUT) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function IsTimerActive(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ERROR or TRUE or FALSE
 * @brief  used to determine if a timer is currently active.
 * @author Max Dunne   2013.01.04 */
int8_t IsTimerActive(unsigned char Num) {
    if (Num >= TIMERS_USED) {
        return ERROR;
    }
    if (UserTimerStates[Num] == ES_TIMERACTIVE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function IsTimerStopped(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ERROR or TRUE or FALSE
 * @brief  used to determine if a timer is currently stopped.
 * @author Max Dunne   2013.01.04 */
int8_t IsTimerStopped(unsigned char Num) {
    if (Num >= TIMERS_USED) {
        return ERROR;
    }
    if (UserTimerStates[Num] == ES_TIMERSTOPPED) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function GetUserTimerState(unsigned char Num)
 * @param Num - the number of the timer to check
 * @return ES_EventTyp_t current state of timer
 * @brief  used to get the current timer state.
 * @author Max Dunne   2013.01.04 */
ES_EventTyp_t GetUserTimerState(unsigned char Num) {
    if (Num >= TIMERS_USED) {
        return ERROR;
    }
    return UserTimerStates[Num];
}

/***************************************************************************
 private functions
 ***************************************************************************/

//#define TIMER_SERVICE_TEST
#ifdef TIMER_SERVICE_TEST
#include <stdio.h>

void main(void) {

    ES_Return_t ErrorType;
    uint8_t i;
    BOARD_Init();
    // When doing testing, it is usefull to annouce just which program
    // is running.

    printf("Starting Timer Service Test Harness \r\n");
    printf("using the 2nd Generation Events & Services Framework\n\r");

    // Your hardware initialization function calls go here

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize();

    timerServiceTestingState = init;
    for (i = 0; i <= TIMERS_USED; i++) {
        ES_Timer_InitTimer(i, (i + 1)*1000); //for second scale
    }
    if (ErrorType == Success) {

        ErrorType = ES_Run();

    }
    //if we got to here, there was an error
    switch (ErrorType) {
        case FailedPointer:
            printf("Failed on NULL pointer");
            break;
        case FailedInit:
            printf("Failed Initialization");
            break;
        default:
            printf("Other Failure");
            break;
    }
    for (;;)
        ;

};


#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


/****************************************************************************
 Module
     EaS_LookupTables.c
 Description
     This is the home for a set of constant lookup tables that are used in
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
 01/15/12 09:07 jec      change of heart: converted to global vars for the
                         tables and removed the access functions
 01/12/12 10:25 jec      adding the Ready variable and management functions.
 01/03/12 11:16 jec      started coding
*****************************************************************************/
/*----------------------------- Include Files -----------------------------*/

#include <inttypes.h>

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/

/*
  this table is used to go from a bit number (0-7) to the mask used to clear
  that bit in a byte. Technically, this is not necessary (as you could always
  complement the SetMask) but it will save the complement operation every 
  time it is used to clear a bit. If we clear a bit with it in more than 8
  places, then it is a win on code size and speed.
*/
uint8_t const BitNum2ClrMask[] = {
  0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F
};

/*
  this table is used to go from a bit number (0-7) to the mask used to set
  that bit in a byte.
*/
uint8_t const BitNum2SetMask[] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

/*
  this table is used to go from an unsigned 8bit value to the most significant
  bit that is set in that byte. It is used in the determination of priorities
  from the Ready variable and in determining active timers in 
  the timer interrupt response. Index into the array with (ByteVal-1) to get 
  the correct MS Bit num.
*/
uint8_t const Byte2MSBitNum[255] = {
    0U, 1U, 1U, 2U, 2U, 2U, 2U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 4U,
    4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 5U,
    5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U,
    5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 6U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U
};

/*------------------------------ Module Code ------------------------------*/


/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------ End of File ------------------------------*/


  

#include <BOARD.h>
#include "serial.h"
#include <stdio.h>
#include <peripheral/timer.h>



void ParseCommand(void);
/*---------------------------- Module Defines ---------------------------*/
#define COMMANDSTRINGLENGTH 20
#define TERMINATION_CHARACTER ';'

#define STRINGIFY(x) SSTRINGIFY(x)
#define SSTRINGIFY(x) #x

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;



static char CommandString[COMMANDSTRINGLENGTH] = {0};

/**
 * @Function InitKeyboardInput(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief this initializes the keyboard input system which
 *        Returns TRUE if successful, FALSE otherwise
 * @author Max Dunne , 2013.09.26 */
uint8_t InitKeyboardInput(uint8_t Priority)
{
    ES_Event ThisEvent;

    MyPriority = Priority;
    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostKeyboardInput(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief Used to post events to keyboard input
 *        Returns TRUE if successful, FALSE otherwise
* @author Max Dunne , 2013.09.26 */
uint8_t PostKeyboardInput(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

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
ES_Event RunKeyboardInput(ES_Event ThisEvent)
{
#ifdef USE_KEYBOARD_INPUT
    ES_Event GeneratedEvent;
    uint8_t stringPos = 0;
    uint8_t numbersParsed = 0;
    static uint8_t curCommandLength = 0;
    GeneratedEvent.EventType = ES_NO_EVENT;
    GeneratedEvent.EventParam = 0;
    /********************************************
     in here you write your service code
     *******************************************/
    switch (ThisEvent.EventType) {
    case ES_INIT:
        for (stringPos = 0; stringPos < COMMANDSTRINGLENGTH; stringPos++) {
            CommandString[stringPos] = 0;
        }
        curCommandLength = 0;
        while (!IsTransmitEmpty());
        KeyboardInput_PrintEvents();
        while (!IsTransmitEmpty());
        printf("Keyboard input is active,\
             no other events except timer activations will be processed. \
                You can redisplay the event list by sending a %d event.\r\n \
                Send an event using the form [event#]; \
                or [event#]->[EventParam];", ES_LISTEVENTS);
        break;

    case ES_KEYINPUT:
        if (ThisEvent.EventParam < 127) {
            CommandString[curCommandLength] = (char) ThisEvent.EventParam;
            curCommandLength++;
            if (ThisEvent.EventParam == TERMINATION_CHARACTER) {
                numbersParsed = sscanf(CommandString, "%d -> %X", &GeneratedEvent.EventType, &GeneratedEvent.EventParam);
                if (numbersParsed != 0) {

                    if (GeneratedEvent.EventType < NUMBEROFEVENTS) {
                        switch (GeneratedEvent.EventType) {
                        case ES_LISTEVENTS:
                            KeyboardInput_PrintEvents();
                            break;
                        default:
                            printf("\n\n%s with parameter %X was passed to %s\n", EventNames[GeneratedEvent.EventType], GeneratedEvent.EventParam, STRINGIFY(POSTFUNCTION_FOR_KEYBOARD_INPUT));
                            POSTFUNCTION_FOR_KEYBOARD_INPUT(GeneratedEvent);
                            break;
                        }
                    } else {
                        printf("Event #%d is Invalid, Please try again\n", GeneratedEvent.EventType);
                    }
                }
                for (stringPos = 0; stringPos < COMMANDSTRINGLENGTH; stringPos++) {
                    CommandString[stringPos] = 0;

                }
                curCommandLength = 0;
            }
        }
    default:
        break;
    }
#endif
    return (NO_EVENT);
}



/**
 * @Function KeyboardInput_PrintEvents(void)
 * @param None
 * @return None
 * @brief  Lists out all Events in the EventNames array.
 * @author Max Dunne, 2013.09.26 */
void KeyboardInput_PrintEvents(void)
{
#ifdef USE_KEYBOARD_INPUT
    int curEvent = 0;
    printf("Printing all events available in the system\n");
    for (curEvent = 0; curEvent < NUMBEROFEVENTS; curEvent++) {
        printf("%2d: %-25s", curEvent, EventNames[curEvent]);
        //while(!IsTransmitEmpty());
        if (((curEvent + 3) % 3) == 0) {
            printf("\n");
        }

    }
    printf("\n");
#endif
}

