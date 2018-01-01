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
#define MUJOEDATAPROFILE_ASYNCBULK             0  // Notify - Async Bulk Characteristic
#define MUJOEDATAPROFILE_SYNCBULK              1  // R - Sync Bulk Characteristic
#define MUJOEDATAPROFILE_ASYNC                 2  // Notify - Async Characteristic      

// muJoe Data Profile Service UUID
#define MUJOEDATAPROFILE_SERV_UUID             0xFFE0

// Characteristic UUIDs
#define MUJOEDATAPROFILE_ASYNCBULK_UUID        0xFFE1
#define MUJOEDATAPROFILE_SYNCBULK_UUID         0xFFE2
#define MUJOEDATAPROFILE_ASYNC_UUID            0xFFE3 

// Length of Async Bulk Characteristic in bytes
#define MUJOEDATAPROFILE_ASYNCBULK_LEN         20

// Length of Sync Bulk Characteristic in bytes
#define MUJOEDATAPROFILE_SYNCBULK_LEN          20 

// Length of Async Characteristic in bytes
#define MUJOEDATAPROFILE_ASYNC_LEN             4 

// Number of Characteristics within the muJoe Data Service
#define MUJOEDATAPROFILE_NUM_CHAR              3

////////////////////////////////////////////////////////////////////////////////
// Profile Callbacks
////////////////////////////////////////////////////////////////////////////////

// Callback when a characteristic value has changed
typedef void (*muJoeDataProfileChange_t)( uint8 paramID );

typedef struct
{
  muJoeDataProfileChange_t        pfnDataProfileChange;  // Called when characteristic value changes
  muJoeDataProfileChange_t        pfnDataProfileRead;    // Called when a characteristic is read by central

} muJoeDataProfileCBs_t;


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
bStatus_t muJoeDataProfile_writeAsync( uint8 *pAsyncBuff, uint8 buffSize );
bStatus_t muJoeDataProfile_readAsyncBulk( uint8 *pAsyncBulkBuff, uint8 buffSize );
void muJoeDataProfile_clearAsyncBulk( void );
bStatus_t muJoeDataProfile_writeSyncBulk( uint8 *pSyncBulkBuff, uint8 buffSize );
void muJoeDataProfile_clearSyncBulk( void );

#endif