////////////////////////////////////////////////////////////////////////////////
// @filename: CAT24C512.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef CAT24C512_H
#define CAT24C512_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"
#include "mujoeI2C.h"
#include "OSAL_Memory.h"        // for osal_mem_alloc
#include "string.h"             // for memcpy

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define CAT24C512_FIRST_PAGE_ADDR       0
#define CAT24C512_LAST_PAGE_ADDR        511

#define CAT24C512_FIRST_BYTE_ADDR       0
#define CAT24C512_LAST_BYTE_ADDR        127

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef struct memMgr_def
{
   uint16               currHdrAddr;
   uint16               currTlAddr;
   
}memMgr_t;

typedef struct CAT24C512_def
{
  uint8         i2cWriteAddr;
  uint8         *pBuff;                 // Pointer to RX/TX buff
  uint8         buffSize;               // Size of buffer that pBuff pts to
  memMgr_t      memMgr;
  
}CAT24C512_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool CAT24C512_initDriver( uint8 buffSize, bool a2, bool a1, bool a0 );
bool CAT24C512_initHardware( void );
bool CAT24C512_writeByte( uint16 pageAddr, uint8 byteAddr, uint8 byteData );
bool CAT24C512_writePage( uint16 stPageAddr, uint8 stByteAddr, uint8 *pDataBytes, uint8 numBytes );
bool CAT24C512_selectiveRead( uint16 pageAddr, uint8 byteAddr, uint8 *pByteData );
bool CAT24C512_sequentialRead( uint16 stPageAddr, uint8 stByteAddr, uint8 *pByteData, uint8 numBytes );

#endif // CAT24C512_H