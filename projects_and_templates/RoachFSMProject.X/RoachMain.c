#include <xc.h>
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "serial.h"
#include "roach.h"
#include "BOARD.h"



void main(void)
{
    ES_Return_t ErrorType;

    BOARD_Init();
    // When doing testing, it is usefull to annouce just which program
    // is running.
    
    printf("Starting Roach State Machine \r\n");
    printf("using the 2nd Generation Events & Services Framework\n\r");


    // Your hardware initialization function calls go here
    Roach_Init();
    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize();
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

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
