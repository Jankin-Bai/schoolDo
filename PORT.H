/*------------------------------------------------------------------*-

   Port.H (v1.00)
	端口头文件
  
-*------------------------------------------------------------------*/

// ------ Sch51.C ----------------------------------------

// Comment this line out if error reporting is NOT required
#define SCH_REPORT_ERRORS

#ifdef SCH_REPORT_ERRORS
// The port on which error codes will be displayed
// ONLY USED IF ERRORS ARE REPORTED
#define Error_port P1

#endif

// ------ LED_hyb.C ------------------------------------------------

// Two tasks share this port (P2)
// We will treat it as a shared resource, and 'lock' it
sbit LED_short_pin = P2^2;

#define LED_long_port P2

// --------------ReadKeyBoard.c-------------------------------------

#define No_key 255
#define K1_1 1
#define K1_2 2
#define K1_3 3
#define K2_1 4
#define K2_2 5
#define K2_3 6
#define K3_1 7
#define K3_2 8
#define K3_3 9
#define K4_1 10
#define K4_2 0
#define K4_3 11
#define Key_mask B00000111
 
/*------------------------------------------------------------------*-
  ---- END OF FILE -------------------------------------------------
-*------------------------------------------------------------------*/



