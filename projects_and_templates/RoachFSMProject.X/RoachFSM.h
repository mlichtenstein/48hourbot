/****************************************************************************
 
  Header file for template Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef RoachFSM_H
#define RoachFSM_H

// Event Definitions
#include "ES_Configure.h"
#include <inttypes.h>
// typedefs for the states
// State definitions for use with the query function


//list your states as a macro'd list of other functionlike macros
//Give them unique names!
#define ROACH_STATES(STATE) \
        STATE(Init) \
        STATE(Moving)       \
        STATE(Stopped)  \

// This turns the states into typedefs
#define ENUM_FORM(STATE) STATE, //Enums are reprinted verbatim and comma'd
typedef enum {
    ROACH_STATES(ENUM_FORM)
} RoachState_t;



// Public Function Prototypes


uint8_t InitRoachFSM(uint8_t Priority);
uint8_t PostRoachFSM(ES_Event ThisEvent);
ES_Event RunRoachFSM(ES_Event ThisEvent);
RoachState_t QueryRoachSM(void);


#endif /* FSMTemplate_H */

