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
#define SENSORMGR_DATA_COLLECTOR_EVT                            0x0002
  
// Max number of onboard sensors
#define SENSORMGR_MAX_NUM_SENSORS                               2

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef struct ppgfgSensorData_def
{
  uint32                barPresCode;
  uint32                barTempCode;
  
}ppgfgSensorData_t;

typedef union boardSensorData_def
{
  ppgfgSensorData_t             ppgfg;
  
} boardSensorData_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

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
