/*
 * File:   Stepper.c
 * Author: Elkaim
 *
 * Created on January 2, 2012, 9:49 PM
 */

#include <xc.h>
#include <peripheral/timer.h>
#include <SimpleStepper.h>
#include <serial.h>
#include <IO_Ports.h>

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
//#define STEPPER_TEST
//#define DEBUG_VERBOSE

#ifdef DEBUG_VERBOSE
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif


/* Note that you need to set the prescalar and periferal clock appropriate to
 * the processor board that you are using. In order to calculate the minimal
 * prescalar: Prescalar = (2000*F_PB/(1000000*0xFFFF))+1, round down */
#ifndef F_CPU
#define MED_HZ_RATE 77
#define LOW_HZ_RATE 10
#define LOW_REP_NUM 64
#define TEN_KILOHERTZ 10000

#define F_CPU       80000000L
#define F_PB        (F_CPU/2)
#define F_PB_DIV8   (F_PB/8)
#define F_PB_DIV64  (F_PB/64)
#define ONE_KHZ_RATE  (F_PB_DIV8-1)/1000
#define TEN_KHZ_RATE  (F_PB_DIV8-1)/TEN_KILOHERTZ
#define MAX_STEP_RATE 2000
#endif



#define LED_BANK1_3 LATDbits.LATD6
#define LED_BANK1_2 LATDbits.LATD11
#define LED_BANK1_1 LATDbits.LATD3
#define LED_BANK1_0 LATDbits.LATD5


#define ShutDownDrive() IO_PortsClearPortBits(curStepper.port, (curStepper.aEnable | curStepper.bEnable))
#define SetPinsHigh(x) IO_PortsSetPortBits(curStepper.port, x)
#define SetPinsLow(x) IO_PortsClearPortBits(curStepper.port,x)

/*******************************************************************************
 * PRIVATE STRUCTS and TYPEDEFS                                                *
 ******************************************************************************/
const enum stepperState {
    off, paused, stepping
} stepperState;

const enum coilState {
    step_one, step_two, step_three, step_four, step_five, step_six, step_seven, step_eight
} coilState;

typedef struct {
    int8_t port;

    //Uses these with H Bridge
    uint16_t aEnable;
    uint16_t bEnable;
    uint16_t aDir;
    uint16_t bDir;

    //Use these with DRV9911 Stepper Motor driver
    uint16_t boardEnable;
    uint16_t boardDir;
    uint16_t boardStep;

    // Code intializes countdown at 10000/rate
    //each time interrupt happens countdown -1
    //when countdown goes below zero, reset to countdownMax
    int16_t stepCount; //how many steps the stepper has left to take
    int16_t rate; //how many steps per second
    int16_t countdownMax; //what we reset the countdown to
    int16_t countdown; //internal: when to take a step

    unsigned char stepDir; //Forward or backward
    unsigned char deEnergize;
    enum drive driveType; //Full, Wave, Half, or Step

    enum stepperState stepState;
    enum coilState coState;
} Stepper;


/*******************************************************************************
 * PRIVATE VARIABLES                                                           *
 ******************************************************************************/
static Stepper curStepper;
static char initialized = FALSE;
static int timerLoopCount = 0;

/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/
char StartIntTimer(void);
char Init_Stepper_Pins(void);
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/* There are three possibilities for the step rate calculation that keep the
   step rate error to under 0.2% of the desired value, from within 0.5 to 2KHz:
   from 78 to 20Khz, Prescale is 1:8, Rollover = (F_PB/8-1)/PPS
   from 10 to 77Hz, Prescale is 1:64, Rollover = (F_PB/64-1)/PPS
   from 0.5 to 10Hz, Set rollover for a constant 1Khz, and increment to reach
   the desired time, Prescale is 1:8, Rollover = (F_PB/8-1)/1000 and the number
   of times to repeat is (1000-1)/PPS + 1                                     */
