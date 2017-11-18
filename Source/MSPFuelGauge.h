////////////////////////////////////////////////////////////////////////////////
// @filename: MSPFuelGauge.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MSPFUELGAUGE_H
#define MSPFUELGAUGE_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"
#include "mujoeI2C.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define MSPFG_DEFAULT_I2C_ADDR          0xF1    // MSPFuelGauge Default I2C Address

#define MSPFG_CMD_ST_CONT_DATA          0x81    // Start Continous Data colleciton
#define MSPFG_CMD_SP_CONT_DATA          0x82    // Stop Continous Data colleciton
#define MSPFG_CMD_SINGLESHOT_DATA       0x83    // Trigger a single shot measurement
#define MSPFG_CMD_SLEEP                 0x84    // Put MSPFuelGauge to sleep

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  MSPFG_WHO_AM_I                = 0x00, // Who Am I Register
  MSPFG_DEV_INFO,                       // Device Info Register
  MSPFG_CFG,                            // Configuration Register
  MSPFG_CAP_FULL_LSB,                   // Capacitance value when tank full LSByte
  MSPFG_CAP_FULL_MSB,                   // Capacitance value when tank full MSByte
  MSPFG_CAP_ALGO_LSB,                   // Tracking Algo Capacitance Reading LSByte
  MSPFG_CAP_ALGO_MSB,                   // Tracking Algo Capacitance Reading MSByte
  MSPFG_CAP_RAW_LSB,                    // Raw Capacitance Reading LSByte
  MSPFG_CAP_RAW_MSB,                    // Raw Capacitance Reading MSByte
  MSPFG_FUEL_LVL_CRIT_THRESH,           // Critical Fuel Level Threshold (0 to 100%)
  MSPFG_FUEL_LVL,                       // Current Fuel Level (0 to 100%)
  
}mspfg_regAddr_t;


typedef struct mspfg_def
{
  uint8         i2cWriteAddr;
  
}mspfg_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool mspfg_sendCommand( uint8 cmd );

#endif // MSPFUELGAUGE_H