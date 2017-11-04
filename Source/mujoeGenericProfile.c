////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfile.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////


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

#include "muJoeGenericProfile.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define MUJOEGEN_NUM_ATTR_SUPPORTED        13

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 mujoeGenericProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(MUJOEGENERICPROFILE_SERV_UUID), HI_UINT16(MUJOEGENERICPROFILE_SERV_UUID)
};

// Command Characteristic UUID: 0xFFF1
CONST uint8 mujoeGenericProfileCmdUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(MUJOEGENERICPROFILE_COMMAND_UUID), HI_UINT16(MUJOEGENERICPROFILE_COMMAND_UUID)
};

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static muJoeGenProfileCBs_t *muJoeGenProfile_AppCBs = NULL;

// Profile Attributes - variables //////////////////////////////////////////////

// muJoe Generic Profile Service attribute
static CONST gattAttrType_t muJoeGenProfileService = { ATT_BT_UUID_SIZE, mujoeGenericProfileServUUID };

// muJoe Generic Profile Command Characteristic Properties
static uint8  muJoeGenProfileCmdProps = GATT_PROP_READ | GATT_PROP_WRITE;

// muJoe Generic Profile Response Characteristic Properties
static uint8  muJoeGenProfileRspProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

// muJoe Generic Profile Mailbox Characteristic Properties
static uint8  muJoeGenProfileMboxProps = GATT_PROP_READ | GATT_PROP_WRITE;

// muJoe Generic Profile Device Info Characteristic Properties
static uint8  muJoeGenProfileDevinfoProps = GATT_PROP_READ;

// Characteristic Values ///////////////////////////////////////////////////////

// Command Characteristic Value
static uint8 muJoeGenProfileCmd[MUJOEGENERICPROFILE_CMD_LEN] = {0};

// Response Characteristic Value
static uint8 muJoeGenProfileRsp[MUJOEGENERICPROFILE_RSP_LEN] = {0};

// Mailbox Characteristic Value
static uint8 muJoeGenProfileMbox[MUJOEGENERICPROFILE_MBOX_LEN] = {0};

// Device Info Characteristic Value
static uint8 muJoeGenProfileDevinfo[MUJOEGENERICPROFILE_DEVINFO_LEN] = {0};

// muJoe Generic Profile Response Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *muJoeGenProfileRspConfig; // NOTE: Create a seperate config for each characteristic that has Notifications enabled (prob has additional functionality but need more research)

// muJoe Generic Profile Command Characteristic User Description
static uint8 muJoeGenProfileCmdUserDesp[8] = "Command";

// muJoe Generic Profile Response Characteristic User Description
static uint8 muJoeGenProfileRspUserDesp[9] = "Response";

// muJoe Generic Profile Mailbox Characteristic User Description
static uint8 muJoeGenProfileMboxUserDesp[8] = "Mailbox";

// muJoe Generic Profile Device Info Characteristic User Description
static uint8 muJoeGenProfileDevinfoUserDesp[12] = "Device Info";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t muJoeGenProifleAttrTbl[MUJOEGEN_NUM_ATTR_SUPPORTED] = 
{
  // muJoe Generic Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&muJoeGenProfileService          /* pValue */
  },

  // COMMAND CHARACTERISTIC ////////////////////////////////////////////////////
  
  // Command Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeGenProfileCmdProps 
  },

  // Command Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeGenericProfileCmdUUID },
    GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
    0, 
    muJoeGenProfileCmd 
  },

  // Command Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    muJoeGenProfileCmdUserDesp 
  },
  
  // RESPONSE CHARACTERISTIC ///////////////////////////////////////////////////
  
  // Response Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeGenProfileRspProps 
  },

  // Response Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeGenericProfileCmdUUID },
    GATT_PROP_READ | GATT_PROP_NOTIFY, 
    0, 
    muJoeGenProfileRsp 
  },

  // Response Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    muJoeGenProfileRspUserDesp 
  },  
  
  // MAILBOX CHARACTERISTIC ////////////////////////////////////////////////////
  
  // Mailbox Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeGenProfileMboxProps 
  },

  // Mailbox Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeGenericProfileCmdUUID },
    GATT_PROP_READ | GATT_PROP_WRITE,
    0, 
    muJoeGenProfileMbox 
  },

  // Mailbox Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    muJoeGenProfileMboxUserDesp 
  },
  
  // DEVICE INFO CHARACTERISTIC ////////////////////////////////////////////////
  
  // Device Info Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeGenProfileDevinfoProps 
  },

  // Device Info Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeGenericProfileCmdUUID },
    GATT_PROP_READ | GATT_PROP_WRITE,
    0, 
    muJoeGenProfileDevinfo
  },

  // Device Info Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    muJoeGenProfileDevinfoUserDesp 
  },
  
};

//////////////////////////////////////////////////////////////////////
// Local Functions 
//////////////////////////////////////////////////////////////////////

static bStatus_t muJoeGenProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method );
static bStatus_t muJoeGenProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method );

//////////////////////////////////////////////////////////////////////
// Profile Callbacks
//////////////////////////////////////////////////////////////////////

