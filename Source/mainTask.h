////////////////////////////////////////////////////////////////////////////////
// @filename: mainTask.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MAINTASK_H
#define MAINTASK_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
  
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"
#include "mujoeGenericProfile.h"
#include "mujoeDataProfile.h"

#include "peripheral.h"
#include "gapbondmgr.h"
 
#include "mujoeTaskMsgr.h"
#include "sensorMgrTask.h"
#include "mujoeGenericProfileMgr.h"
#include "mujoeGenericProfileManager.h"
#include "muJoeBoardSpecificDrivers.h"  
  
/*********************************************************************
 * CONSTANTS
 */

// Simple BLE Peripheral Task Events
#define MAIN_START_DEVICE_EVT                             0x0001
#define MAIN_CMD_WRITE_EVT                                0x0002
#define MAIN_RSP_NOTI_EVT                                 0x0004
#define MAIN_ASYNCBULK_EVT                                0x0008
#define MAIN_ADVBEGIN_EVT                                 0x0010
#define MAIN_ADVEND_EVT                                   0x0020
#define MAIN_GPIOINTMGR_EVT                               0x0040
#define MAIN_BRD_LEDMGR_EVT                               0x0080

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

uint8 mainTask_getTaskId( void );
/*
 * Task Initialization for the BLE Application
 */
extern void mainTask_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 mainTask_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MAINTASK */
