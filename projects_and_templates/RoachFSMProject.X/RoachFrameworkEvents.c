#include <BOARD.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "RoachFrameworkEvents.h"
#ifdef DEV
#include "ES_ServiceHeaders.h"
#include "ES_Events.h"
#endif
#include "roach.h"


#define DARK_THRESHOLD 234
#define LIGHT_THRESHOLD 88

//This is simply a placeholder. Figure out how checkBumps is called and works and implement from there.
uint8_t CheckLightLevel(void)
{
    //put some cool code in here
}

uint8_t CheckBumps(void)
{
    //or in here.  Check the ES Framework doc to see how to post events.
}

