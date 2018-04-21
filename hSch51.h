/*------------------------------------------------------------------*-

   hSCH51.h (v1.00)

-*------------------------------------------------------------------*/

#ifndef _hSCH51_H
#define _hSCH51_H

#include "Main.h"

// ------ Public data type declarations ----------------------------

typedef data struct {
    // Pointer to the task (must be a 'void (void)' function)
    void (code * pTask)(void);

    // Delay (ticks) until the function will (next) be run
    // - see SCH_Add_Task() for further details
    tWord Delay;

    // Interval (ticks) between subsequent runs.
    // - see SCH_Add_Task() for further details
    tWord Period;

    // Incremented (by scheduler) when task is due to execute
    tByte RunMe;

    // Set to 1 if task is co-operative	合作
    // Set to 0 if task is pre-emptive		抢占
    tByte Co_op;
} sTaskH;

// ------ Public function prototypes -------------------------------

// Core scheduler functions

void  hSCH_Dispatch_Tasks(void);
tByte hSCH_Add_Task(void (code *)(void), tWord, tWord, bit);
bit   hSCH_Delete_Task(tByte);
void  hSCH_Report_Status(void);

// ------ Public constants -----------------------------------------

#define hSCH_MAX_TASKS   (2)

#endif

/*------------------------------------------------------------------*-
  ---- END OF FILE -------------------------------------------------
-*------------------------------------------------------------------*/