char Stepper_Init(char direction, unsigned int steps, unsigned int rate) {
    //if the stepper is not initialized, initialize it.
    //this is the only function to initialize and start the timer
    if (!initialized) {
        if (!(StartIntTimer() && Init_Stepper_Pins())) return ERROR;
        initialized = TRUE;
    }
    //Check if rate is outside of parameters
    if (rate > MAX_STEP_RATE || rate < 0) {
        dbprintf("\n Rate = %d", rate);
        dbprintf("\nRate too High in Stepper Init");
        return ERROR;
    }

    uint16_t allPins;
    if (curStepper.driveType == stepperboard) {
        //Set the step high but dont enable yet
        SetPinsHigh(curStepper.boardStep);
        SetPinsLow(curStepper.boardEnable);

        //set direction high or low depending on direction set
        if (direction) SetPinsHigh(curStepper.boardDir);
        else SetPinsLow(curStepper.boardDir);

        // Initialize hardware (no current flow)
        allPins = (curStepper.boardEnable | curStepper.boardDir | curStepper.boardStep);

        //hbridge
    } else {
        //Set the directions high but dont enable yet
        SetPinsHigh(curStepper.aDir | curStepper.bDir);
        SetPinsLow(curStepper.aEnable | curStepper.bEnable);

        // Initialize hardware (no current flow)
        allPins = (curStepper.aDir | curStepper.bDir | curStepper.aEnable | curStepper.bEnable);
    }

    IO_PortsSetPortOutputs(curStepper.port, allPins);


    //Set up rate to step
    curStepper.rate = rate;
    curStepper.countdownMax = TEN_KILOHERTZ / rate;
    curStepper.countdown = -1;

    //set up steps
    curStepper.stepDir = direction;
    curStepper.stepCount = steps;

    curStepper.stepState = paused;
    return SUCCESS;
}

char Stepper_ChangeStepRate(unsigned short int rate) {
    if (!initialized) {
        dbprintf("Stepper not initialized in ChangeStepRate");
        return ERROR;
    }
    dbprintf("\nChanging step rate");
    //T3CONbits.ON = 0; // halt timer3
    curStepper.rate = rate;
    curStepper.countdownMax = TEN_KILOHERTZ / rate;
    return SUCCESS;
}

char Stepper_SetSteps(char direction, unsigned int steps) {
    if (!initialized) {
        dbprintf("Stepper not initialized in SetSteps");
        return ERROR;
    }
    if (curStepper.stepState == off) return ERROR;
    if ((direction == FORWARD) || (direction == REVERSE)) {
        curStepper.stepDir = direction;
        curStepper.stepCount = steps;
        curStepper.countdown = curStepper.countdownMax;
        return SUCCESS;
    }
    return ERROR;
}

//adds or removes steps from the current step count and deals with the consequences

char Stepper_IncrementSteps(char direction, unsigned int steps) {
    if (!initialized) {
        dbprintf("Stepper not initialized in ChangeStepRate");
        return ERROR;
    }

    unsigned int tempSteps;

    if ((direction == FORWARD) || (direction == REVERSE)) {
        //if directions match it adds steps
        if ((direction == FORWARD) && (curStepper.stepDir == FORWARD)) {
            curStepper.stepCount += steps;
        }
        if ((direction == REVERSE) && (curStepper.stepDir == REVERSE)) {
            curStepper.stepCount += steps;
        }
        //if directions dont match it removes steps
        if ((direction == FORWARD) && (curStepper.stepDir == REVERSE)) {
            if (steps > curStepper.stepCount) {
                tempSteps = steps - curStepper.stepCount;
                curStepper.stepDir = FORWARD;
                curStepper.stepCount = tempSteps;
            } else {
                curStepper.stepCount -= steps;
            }
        }
        if ((direction == REVERSE) && (curStepper.stepDir == FORWARD)) {
            if (steps > curStepper.stepCount) {
                tempSteps = steps - curStepper.stepCount;
                curStepper.stepDir = REVERSE;
                curStepper.stepCount = tempSteps;
            } else {
                curStepper.stepCount -= steps;
            }
        }
    }

    //if at the end, there are still steps, it ensures they get executed
    if (curStepper.stepCount > 0) {
        curStepper.stepState = stepping;
        //else it pauses the motor
    } else {
        curStepper.stepState = paused;
    }

    return SUCCESS;
}

char Stepper_GetDirection(void) {
    if (!initialized) {
        dbprintf("Stepper not initialized in GetDirection");
        return ERROR;
    }
    return curStepper.stepDir;
}

int Stepper_GetRemainingCount() {
    if (!initialized) {
        dbprintf("Stepper not initialized in GetRemainingCount");
        return ERROR;
    }
    return curStepper.stepCount;
}

char Stepper_Pause() {
    if (!initialized) {
        dbprintf("Stepper not initialized in Pause");
        return ERROR;
    }
    dbprintf("\nHalting Stepper drive");
    curStepper.stepState = paused;
    if (curStepper.driveType==stepperboard) {
        SetPinsLow(curStepper.boardEnable);
    } else { //hbridge
        ShutDownDrive();
    }
    return SUCCESS;
}

