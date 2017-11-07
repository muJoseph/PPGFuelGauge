////////////////////////////////////////////////////////////////////////////////
// @filename: MS560702.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "MS560702.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static MS560702_t MS560702 = 
{
  .i2cWriteAddr = 0,
};


////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

static bool MS560702_readPROMCoeff( MS560702_promCoeffAddr_t addr, uint16 *pCoeffVal );
static bool MS560702_sendCommand( MS560702_cmds_t cmd );


////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

bool MS560702_initDriver( bool csbState )
{
  // Note Bit 1 of the I2C write address is the inverse of the state at the CSB pin
  if( csbState )
    MS560702.i2cWriteAddr = 0xEC;
  else
    MS560702.i2cWriteAddr = 0xEE;
  
  return TRUE;
}

// Initialize the MS560702 IC
// Reads out the cal coefficients stored within the PROM (C1 thru C6) 
// and stores them in the local driver struct
// Returns TRUE if successful, FALSE otherwise.
bool MS560702_initHardware( void )
{
  for( uint8 i = MS5_PROM_COEFF1_ADDR; i <= MS5_PROM_COEFF6_ADDR; i++ )
  {
     if( !MS560702_readPROMCoeff( (MS560702_promCoeffAddr_t)i, MS560702.coeffTbl + i - 1 ) )
       return FALSE;
  }
  
  return TRUE;
  
} // MS560702_initHardware

// Resets the MS560702 IC 
// NOTE: Allow at least 3 ms before resuming comms
bool MS560702_reset( void )
{
  return MS560702_sendCommand( MS5_CMD_RESET );
  
} // MS560702_reset


// Triggers a Pressure Conversion at the oversampling rate specified by "osr" input arg
bool MS560702_trigPressureConv( MS560702_osr_t osr )
{
  return MS560702_sendCommand( (MS560702_cmds_t)(MS5_CMD_ADC_CONV + MS5_CMD_ADC_D1 + osr) );
  
} // MS560702_trigPressureConv

// Triggers a Temperature Conversion at the oversampling rate specified by "osr" input arg
bool MS560702_trigTemperatureConv( MS560702_osr_t osr )
{
  return MS560702_sendCommand( (MS560702_cmds_t)(MS5_CMD_ADC_CONV + MS5_CMD_ADC_D2 + osr) );
  
} // MS560702_trigTemperatureConv

// Reads the 24-bit ADC conversion result
bool MS560702_readAdcConv( uint32 *pAdcCode )
{
  // TX ADC conversion read cmd
  if( MS560702_sendCommand( MS5_CMD_ADC_READ ) )
  {
    uint8 adcConvBytes[3];
    // Read 24-bit conversion and concatenate
    if( mujoeI2C_read( MS560702.i2cWriteAddr , 3, adcConvBytes ) == 3 )
    {
      *pAdcCode = ( ( (uint32)adcConvBytes[0] ) << 16 ) + 
                  ( ( (uint32)adcConvBytes[1] ) << 8 ) +
                   adcConvBytes[2]; 
      return TRUE;
    }
    else
      return FALSE;
  }
  else
    return FALSE;
  
} // MS560702_readAdcConv

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
static bool MS560702_readPROMCoeff( MS560702_promCoeffAddr_t addr, uint16 *pCoeffVal )
{
  // Attempt reading reserved mem or drivers uninitialized, return failure
  if( addr == MS5_PROM_RESERVED_ADDR || MS560702.i2cWriteAddr == 0 )  
    return FALSE;
  
  // Send PROM read cmd concatenated with shifted coeff addr
  if( MS560702_sendCommand( (MS560702_cmds_t)(MS5_CMD_PROM_RD + ( addr << 1 ) ) ) )
  {
    uint8 coeffBytes[2]; 
    // Read coeff value
    if( mujoeI2C_read( MS560702.i2cWriteAddr , 2, coeffBytes ) == 2 )
    {
      *pCoeffVal = ( ( (uint16)coeffBytes[0] ) << 8 ) + coeffBytes[1];
      return TRUE;
    }
    else
      return FALSE;
  }
  else
    return FALSE;
  
} // MS560702_readPROMCoeff

static bool MS560702_sendCommand( MS560702_cmds_t cmd )
{
  if( MS560702.i2cWriteAddr == 0 )      // Driver not initialized, abort
    return FALSE;
  
  if( mujoeI2C_write( MS560702.i2cWriteAddr , 1, (uint8*)cmd, STOP_CMD ) )
    return TRUE;
  else 
    return FALSE;
  
} // MS560702_sendCommand

