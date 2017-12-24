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

// Send I2C Command to MSPFuelGauge
// Returns TRUE if command TX was successful
// Returns FALSE otherwise
bool mspfg_sendCommand( uint8 cmd )
{
  cmd |= 0x80;  // Set MSBit to designate byte as I2C command
  
  if( mujoeI2C_write( mspfg.i2cWriteAddr, 1, &cmd, STOP_CMD ) )
    return TRUE;
  else
    return FALSE;
  
} // mspfg_sendCommand

bool mspfg_readReg( uint8 addr, uint8 *pData )
{
  if( mujoeI2C_write( mspfg.i2cWriteAddr, 1, &addr, STOP_CMD ) )
  {
    if( mujoeI2C_read( mspfg.i2cWriteAddr, 1, pData ) )
      return TRUE;
    else
      return FALSE;
  }
  else
    return FALSE;
      
} // mspfg_readReg
    
bool mspfg_writeReg( uint8 addr, uint8 data )
{
  uint8 txBuff[2] = { addr, data };
  if( mujoeI2C_write( mspfg.i2cWriteAddr, 2, txBuff, STOP_CMD ) == 2 )
    return TRUE;
  else
    return FALSE;
  
} // mspfg_writeReg

bool mspfg_bulkRead( uint8 startAddr, uint8 numBytes, uint8 *pData )
{
  if( mujoeI2C_write( mspfg.i2cWriteAddr, 1, &startAddr, STOP_CMD ) )
  {
    if( mujoeI2C_read( mspfg.i2cWriteAddr, numBytes, pData ) == numBytes )
      return TRUE;
    else
      return FALSE;
  }
  else
    return FALSE;
} // mspfg_bulkRead

bool mspfg_getAllData( mspfg_data_t *p_mspfg_data )
{
  uint8 rxBuff[6] = {0};
  if( mspfg_bulkRead( MSPFG_STATUS, 6, rxBuff ) )
  {
    //p_mspfg_data->capAlgo = ( ( (uint16)rxBuff[1] ) << 8 ) + rxBuff[0];
    
    if( rxBuff[0] & MSPFG_STAT_CAPOVRFLW )
      p_mspfg_data->capRawOvrFlw = TRUE;
    
    p_mspfg_data->capAlgo = MAKE_UINT16( rxBuff[2], rxBuff[1]);
    p_mspfg_data->capRaw =  MAKE_UINT16( rxBuff[4], rxBuff[3]);
    p_mspfg_data->fuelLvl = rxBuff[5];
    
    return TRUE;
  }
  else
    return FALSE;
  
} // mspfg_getAllData

bool mspfg_clearIntFlag( void )
{
  uint8 status = 0;
  if( mspfg_readReg( MSPFG_STATUS, &status ) )
  {
    status &= ~MSPFG_STAT_HW_INT_PENDING;
    if( mspfg_writeReg( MSPFG_STATUS, status ) )
      return TRUE;
    else
      return FALSE;
  }
  else
    return FALSE;
} // mspfg_clearIntFlag

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////