char Stepper_Start() {
    if (!initialized) {
        dbprintf("Stepper not initialized in Start");
        return ERROR;
    }
    if (curStepper.stepState != paused) {
        dbprintf("Error: Stepper was not in a paused state");
        return ERROR;
    }

    dbprintf("\nResuming Stepper drive");
    if (curStepper.stepCount > 0) {
        curStepper.stepState = stepping;
    } else {
        dbprintf("\n Stepper has 0 steps left");
        curStepper.stepState = paused;
    }
    return SUCCESS;
}

char Stepper_End() {
    // turn off timer and interrupt
    CloseTimer3();
    //Release the pins
    dbprintf("\nEntering Stepper_End().");
    if (curStepper.stepState == off) {
        dbprintf("\n Stepper already off");
        return ERROR;
    }
    curStepper.stepState = off;
    uint16_t allPins;
    if (curStepper.driveType == stepperboard) {
        allPins = (curStepper.boardStep | curStepper.boardDir | curStepper.boardEnable);

    } else { //hbridge
        allPins = (curStepper.aDir | curStepper.bDir | curStepper.aEnable | curStepper.bEnable);
    }

    SetPinsLow(allPins);
    // reset module variables
    curStepper.stepCount = 0;
    curStepper.countdown = -1;
    curStepper.coState = step_one;



    return SUCCESS;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
char Init_Stepper_Pins(void) {
    //Pull in the #defines from the h file
    curStepper.port = STEPPER_PORT;
    curStepper.driveType = TYPE_OF_DRIVE;
    uint16_t pins[] = STEPPER_PINS;
    if (curStepper.driveType == stepperboard) {
        curStepper.boardEnable = pins[0];
        curStepper.boardDir = pins[1];
        curStepper.boardStep = pins[2];

    } else {
        //hbridge
        curStepper.aEnable = pins[0];
        curStepper.bEnable = pins[1];
        curStepper.aDir = pins[2];
        curStepper.bDir = pins[3];
    }

    //internal
    curStepper.stepCount = 0;
    curStepper.stepDir = FORWARD;
    curStepper.coState = step_one;
    curStepper.countdown = -1;
    curStepper.stepState = paused;

    curStepper.deEnergize = TRUE;
    curStepper.rate = -1;
}

char StartIntTimer(void) {
    int overflowPeriod = TEN_KHZ_RATE;

    timerLoopCount = 0;
    dbprintf("\nOverflow Period: %u", overflowPeriod);
    // Setup timer and interrupt
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_8, overflowPeriod);
    //    OpenTimer5(T5_ON | T5_IDLE_STOP | T5_GATE_OFF | T5_PS_1_8 | T5_SOURCE_INT,overflowPeriod);
    //    OpenTimer5(T5_ON | T5_PS_1_8 | T5_SOURCE_INT, overflowPeriod);
    ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_3);
    mT3IntEnable(1);
    return SUCCESS;
}

char executeStateMachineFull() {
    if (curStepper.stepState == paused) {
        dbprintf("stepState is paused but should not be");
        return ERROR;
    }
    LED_BANK1_3 ^= 1;
    switch (curStepper.coState) {
        case step_one:
            dbprintf("\n Stepper: step one");
            // coil drive both forward
            SetPinsHigh(curStepper.aDir | curStepper.bDir);
            if (curStepper.stepDir == FORWARD) curStepper.coState = step_two;
            else curStepper.coState = step_four;
            break;

        case step_two:
            dbprintf("\n Stepper: step two");
            // coil drive A forward, B reverse
            SetPinsHigh(curStepper.aDir);
            SetPinsLow(curStepper.bDir);
            if (curStepper.stepDir == FORWARD) curStepper.coState = step_three;
            else curStepper.coState = step_one;
            break;

        case step_three:
            dbprintf("\n Stepper: step three");
            // coil drive both reverse
            SetPinsLow(curStepper.aDir | curStepper.bDir);
            if (curStepper.stepDir == FORWARD) curStepper.coState = step_four;
            else curStepper.coState = step_two;
            break;

        case step_four:
            dbprintf("\n Stepper: step four");
            // coild drive A reverse, B forward
            SetPinsLow(curStepper.aDir);
            SetPinsHigh(curStepper.bDir);
            if (curStepper.stepDir == FORWARD) curStepper.coState = step_one;
            else curStepper.coState = step_three;
            break;
    }
    return SUCCESS;
}

char executeStateMachineWave() {
    //Insert your code for Wave Drive State Machine here
    return SUCCESS;
}

char executeStateMachineStep() {
    //Insert your code for the stepper board state machine here
    return SUCCESS;
}

char executeStateMachineHalf() {
    //Insert your code for Half Drive State Machine Here
    return SUCCESS;
}

