////////////////////////////////////////////////////////////////////////////////
// @filename: mujoeDataProfile.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEDATAPROFILE_H
#define MUJOEDATAPROFILE_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINE
////////////////////////////////////////////////////////////////////////////////

// Profile Parameter IDs
#define MUJOEDATAPROFILE_ASYNCBULK             0  // R/Notify - Bulk Data Characteristic

// muJoe Generic Profile Service UUID
#define MUJOEDATAPROFILE_SERV_UUID             0xFFE0

// Characteristic UUIDs
#define MUJOEDATAPROFILE_ASYNCBULK_UUID        0xFFE1

// Length of Bulk Data Characteristic in bytes
#define MUJOEDATAPROFILE_ASYNCBULK_LEN         20

// Number of Characteristics within the muJoe Data Service
#define MUJOEDATAPROFILE_NUM_CHAR              1

////////////////////////////////////////////////////////////////////////////////
// Profile Callbacks
////////////////////////////////////////////////////////////////////////////////

// Callback when a characteristic value has changed
typedef void (*muJoeDataProfileChange_t)( uint8 paramID );

typedef struct
{
  muJoeDataProfileChange_t        pfnDataProfileChange;  // Called when characteristic value changes

} muJoeDataProfileCBs_t;

// muJoe Generic Service  Characteristic 
typedef struct muJoeDataService_Char_def
{
   uint8                paramId;
   uint16               uuid;
   uint8                size;
  
}muJoeDataService_Char_t;

// muJoe Generic Service 
typedef struct muJoeDataService_def
{
  muJoeDataService_Char_t       *muJoeDataService_charTbl;        // Characteristic Table
  uint8                         numChars;                       // Number of characteristics
}muJoeDataService_t;

////////////////////////////////////////////////////////////////////////////////
// PROTOS
////////////////////////////////////////////////////////////////////////////////

/*
 * MuJoeDataProfile_AddService- Initializes the muJoe Generic GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

bStatus_t MuJoeDataProfile_AddService( void );

/*
 * muJoeDataProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t muJoeDataProfile_RegisterAppCBs( muJoeDataProfileCBs_t *appCallbacks );

bStatus_t muJoeDataProfile_writeAsyncBulk( uint8 *pAsyncBulkBuff, uint8 buffSize );
bStatus_t muJoeDataProfile_readAsyncBulk( uint8 *pAsyncBulkBuff, uint8 buffSize );
  
#endif