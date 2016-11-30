/*
 * File: TemplateSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel Elkaim and Soja-Marie Morgens
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is another template file for the SubHSM's that is slightly differet, and
 * should be used for all of the subordinate state machines (flat or heirarchical)
 *
 * This is provided as an example and a good place to start.
 *
 * History
 * When           Who     What/Why
 * -------------- ---     --------
 * 09/13/13 15:17 ghe      added tattletail functionality and recursive calls
 * 01/15/12 11:12 jec      revisions for Gen2 framework
 * 11/07/11 11:26 jec      made the queue static
 * 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 * 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
*/


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "TemplateHSM.h"
#include "TemplateSubHSM.h" //#include all sub state machines called
/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
//Include any defines you need to do
/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


#define STRING_FORM(STATE) #STATE, //Strings are stringified and comma'd
static const char *StateNames[] = {
    LIST_OF_HSM_STATES(STRING_FORM)
};

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
   Example: char RunAway(uint_8 seconds);*/
/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TemplateState_t CurrentState = InitHState;   // <- change enum name to match ENUM
static uint8_t MyPriority;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitTemplateHSM(uint8_t Priority)
{
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitHState;
    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/**
 * @Function PostTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostTemplateHSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}


/**
 * @Function QueryTemplateHSM(void)
 * @param none
 * @return Current state of the state machine
 * @brief This function is a wrapper to return the current state of the state
 *        machine. Return will match the ENUM above. Remember to rename to
 *        something appropriate, and also to rename the TemplateState_t to your
 *        correct variable name.
 * @author J. Edward Carryer, 2011.10.23 19:25 */
TemplateState_t QueryTemplateHSM(void)
{
    return (CurrentState);
}


/**
 * @Function RunTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunTemplateHSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateState_t nextState;      // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
    case InitHState: // If current state is initial Pseudo State
        if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
        {
            // this is where you would put any actions associated with the
            // transition from the initial pseudo-state into the actual
            // initial state
            // Initialize all sub-state machines
                InitTemplateSubHSM();
            // now put the machine into the actual initial state
            CurrentState = FirstState;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;;
        }
        break;

    case FirstState: // in the first state, replace this with correct names
        // run sub-state machine for this state
        //NOTE: the SubState Machine runs and responds to events before anything in the this
        //state machine does
        ThisEvent = RunTemplateSubHSM(ThisEvent); 
        if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
            switch (ThisEvent.EventType) {
            case ES_ENTRY:
                // this is where you would put any actions associated with the
                // entry to this state
                break;

            case ES_EXIT:
                // this is where you would put any actions associated with the
                // exit from this state
                break;

            case ES_KEYINPUT:
                // this is an example where the state does NOT transition
                // do things you need to do in this state
                // event consumed
                ThisEvent.EventType = ES_NO_EVENT;
                break;

            case ES_TIMEOUT:
                // create the case statement for all other events that you are
                // interested in responding to.
                nextState = OtherState;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                break;

            default: // all unhandled events pass the event back up to the next level
                break;
            }
        }
        break;

    case OtherState: // If current state is state OtherState
        ThisEvent = RunTemplateSubHSM(ThisEvent); // run sub-state machine for this state
        //Note: this is currently running the same substate machine for both states
        //usually you want to use two Different ones
        if (ThisEvent.EventType != ES_NO_EVENT) { // An event is active
            switch (ThisEvent.EventType) {
            case ES_ENTRY:
                // this is where you would put any actions associated with the
                // entry to this state
                break;

            case ES_EXIT:
                // this is where you would put any actions associated with the
                // exit from this state
                break;

            case ES_KEYINPUT:
                // this is an example where the state does NOT transition
                // do things you need to do in this state
                // event consumed
                ThisEvent.EventType = ES_NO_EVENT;
                break;

            case ES_TIMEOUT:
                // create the case statement for all other events that you are
                // interested in responding to. This does a transition
                nextState = YetAnotherState;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                break;

            default: // all unhandled events pass the event back up to the next level
                break;
            }
        }
        break;

    case YetAnotherState: // example of a state without a sub-statemachine
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            // this is where you would put any actions associated with the
            // entry to this state
            break;

        case ES_EXIT:
            // this is where you would put any actions associated with the
            // exit from this state
            break;

        case ES_KEYINPUT:
            // this is an example where the state does NOT transition
            // do things you need to do in this state
            // event consumed
            ThisEvent.EventType = ES_NO_EVENT;
            break;

        case ES_TIMEOUT:
            // create the case statement for all other events that you are
            // interested in responding to. This one does a transition
            nextState = FirstState;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;

        default: // all unhandled events pass the event back up to the next level
            break;
        }
        break;

    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTemplateHSM(EXIT_EVENT);   // <- rename to your own Run function
        CurrentState = nextState;
        RunTemplateHSM(ENTRY_EVENT);  // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
/*Here's where you put the actual content of your functions.
Example:
 * char RunAway(uint_8 seconds) {
 * Lots of code here
 * } */

/*******************************************************************************
 * TEST HARNESS                                                                *
 ******************************************************************************/
/* Define TEMPLATEFSM_TEST to run this file as your main file (without the rest
 * of the framework)-useful for debugging */
#ifdef TEMPLATEHSM_TEST // <-- change this name and define it in your MPLAB-X
                        //     project to run the test harness
#include <stdio.h>

void main(void)
{
    ES_Return_t ErrorType;
    BOARD_Init();
    // When doing testing, it is useful to annouce just which program
    // is running.

    printf("Starting the Hierarchical State Machine Test Harness \r\n");
    printf("using the 2nd Generation Events & Services Framework\n\r");

    // Your hardware initialization function calls go here

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize();

    if (ErrorType == Success) {
        ErrorType = ES_Run();
    }

    //
    //if we got to here, there was an error
    //

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

    while (1) {
        ;
    }
}

#endif // TEMPLATEHSM_TEST
