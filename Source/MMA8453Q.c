////////////////////////////////////////////////////////////////////////////////
// @filename: MMA8453Q.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "MMA8453Q.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static MMA845xQ_t      MMA845xQ = 
{
  .i2cWriteAddr = 0x00,
  .buffSize = 0,
};

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

static bool MMA8453Q_whoAmI( void );

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// sa0 = TRUE, SA0 pin pulled to VCC; sa0 = FALSE, SA0 pin pulled to GND
bool MMA8453Q_initDriver( uint8 buffSize, bool sa0 )
{
  MMA845xQ.i2cWriteAddr = MMA845xQ_DEFAULT_I2C_WRITE_ADDR;
  
  if( sa0 )
     MMA845xQ.i2cWriteAddr |= 0x02;
  
  // Add 1 byte to allocation to accomodate 8-bit address on I2C writes
  MMA845xQ.pBuff = (uint8 *)osal_mem_alloc( buffSize + 1 );
  
  if( MMA845xQ.pBuff != NULL )
  {
    MMA845xQ.buffSize = buffSize;
    return TRUE;
  }
  else
    return TRUE; 
  
} // MMA8453Q_initDriver

bool MMA845Q_initHardware( void )
{
  return MMA8453Q_whoAmI();
  
} // MMA845Q_initHardware

bool MMA8453Q_readReg( mma845xq_regAddr_t addr, uint8 *pData )
{
   // Driver uninitialized, abort
   if( MMA845xQ.i2cWriteAddr == 0x00 )
     return FALSE;
   
   uint8 u8_addr = (uint8)addr;
   if( mujoeI2C_write( MMA845xQ.i2cWriteAddr, 1, &u8_addr, REPEAT_CMD ) )      
   {
      uint8 rxBuff;
      if( mujoeI2C_read( MMA845xQ.i2cWriteAddr, 1, &rxBuff ) )
      {
        *pData = rxBuff;
        return TRUE;
      }
      else
        return FALSE;
   }
   else
     return FALSE;
} // MMA8453Q_readReg

bool MMA8453Q_bulkRead( mma845xq_regAddr_t stAddr, uint8 *pData, uint8 numBytes )
{
   // Driver uninitialized or local buffer size insufficent, abort
   if( MMA845xQ.i2cWriteAddr == 0x00 || numBytes > MMA845xQ.buffSize )
     return FALSE;
   
   uint8 u8_stAddr = (uint8)stAddr;
   if( mujoeI2C_write( MMA845xQ.i2cWriteAddr, 1, &u8_stAddr, REPEAT_CMD ) )     
   {
      if( mujoeI2C_read( MMA845xQ.i2cWriteAddr, numBytes, MMA845xQ.pBuff ) == numBytes )
      {
        VOID memcpy( pData, MMA845xQ.pBuff, numBytes );
        return TRUE;
      }
      else
        return FALSE;
   }
   else
     return FALSE;
   
} // MMA8453Q_bulkRead

bool MMA8453Q_writeReg( mma845xq_regAddr_t addr, uint8 data )
{
  // Driver uninitialized, abort
  if( MMA845xQ.i2cWriteAddr == 0x00 )
    return FALSE;
   
  uint8 txBuff[2] = { (uint8)addr, data };
  return mujoeI2C_write( MMA845xQ.i2cWriteAddr, 2, txBuff, STOP_CMD ) == 2 ? TRUE : FALSE;
  
}// MMA8453Q_writeReg

bool MMA8453Q_bulkWrite( mma845xq_regAddr_t stAddr, uint8 *pData, uint8 numBytes )
{
  // Driver uninitialized or local buffer size insufficent, abort
  if( MMA845xQ.i2cWriteAddr == 0x00 || numBytes > MMA845xQ.buffSize )
    return FALSE;
  
  // Put start address and data bytes into RX/TX buffer
  MMA845xQ.pBuff[0] = stAddr;
  VOID memcpy( MMA845xQ.pBuff + 1, pData, numBytes );
  
  return ( mujoeI2C_write( MMA845xQ.i2cWriteAddr, 
                           numBytes + 1, 
                           MMA845xQ.pBuff, STOP_CMD ) == ( numBytes + 1 ) ) ? TRUE : FALSE;

}// MMA8453Q_bulkWrite

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Returns TRUE if expected Who Am I ID is RX'd. False otherwise.
static bool MMA8453Q_whoAmI( void )
{
  uint8 rxBuff;
  if( MMA8453Q_readReg( MMA_REG_WHO_AM_I, &rxBuff ) )
    return ( rxBuff == MMA845xQ_WHO_AM_I_ID ) ? TRUE : FALSE;
  else
    return FALSE;
} // MMA8453Q_whoAmI