// Simple Profile Service Callbacks
CONST gattServiceCBs_t muJoeGenProfileCBs =
{
  muJoeGenProfile_ReadAttrCB,  // Read callback function pointer
  muJoeGenProfile_WriteAttrCB, // Write callback function pointer
  NULL                         // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      MuJoeGenericProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t MuJoeGenericProfile_AddService( void )
{
  uint8 status;
  
  // Allocate Client Characteristic Configuration table
  muJoeGenProfileRspConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                              linkDBNumConns );
  if ( muJoeGenProfileRspConfig == NULL )
  {     
    return ( bleMemAllocError );
  }
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, muJoeGenProfileRspConfig );      // GATTServApp_InitCharCfg must be called for all characteristics with notifications enabled
  
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( muJoeGenProifleAttrTbl, 
                                        GATT_NUM_ATTRS( muJoeGenProifleAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &muJoeGenProfileCBs );
  return ( status );
  
} // MuJoeGenericProfile_AddService

/*********************************************************************
 * @fn          muJoeGenProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t muJoeGenProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those reads
      case MUJOEGENERICPROFILE_COMMAND_UUID:
      case MUJOEGENERICPROFILE_RESPONSE_UUID:
        *pLen = MUJOEGENERICPROFILE_CMD_LEN;    // Note: CMD and RSP are of equal length
        VOID memcpy( pValue, pAttr->pValue, MUJOEGENERICPROFILE_CMD_LEN );
        break;
      case MUJOEGENERICPROFILE_MAILBOX_UUID:
        *pLen = MUJOEGENERICPROFILE_MBOX_LEN;
        VOID memcpy( pValue, pAttr->pValue, MUJOEGENERICPROFILE_MBOX_LEN );
        break;
      case MUJOEGENERICPROFILE_DEVICEINFO_UUID:
        *pLen = MUJOEGENERICPROFILE_DEVINFO_LEN;
        VOID memcpy( pValue, pAttr->pValue, MUJOEGENERICPROFILE_DEVINFO_LEN );
        break;
      default:
        // Should never get here! (characteristics 3 and 4 do not have read permissions)
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
  
} // muJoeGenProfile_ReadAttrCB

/*********************************************************************
 * @fn      muJoeGenProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t muJoeGenProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  uint8 *pCurValue = NULL;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case MUJOEGENERICPROFILE_COMMAND_UUID:
        // Make sure it's not a blob oper
        if ( offset == 0 )
        {
          // Validate length of data
          if ( len != MUJOEGENERICPROFILE_CMD_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }
        
        // Write the value
        if ( status == SUCCESS )
        {
          pCurValue = (uint8 *)pAttr->pValue;
          VOID memcpy( pCurValue + offset, pValue, len );
          notifyApp = MUJOEGENERICPROFILE_COMMAND;
        }  
        break;
      case MUJOEGENERICPROFILE_MAILBOX_UUID:
        // Make sure it's not a blob oper
        if ( offset == 0 )
        {
          // Validate length of data
          if ( len != MUJOEGENERICPROFILE_MBOX_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }
        
        // Write the value
        if ( status == SUCCESS )
        {
          pCurValue = (uint8 *)pAttr->pValue;
          VOID memcpy( pCurValue + offset, pValue, len );
          notifyApp = MUJOEGENERICPROFILE_MAILBOX;
        }  
        break;
      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        break;
      default:
        // Should never get here! (characteristics 2 and 4 do not have write permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && muJoeGenProfile_AppCBs && muJoeGenProfile_AppCBs->pfnSimpleProfileChange )
  {
    muJoeGenProfile_AppCBs->pfnSimpleProfileChange( notifyApp );  
  }
  
  return ( status );
} // muJoeGenProfile_WriteAttrCB

/*********************************************************************
 * @fn      muJoeGenProfile_SetParameter
 *
 * @brief   Set a muJoe Generic Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t muJoeGenProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MUJOEGENERICPROFILE_COMMAND:
      if( len <= MUJOEGENERICPROFILE_CMD_LEN )
      {
        VOID memcpy( muJoeGenProfileCmd, value, len );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    /*case MUJOEGENERICPROFILE_RESPONSE:
      if( len <= MUJOEGENERICPROFILE_RSP_LEN )
      {
        VOID memcpy( muJoeGenProfileRsp, value, len );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;*/
    case MUJOEGENERICPROFILE_RESPONSE:
      if( len <= MUJOEGENERICPROFILE_RSP_LEN )
      //if ( len == sizeof ( uint8 ) ) 
      {
        VOID memcpy( muJoeGenProfileRsp, value, len );
        //simpleProfileChar4 = *((uint8*)value);
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( muJoeGenProfileRspConfig, muJoeGenProfileRsp, FALSE,
                                    muJoeGenProifleAttrTbl, GATT_NUM_ATTRS( muJoeGenProifleAttrTbl ),
                                    INVALID_TASK_ID, muJoeGenProfile_ReadAttrCB );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    case MUJOEGENERICPROFILE_MAILBOX:
      if( len <= MUJOEGENERICPROFILE_MBOX_LEN )
      {
        VOID memcpy( muJoeGenProfileMbox, value, len );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    case MUJOEGENERICPROFILE_DEVICEINFO:
      if( len <= MUJOEGENERICPROFILE_DEVINFO_LEN )
      {
        VOID memcpy( muJoeGenProfileDevinfo, value, len );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
} // muJoeGenProfile_SetParameter

/*********************************************************************
 * @fn      muJoeGenProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t muJoeGenProfile_RegisterAppCBs( muJoeGenProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    muJoeGenProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
} // muJoeGenProfile_RegisterAppCBs