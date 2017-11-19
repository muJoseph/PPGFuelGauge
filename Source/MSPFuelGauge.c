////////////////////////////////////////////////////////////////////////////////
// @filename: MSPFuelGauge.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "MSPFuelGauge.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static mspfg_t  mspfg =
{
  .i2cWriteAddr = MSPFG_DEFAULT_I2C_ADDR,
};

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

bool mspfg_ping( void )
{
  return mujoeI2C_i2cPingSlave( mspfg.i2cWriteAddr );
} // mspfg_ping

// Works
bool mspfg_commTest( void )
{
  uint8 txBuff[3] = {  0x55, 0x55, 0x55 };
  if( mujoeI2C_write( mspfg.i2cWriteAddr, 3, txBuff, REPEAT_CMD ) == 3 )
  {
    uint8 rxBuff[2] = {0};
    if( mujoeI2C_read( mspfg.i2cWriteAddr, 2, rxBuff ) == 2 )
    {
      uint8 breakVal = 100;
      return TRUE;
    }
    else
      return FALSE;
  }
  else
    return FALSE;
  
} // mspfg_commTest

bool mspfg_sendCommand( uint8 cmd )
{
  if( mujoeI2C_write( mspfg.i2cWriteAddr, 1, &cmd, STOP_CMD ) )
    return FALSE;
  else
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////






