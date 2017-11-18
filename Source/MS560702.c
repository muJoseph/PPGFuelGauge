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
static uint8 crc4( uint16 *prom );

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
// and stores them in the local driver struct along with assoc CRC.
// Returns TRUE if successful, FALSE otherwise.
bool MS560702_initHardware( void )
{
  // Read the eight 16-bit words out of PROM
  for( uint8 i = MS5_PROM_RESERVED_ADDR; i <= MS5_PROM_CRC_ADDR; i++ )
  {
     // Abort if any words cannot be read
     if( !MS560702_readPROMCoeff( (MS560702_promCoeffAddr_t)i, MS560702.prom + i ) )
       return FALSE;
  }
  
  // Check the 4-bit CRC stored in PROM against the computed CRC
  if( crc4( MS560702.prom ) == ( MS560702.prom[7] & 0x000F ) )
    return TRUE;
  else
    return FALSE;
  
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

// @TODO: 
// - Add fncs to read PROM CRC
// - Add fncs to convert ADC codes to mBar and degrees C (prob better to do this on the central device due to floating pt math)
// TODO@

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static bool MS560702_readPROMCoeff( MS560702_promCoeffAddr_t addr, uint16 *pCoeffVal )
{
  // Drivers uninitialized, abort
  if( MS560702.i2cWriteAddr == 0 )  
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
  
  uint8 u8_cmd = (uint8)cmd;
  if( mujoeI2C_write( MS560702.i2cWriteAddr , 1, &u8_cmd, STOP_CMD ) )
    return TRUE;
  else 
    return FALSE;
  
} // MS560702_sendCommand

// Computes the 4-bit crc on eight 16-bit words of PROM data
static uint8 crc4( uint16 *prom )
{
  uint16 n_rem = 0x00;               // crc remainder
  uint16 crc_read = prom[7];         // original value of the crc
  prom[7] &= 0xFF00;                 // CRC byte is replaced by 0
  
  // operation is performed on bytes
  for ( uint8 cnt = 0; cnt < 16; cnt++) 
  {     
    // choose LSB or MSB
    if ( cnt % 2 == 1 )
      n_rem ^= (uint16) ((prom[cnt>>1]) & 0x00FF);  // LSB
    else
      n_rem ^= (uint16) (prom[cnt>>1]>>8);          // MSB
    
    for ( uint8 n_bit = 8; n_bit > 0; n_bit-- )
    {
        if (n_rem & (0x8000))
          n_rem = (n_rem << 1) ^ 0x3000;
        else
          n_rem = (n_rem << 1);
    }
  }
  
  // final 4-bit reminder is CRC code
  n_rem = (0x000F & (n_rem >> 12));
  // restore the crc_read to its original place
  prom[7] = crc_read; 
  
  return (n_rem ^ 0x0);
}

