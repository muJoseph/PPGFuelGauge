////////////////////////////////////////////////////////////////////////////////
// @filename: sensorMgrTask.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef SENSORMGRTASK_H
#define SENSORMGRTASK_H

#ifdef __cplusplus
extern "C"
{
#endif

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////
   
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "gatt.h"
#include "hci.h"

#include "mainTask.h"

// Device Drivers
#include "MS560702.h"
  
////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

// Sensor Manager Task Events
#define SENSORMGR_INIT_SENSORS_EVT                              0x0001

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * FUNCTIONS
 */

uint8 sensorMgrTask_getTaskId( void );
/*
 * Task Initialization for the BLE Application
 */
extern void sensorMgrTask_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 sensorMgrTask_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSORMGRTASK_H */