/****************************************************************************
 Function: Timer3IntHandler

 Parameters: None.

 Returns: None.

 Description
    Implements the Stepper motor FULL STEP drive state machine.

 Notes
    

 Author: Gabriel Hugh Elkaim, 2011.12.15 16:42
 ****************************************************************************/
void __ISR(_TIMER_3_VECTOR, ipl4) Timer3IntHandler(void) {
    int index;
    LED_BANK1_0 ^= 1;
    if (curStepper.stepState == stepping) {
        curStepper.countdown--;
        if (curStepper.countdown < 1) {
            LED_BANK1_1 ^= 1;
            // execute Stepper Drive state machine here
            switch (curStepper.driveType) {
                case stepperboard:
                    executeStateMachineStep(index);
                    break;
                case wave:
                    executeStateMachineWave(index);
                    break;
                case half:
                    executeStateMachineHalf(index);
                    break;
                default:
                    executeStateMachineFull(index);
                    break;
            }
            //reset the countdowntimer for that stepper
            curStepper.countdown = curStepper.countdownMax;
            //remove one step
            curStepper.stepCount--;
            if (curStepper.stepCount <= 0) Stepper_Pause();
        }
    }
    mT3ClearIntFlag();
}



/*******************************************************************************
 * TEST HARNESS                                                                *
 ******************************************************************************/
#ifdef STEPPER_TEST

#define NOPCOUNT 150000
//only use DELAYS in test harnesses not frameworks!!
#define DELAY() for(delayct=0; delayct< NOPCOUNT; delayct++) __asm("nop")
long unsigned int delayct;

int main(void) {
    long unsigned int i, j;
    BOARD_Init();
    int rate;
    int steps;

    rate = 400;
    steps = 1000;
    if (Stepper_Init(FORWARD, steps, rate) == ERROR) {
        printf("\nError initializing stepper 1");
        return (0);
    }
    printf("\nStepper initialized!");

    //Start Stepper
    if (Stepper_Start() == ERROR) {
        printf("\nError adding steps to stepper");
    }
    printf("\nStepping forward %d steps at %d steps a second", steps, rate);
    //Test pause:
    while (Stepper_GetRemainingCount() > 30) {
        printf("\nSteps left: %d", Stepper_GetRemainingCount());
        DELAY(); //only use these in test harnesses not frameworks!!
    }
    printf("\nPausing stepper ...");
    Stepper_Pause();
    DELAY();
    DELAY();
    DELAY();
    printf("\nunPausing (ie starting) stepper ...");
    printf("Adding More steps to stepper");
    if (Stepper_SetSteps(REVERSE, 800) == ERROR) {
        printf("\nError adding steps");
        return (0);
    }

    Stepper_Start();
    while (Stepper_GetRemainingCount() > 0) {
    }

    printf("\nAdding 800 more steps");
    if (Stepper_SetSteps(FORWARD, 800) == ERROR) {
        printf("\nError adding steps");
        return (0);
    }
    Stepper_Start(); //Need to restart it because it paused after running out of steps.
    while (Stepper_GetRemainingCount() > 5) {
        printf("\nSteps left: %d", Stepper_GetRemainingCount());
        DELAY();
    }
    printf("\nEnding module, so stepper will stop");
    Stepper_End();

    return (0);
}

// ORIGINAL TEST CODE
//    //Initialize Stepper
//    rate= 2;
//    steps=50;
//    printf("\nInitializing stepper...");
//    if(Stepper_Init(FORWARD, steps, rate)== ERROR){
//        printf("\nError initializing stepper 0");
//    }
//
//    //Start Stepper
//    printf("\nStepping forward %d steps at %d steps a second", steps, rate);
//    if(Stepper_Start() == ERROR){
//        printf("\nError adding steps to stepper 0");
//    }
//    printf("\nSteps left: %u", Stepper_GetRemainingCount());
//    while (Stepper_GetRemainingCount()>25) {
//        printf("\nSteps left: %u", Stepper_GetRemainingCount());
//        DELAY();
//    }
//    rate= 4;
//    if (Stepper_ChangeStepRate(4)==SUCCESS) {
//        printf("\nStepping Twice as Quick");
//    }
//    while (Stepper_GetRemainingCount()>5) {
//        printf("\nSteps left: %u", Stepper_GetRemainingCount());
//        DELAY();
//    }
//    Stepper_Pause();
//    printf("\nWe Paused! Steps left: %u", Stepper_GetRemainingCount());
//    Stepper_End();
//
//    return 0;
//}

#endif

