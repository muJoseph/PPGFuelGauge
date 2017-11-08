////////////////////////////////////////////////////////////////////////////////
// @filename: CAT24C512.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "CAT24C512.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static CAT24C512_t CAT24C512 = 
{
  .i2cWriteAddr = 0,
  .pBuff = NULL,
  .buffSize = 0,
};

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Input Args:
// a2 thru a0: Corresponds to the state of the physical I2C address select pins of
//             the chip. If ax = FALSE then pin is GND if TRUE then pin is pulled to
//             VCC
// buffSize: Size of the TX/RX buffer to be created for CAT24C512 transactions
bool CAT24C512_initDriver( uint8 buffSize, bool a2, bool a1, bool a0 )
{
  CAT24C512.i2cWriteAddr = 0xA0;
  
  if( a2 )
    CAT24C512.i2cWriteAddr |= 0x08;
  if( a1 )
    CAT24C512.i2cWriteAddr |= 0x04;
  if( a0 )
    CAT24C512.i2cWriteAddr |= 0x02;
  
  // Clamp buffer size to max Page write transaciton size
  if( buffSize > 128 ){ buffSize = 128; }
  // Add two bytes to allocation to accomodate 16-bit address
  CAT24C512.pBuff = (uint8 *)osal_mem_alloc( buffSize + 2 );    
  
  if( CAT24C512.pBuff != NULL )
  {
    CAT24C512.buffSize = buffSize;
    return TRUE;
  }
  else
    return FALSE;
  
} // CAT24C512_initDriver

// Write to single byte
bool CAT24C512_writeByte( uint16 byteAddr, uint8 byteData )
{
  if( CAT24C512.i2cWriteAddr == 0x00 )  // Drivers uninitialized, abort
    return FALSE;
  
  uint8 txBuff[3] = { (uint8)(byteAddr >> 8), (uint8)byteAddr , byteData };
  if( mujoeI2C_write( CAT24C512.i2cWriteAddr, 3, txBuff, STOP_CMD ) == 3 )
    return TRUE;
  else
    return FALSE;
  
} // CAT24C512_writeByte


bool CAT24C512_writePage( uint16 stPageAddr, uint8 stByteAddr, uint8 *pDataBytes, uint8 numBytes )
{
  // Check for unsupported params, abort if necessary
  if( ( numBytes > 128 ) || ( stPageAddr > 511 ) || ( stByteAddr > 127 ) )
    return FALSE;
  
  uint8 addrMsbyte = (uint8)(stPageAddr >> 1);
  uint8 addrLsbyte = stByteAddr;
  if( stPageAddr & 0x0001 )     // Transfer LSb of stPageAddr to MSb of addrLsbyte
    addrLsbyte |= 0x80;
  
  // Load 16 bit address into local TX/RX buffer
  CAT24C512.pBuff[0] = addrMsbyte;
  CAT24C512.pBuff[1] = addrLsbyte;
  
  // Copy data to be written into local TX/RX buffer
  if( CAT24C512.pBuff != NULL )
    VOID memcpy( CAT24C512.pBuff + 2, pDataBytes, numBytes );
  
  // TX 16-bit address along with data bytes
  if( mujoeI2C_write( CAT24C512.i2cWriteAddr, ( numBytes + 2 ), CAT24C512.pBuff, STOP_CMD ) == ( numBytes + 2 ) )
    return TRUE;
  else
    return FALSE;
  
} // CAT24C512_writePage

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////





