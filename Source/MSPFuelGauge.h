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
#include "mujoeToolBox.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define MSPFG_DEFAULT_I2C_ADDR          (0x48<<1)   // MSPFuelGauge Default I2C Address 

// Configuration Register Bit Map
#define MSPFG_CFG_EN_HW_INT                     0x01        // If set, enables generation of HW interrupt

// Status Register Bit Map
#define MSPFG_STAT_CAPOVRFLW                    0x01        // If set, raw capacitance count has overflowed during accumulation window. Updated with each capacitance measurement
#define MSPFG_STAT_HW_INT_PENDING               0x08        // If set, HW interrupt is asserted. Must be cleared by host to de-assert HW interrupt  line.

// MSPFuelGauge I2C Commands
#define MSPFG_CMD_ST_CONT_DATA                  0x81        // Start Continuous Data collection
#define MSPFG_CMD_SP_CONT_DATA                  0x82        // Stop Continuous Data collection
#define MSPFG_CMD_SINGLESHOT_DATA               0x83        // Trigger a single shot measurement
#define MSPFG_CMD_SLEEP                         0x84        // Put MSPFuelGauge to sleep

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

// MSPFuelGauge Register Addresses
typedef enum
{
  MSPFG_WHO_AM_I                = 0x00, // Who Am I Register (R)
  MSPFG_DEV_INFO,                       // Device Info Register (R)
  MSPFG_CFG,                            // Configuration Register (R/W)
  MSPFG_STATUS,                         // Status Register (R/W)
  MSPFG_CAP_ALGO_LSB,                   // Tracking Algo Capacitance Reading LSByte (R)
  MSPFG_CAP_ALGO_MSB,                   // Tracking Algo Capacitance Reading MSByte (R)
  MSPFG_CAP_RAW_LSB,                    // Raw Capacitance Reading LSByte (R)
  MSPFG_CAP_RAW_MSB,                    // Raw Capacitance Reading MSByte (R)
  MSPFG_FUEL_LVL,                       // Current Fuel Level (0 to 100%) (R)
  MSPFG_CAP_FULL_LSB,                   // Capacitance value when tank full LSByte (R/W)
  MSPFG_CAP_FULL_MSB,                   // Capacitance value when tank full MSByte (R/W)
  MSPFG_FUEL_LVL_CRIT_THRESH,           // Critical Fuel Level Threshold (0 to 100%) (R/W)
  MSPFG_NUM_REGISTERS,                  // Number of MSPFuelGauge Registers

}mspfg_regAddr_t;

typedef struct mspfg_data_def
{
  uint16        capAlgo;
  uint16        capRaw;
  uint8         fuelLvl;        
  bool          capRawOvrFlw;
  
} mspfg_data_t;

typedef struct mspfgCfg_def
{
  bool         enHwInterrupt;
  
}mspfgCfg_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool mspfg_initHardware( mspfgCfg_t cfg );
bool mspfg_sendCommand( uint8 cmd );
bool mspfg_readReg( uint8 addr, uint8 *pData );
bool mspfg_writeReg( uint8 addr, uint8 data );
bool mspfg_bulkRead( uint8 startAddr, uint8 numBytes, uint8 *pData );
bool mspfg_getAllData( mspfg_data_t *p_mspfg_data );
bool mspfg_clearIntFlag( void );

#endif // MSPFUELGAUGE_H