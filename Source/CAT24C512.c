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

static void CAT24C512_buildAddrPayload(uint16 pageAddr, uint8 byteAddr, uint8 *pPayload );

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

bool CAT24C512_initHardware( void )
{
  return mujoeI2C_i2cPingSlave( CAT24C512.i2cWriteAddr );
  
} // CAT24C512_initHardware

// Write to single byte
bool CAT24C512_writeByte( uint16 pageAddr, uint8 byteAddr, uint8 byteData )
{
  // Drivers uninitialized, abort
  if( CAT24C512.i2cWriteAddr == 0x00 )  
    return FALSE;
  
  // Build TX payload
  uint8 txBuff[3] = {0};
  CAT24C512_buildAddrPayload(pageAddr, byteAddr, txBuff );
  txBuff[2] = byteData;
 
  // Send TX payload
  if( mujoeI2C_write( CAT24C512.i2cWriteAddr, 3, txBuff, STOP_CMD ) == 3 )
    return TRUE;
  else
    return FALSE;
  
} // CAT24C512_writeByte

bool CAT24C512_writePage( uint16 stPageAddr, uint8 stByteAddr, uint8 *pDataBytes, uint8 numBytes )
{
  // Drivers uninitialized, abort
  if( CAT24C512.i2cWriteAddr == 0x00 ) 
    return FALSE;
  
  // Check for unsupported params, abort if necessary
  if( ( numBytes > 128 ) || ( stPageAddr > CAT24C512_LAST_PAGE_ADDR ) || ( stByteAddr > CAT24C512_LAST_BYTE_ADDR ) )
    return FALSE;
  
  /*
  uint8 addrMsbyte = (uint8)(stPageAddr >> 1);
  uint8 addrLsbyte = stByteAddr;
  if( stPageAddr & 0x0001 )     // Transfer LSb of stPageAddr to MSb of addrLsbyte
    addrLsbyte |= 0x80;
  
  // Load 16 bit address into local TX/RX buffer
  CAT24C512.pBuff[0] = addrMsbyte;
  CAT24C512.pBuff[1] = addrLsbyte;
  */
  
  // Load 16 bit address into local TX/RX buffer
  CAT24C512_buildAddrPayload( stPageAddr, stByteAddr, CAT24C512.pBuff );
  
  // Copy data to be written into local TX/RX buffer
  if( CAT24C512.pBuff != NULL )
    VOID memcpy( CAT24C512.pBuff + 2, pDataBytes, numBytes );
  
  // TX 16-bit address along with data bytes
  if( mujoeI2C_write( CAT24C512.i2cWriteAddr, ( numBytes + 2 ), CAT24C512.pBuff, STOP_CMD ) == ( numBytes + 2 ) )
    return TRUE;
  else
    return FALSE;
  
} // CAT24C512_writePage

// Read a single byte
bool CAT24C512_selectiveRead( uint16 pageAddr, uint8 byteAddr, uint8 *pByteData )
{
  // Drivers uninitialized, abort
  if( CAT24C512.i2cWriteAddr == 0x00 ) 
    return FALSE;
  
  // Build TX payload
  uint8 txBuff[2] ={0};
  CAT24C512_buildAddrPayload( pageAddr, byteAddr, txBuff );

  // TX payload
  if( mujoeI2C_write( CAT24C512.i2cWriteAddr, 2, txBuff, REPEAT_CMD ) == 2 )
  {
     uint8 rxBuff;
     if( mujoeI2C_read( CAT24C512.i2cWriteAddr, 1, &rxBuff ) )
     {
       *pByteData = rxBuff;
       return TRUE;
     }
     else
       return FALSE;
  }
  else
    return FALSE;
  
} // CAT24C512_selectiveRead

bool CAT24C512_sequentialRead( uint16 stPageAddr, uint8 stByteAddr, uint8 *pByteData, uint8 numBytes )
{
  // Drivers uninitialized or Buff size is too small for read operation, abort
  if( CAT24C512.i2cWriteAddr == 0x00  || numBytes > CAT24C512.buffSize ) 
    return FALSE;
  
  // Build TX payload
  uint8 txBuff[2] = {0};
  CAT24C512_buildAddrPayload( stPageAddr, stByteAddr, txBuff );
  
  if( mujoeI2C_write( CAT24C512.i2cWriteAddr, 2, txBuff, REPEAT_CMD ) == 2 )
  {
     if( mujoeI2C_read( CAT24C512.i2cWriteAddr, numBytes, CAT24C512.pBuff ) == numBytes )
     {
       // Copy RX'd data into output buffer arg
       for( uint8 i = 0; i < numBytes; i++ )
       {
         pByteData[i] = CAT24C512.pBuff[i];
       }
       return TRUE;
     }
     else
       return FALSE;
  }
  else
    return FALSE;
  
} // CAT24C512_sequentialRead

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Prepares the page address and byte address into a 2-byte array payload in preparation
// for a serial I2C transaction.
static void CAT24C512_buildAddrPayload( uint16 pageAddr, uint8 byteAddr, uint8 *pPayload )
{
  // Clamp page and byte addresses to last address if exceeded
  if( pageAddr > CAT24C512_LAST_PAGE_ADDR ) { pageAddr = CAT24C512_LAST_PAGE_ADDR; }
  if( byteAddr > CAT24C512_LAST_BYTE_ADDR ) { byteAddr = CAT24C512_LAST_BYTE_ADDR; }
  
  // Transfer LSb of pageAddr to MSb of addrLsbyte
  uint8 addrLsbyte = byteAddr;
  if( pageAddr & 0x0001 )    
    addrLsbyte |= 0x80;
  
  pPayload[0] = (uint8)(pageAddr >> 1); // Load Address MSByte
  pPayload[1] = addrLsbyte;             // Load Address LSByte
}




