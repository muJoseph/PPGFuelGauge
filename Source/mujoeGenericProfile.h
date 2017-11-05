////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfile.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEGENERICPROFILE_H
#define MUJOEGENERICPROFILE_H

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
#define MUJOEGENERICPROFILE_COMMAND             0  // RW uint8 - Profile Command Characteristic value 
#define MUJOEGENERICPROFILE_RESPONSE            1  // R/Notify uint8 - Profile Response Characteristic value 
#define MUJOEGENERICPROFILE_MAILBOX             2  // RW uint8 - Profile Mailbox Characteristic value       
#define MUJOEGENERICPROFILE_DEVICEINFO          3  // R uint8 - Profile Device Info Characteristic value

// muJoe Generic Profile Services bit fields
#define MUJOEGENERICPROFILE_SERVICE               0x00000001

// muJoe Generic Profile Service UUID
#define MUJOEGENERICPROFILE_SERV_UUID             0xFFF0

// Characteristic UUIDs
#define MUJOEGENERICPROFILE_COMMAND_UUID          0xFFF1
#define MUJOEGENERICPROFILE_RESPONSE_UUID         0xFFF2
#define MUJOEGENERICPROFILE_MAILBOX_UUID          0xFFF3
#define MUJOEGENERICPROFILE_DEVICEINFO_UUID       0xFFF4

// Length of Command Characteristic in bytes
#define MUJOEGENERICPROFILE_CMD_LEN               2

// Length of Reponse Characteristic in bytes
#define MUJOEGENERICPROFILE_RSP_LEN               2

// Length of Mailbox Characteristic in bytes
#define MUJOEGENERICPROFILE_MBOX_LEN              20

// Length of Device Info Characteristic in bytes
#define MUJOEGENERICPROFILE_DEVINFO_LEN           4

// Number of Characteristics within the muJoe Generic Service
#define MUJOEGENERICPROFILE_NUM_CHAR              4

////////////////////////////////////////////////////////////////////////////////
// Profile Callbacks
////////////////////////////////////////////////////////////////////////////////

// Callback when a characteristic value has changed
typedef void (*muJoeGenProfileChange_t)( uint8 paramID );

typedef struct
{
  muJoeGenProfileChange_t        pfnSimpleProfileChange;  // Called when characteristic value changes

} muJoeGenProfileCBs_t;

// muJoe Generic Service  Characteristic 
typedef struct muJoeGenService_Char_def
{
   uint8                paramId;
   uint16               uuid;
   uint8                size;
  
}muJoeGenService_Char_t;

// muJoe Generic Service 
typedef struct muJoeGenService_def
{
  muJoeGenService_Char_t       *muJoeGenService_charTbl;        // Characteristic Table
  uint8                         numChars;                       // Number of characteristics
}muJoeGenService_t;

////////////////////////////////////////////////////////////////////////////////
// PROTOS
////////////////////////////////////////////////////////////////////////////////

/*
 * MuJoeGenericProfile_AddService- Initializes the muJoe Generic GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t MuJoeGenericProfile_AddService( void );

/*
 * muJoeGenProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t muJoeGenProfile_RegisterAppCBs( muJoeGenProfileCBs_t *appCallbacks );

/*
 * muJoeGenProfile_SetParameter - Set a muJoe Generic GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t muJoeGenProfile_SetParameter( uint8 param, uint8 len, void *value );

/*
 * muJoeGenProfile_GetParameter - Get a muJoe Generic GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t muJoeGenProfile_GetParameter( uint8 param, void *value );

extern bStatus_t muJoeGenProfile_readCommand( uint16 *pCommandValue );
extern bStatus_t muJoeGenProfile_writeCommand( uint16 commandValue );
extern bStatus_t muJoeGenProfile_writeResponse( uint16 responseValue );
extern bStatus_t muJoeGenProfile_writeMailbox( uint8 *pMailboxBuff, uint8 buffSize );
extern bStatus_t muJoeGenProfile_readMailbox( uint8 *pMailboxBuff, uint8 buffSize );
  
#endif