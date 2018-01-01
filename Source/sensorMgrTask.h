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

#include "mujoeTaskMsgr.h"
#include "mainTask.h"

// Device Drivers
#include "MS560702.h"
#include "CAT24C512.h"
#include "MMA8453Q.h"
#include "MSPFuelGauge.h"
  
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
  uint32                barPresCode;    // 4 Bytes
  uint32                barTempCode;    // 4 Bytes
  mspfg_data_t          mspfg;          // 6 Bytes
  
}ppgfgSensorData_t;

typedef union boardSensorData_def
{
  ppgfgSensorData_t             ppgfg;
  
} boardSensorData_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

uint8 sensorMgrTask_getTaskId( void );

boardSensorData_t sensorMgrTask_getSensorData( void );

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
