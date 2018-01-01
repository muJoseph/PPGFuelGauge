////////////////////////////////////////////////////////////////////////////////
// @filename: mujoeDataProfile.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "mujoeDataProfile.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

//#define MUJOEDATA_NUM_ATTR_SUPPORTED        8
#define MUJOEDATA_NUM_ATTR_SUPPORTED          12

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

// Simple GATT Profile Service UUID: 0xFFE0
CONST uint8 mujoeDataProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(MUJOEDATAPROFILE_SERV_UUID), HI_UINT16(MUJOEDATAPROFILE_SERV_UUID)
};

// Async Bulk Characteristic UUID: 0xFFE1
CONST uint8 mujoeDataProfileAsyncBulkUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(MUJOEDATAPROFILE_ASYNCBULK_UUID), HI_UINT16(MUJOEDATAPROFILE_ASYNCBULK_UUID)
};

// Sync Bulk Characteristic UUID: 0xFFE2
CONST uint8 mujoeDataProfileSyncBulkUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(MUJOEDATAPROFILE_SYNCBULK_UUID), HI_UINT16(MUJOEDATAPROFILE_SYNCBULK_UUID)
};

// Async Characteristic UUID: 0xFFE3
CONST uint8 mujoeDataProfileAsyncUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(MUJOEDATAPROFILE_ASYNC_UUID), HI_UINT16(MUJOEDATAPROFILE_ASYNC_UUID)
};

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static muJoeDataProfileCBs_t *muJoeDataProfile_AppCBs = NULL;

// Profile Attributes - variables //////////////////////////////////////////////

// muJoe Data Profile Service attribute
static CONST gattAttrType_t muJoeDataProfileService = { ATT_BT_UUID_SIZE, mujoeDataProfileServUUID };

// muJoe Data Profile Async Bulk Characteristic Properties
static uint8  muJoeDataProfileAsyncBulkProps = GATT_PROP_NOTIFY;

// muJoe Data Profile Sync Bulk Characteristic Properties
static uint8  muJoeDataProfileSyncBulkProps = GATT_PROP_READ;

// muJoe Data Profile Async Characteristic Properties
static uint8  muJoeDataProfileAsyncProps = GATT_PROP_NOTIFY;

// Characteristic Values ///////////////////////////////////////////////////////

// Async Bulk Characteristic Value
static uint8 muJoeDataProfileAsyncBulk[MUJOEDATAPROFILE_ASYNCBULK_LEN] = {0};

// Sync Bulk Characteristic Value
static uint8 muJoeDataProfileSyncBulk[MUJOEDATAPROFILE_SYNCBULK_LEN] = {0};

// Async Characteristic Value
static uint8 muJoeDataProfileAsync[MUJOEDATAPROFILE_ASYNC_LEN] = {0};

// muJoe Data Profile Async Bulk Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *muJoeDataProfileAsyncBulkConfig; // NOTE: Create a seperate config for each characteristic that has Notifications enabled (prob has additional functionality but need more research)

// muJoe Data Profile Async Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *muJoeDataProfileAsyncConfig; // NOTE: Create a seperate config for each characteristic that has Notifications enabled (prob has additional functionality but need more research)

// muJoe Data Profile Async Bulk Characteristic User Description
static uint8 muJoeDataProfileAsyncBulkUserDesp[11] = "Async Bulk";

// muJoe Data Profile Sync Bulk Characteristic User Description
static uint8 muJoeDataProfileSyncBulkUserDesp[10] = "Sync Bulk";

// muJoe Data Profile Async Characteristic User Description
static uint8 mujoeDataProfileAsyncUserDesp[6] = "Async";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t muJoeDataProfileAttrTbl[MUJOEDATA_NUM_ATTR_SUPPORTED] = 
{
  // Index 0
  // muJoe Data Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&muJoeDataProfileService          /* pValue */
  },

  // ASYNC BULK CHARACTERISTIC //////////////////////////////////////////////////
  
  // Index 1
  // Async Bulk Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeDataProfileAsyncBulkProps 
  },

  // Index 2
  // Async Bulk Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeDataProfileAsyncBulkUUID },
    0,                                  // NOTE: Should be zero, do NOT match muJoeDataProfileRspProps. Will need investigate further.
    0, 
    muJoeDataProfileAsyncBulk 
  },
  
  // Index 3
  // Async Bulk Characteristic Configuration
  { 
    { ATT_BT_UUID_SIZE, clientCharCfgUUID },
    GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
    0, 
    (uint8 *)&muJoeDataProfileAsyncBulkConfig 
  },

  // Index 4
  // Async Bulk Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    muJoeDataProfileAsyncBulkUserDesp 
  },

  // SYNC BULK CHARACTERISTIC //////////////////////////////////////////////////
  
  // Index 5
  // Sync Bulk Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeDataProfileSyncBulkProps 
  },

  // Index 6
  // Sync Bulk Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeDataProfileSyncBulkUUID },
    GATT_PERMIT_READ,
    0, 
    muJoeDataProfileSyncBulk 
  },
  
  // Index 7
  // Sync Bulk Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    muJoeDataProfileSyncBulkUserDesp 
  },
  
  // ASYNC CHARACTERISTIC //////////////////////////////////////////////////////
  
  // Index 8
  // Async Characteristic Declaration
  { 
    { ATT_BT_UUID_SIZE, characterUUID },
    GATT_PERMIT_READ, 
    0,
    &muJoeDataProfileAsyncProps 
  },

  // Index 9
  // Async Characteristic Value
  { 
    { ATT_BT_UUID_SIZE, mujoeDataProfileAsyncUUID },
    0,                                  // NOTE: Should be zero, do NOT match muJoeDataProfileRspProps. Will need investigate further.
    0, 
    muJoeDataProfileAsync 
  },
  
  // Index 10
  // Async Characteristic Configuration
  { 
    { ATT_BT_UUID_SIZE, clientCharCfgUUID },
    GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
    0, 
    (uint8 *)&muJoeDataProfileAsyncConfig 
  },

  // Index 11
  // Async Characteristic User Description
  { 
    { ATT_BT_UUID_SIZE, charUserDescUUID },
    GATT_PERMIT_READ, 
    0, 
    mujoeDataProfileAsyncUserDesp 
  },

  
};

//////////////////////////////////////////////////////////////////////
// Local Functions 
//////////////////////////////////////////////////////////////////////

static bStatus_t muJoeDataProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method );
static bStatus_t muJoeDataProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method );

static bStatus_t muJoeDataProfile_GetParameter( uint8 param, void *value );
static bStatus_t muJoeDataProfile_SetParameter( uint8 param, uint8 len, void *value );

//////////////////////////////////////////////////////////////////////
// Profile Callbacks
//////////////////////////////////////////////////////////////////////

// muJoe Data Profile Service Callbacks
CONST gattServiceCBs_t muJoeDataProfileCBs =
{
  muJoeDataProfile_ReadAttrCB,  // Read callback function pointer
  muJoeDataProfile_WriteAttrCB, // Write callback function pointer
  NULL                          // Authorization callback function pointer
};

//////////////////////////////////////////////////////////////////////
// API FUNCTIONS
//////////////////////////////////////////////////////////////////////

/*********************************************************************
 * @fn      MuJoeDataProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t MuJoeDataProfile_AddService( void )
{
  uint8 status;
  
  // Allocate Client Characteristic Configuration table for Async Bulk Characteristic
  muJoeDataProfileAsyncBulkConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                                     linkDBNumConns );
 
  // Allocate Client Characteristic Configuration table for Async Characteristic
  muJoeDataProfileAsyncConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                                 linkDBNumConns );
   
  if ( (muJoeDataProfileAsyncBulkConfig == NULL) || (muJoeDataProfileAsyncConfig == NULL) )
  {     
    return ( bleMemAllocError );
  }
  
  // Initialize Client Characteristic Configuration attributes for Async Bulk Characteristic
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, muJoeDataProfileAsyncBulkConfig );      // GATTServApp_InitCharCfg must be called for all characteristics with notifications enabled
  
  // Initialize Client Characteristic Configuration attributes for Async Characteristic
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, muJoeDataProfileAsyncConfig );
  
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( muJoeDataProfileAttrTbl, 
                                        GATT_NUM_ATTRS( muJoeDataProfileAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &muJoeDataProfileCBs );
  return ( status );
  
} // MuJoeGenericProfile_AddService

/*********************************************************************
 * @fn      muJoeDataProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t muJoeDataProfile_RegisterAppCBs( muJoeDataProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    muJoeDataProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
} // muJoeDataProfile_RegisterAppCBs

bStatus_t muJoeDataProfile_readAsyncBulk( uint8 *pAsyncBulkBuff, uint8 buffSize )
{
  if( buffSize >= MUJOEDATAPROFILE_ASYNCBULK_LEN )
    return muJoeDataProfile_GetParameter( MUJOEDATAPROFILE_ASYNCBULK, pAsyncBulkBuff );
  else
    return FAILURE;
  
} // muJoeDataProfile_readAsyncBulk

bStatus_t muJoeDataProfile_writeAsyncBulk( uint8 *pAsyncBulkBuff, uint8 buffSize )
{
  return muJoeDataProfile_SetParameter( MUJOEDATAPROFILE_ASYNCBULK, 
                                        buffSize, 
                                        pAsyncBulkBuff );
} // muJoeDataProfile_writeAsyncBulk


bStatus_t muJoeDataProfile_writeAsync( uint8 *pAsyncBuff, uint8 buffSize )
{
  return muJoeDataProfile_SetParameter( MUJOEDATAPROFILE_ASYNC, 
                                        buffSize, 
                                        pAsyncBuff );
} // muJoeDataProfile_writeAsync

void muJoeDataProfile_clearAsyncBulk( void )
{
  VOID memset(muJoeDataProfileAsyncBulk, 0, MUJOEDATAPROFILE_ASYNCBULK_LEN);
  
} // muJoeDataProfile_clearAsyncBulk

bStatus_t muJoeDataProfile_writeSyncBulk( uint8 *pSyncBulkBuff, uint8 buffSize )
{
  return muJoeDataProfile_SetParameter( MUJOEDATAPROFILE_SYNCBULK, 
                                        buffSize, 
                                        pSyncBulkBuff ); 
} // muJoeDataProfile_writeSyncBulk


void muJoeDataProfile_clearSyncBulk( void )
{
  VOID memset(muJoeDataProfileSyncBulk, 0, MUJOEDATAPROFILE_SYNCBULK_LEN);
  
} // muJoeDataProfile_clearSyncBulk

//////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
//////////////////////////////////////////////////////////////////////

/*********************************************************************
 * @fn          muJoeDataProfile_ReadAttrCB
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
static bStatus_t muJoeDataProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;

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
      
      // Async Bulk Characteristic does not have read permissions, but because it
      // can be sent as a notification, it is included here.
      case MUJOEDATAPROFILE_ASYNCBULK_UUID:
        *pLen = MUJOEDATAPROFILE_ASYNCBULK_LEN;  
        VOID memcpy( pValue, pAttr->pValue, MUJOEDATAPROFILE_ASYNCBULK_LEN );
        break;
      // Sync Bulk Characteristic has read permissions
      case MUJOEDATAPROFILE_SYNCBULK_UUID:
        *pLen = MUJOEDATAPROFILE_SYNCBULK_LEN;  
        VOID memcpy( pValue, pAttr->pValue, MUJOEDATAPROFILE_SYNCBULK_LEN );
        notifyApp = MUJOEDATAPROFILE_SYNCBULK;
        break;
      // Async Characteristic does not have read permissions, but because it
      // can be sent as a notification, it is included here.
      case MUJOEDATAPROFILE_ASYNC_UUID:
        *pLen = MUJOEDATAPROFILE_ASYNC_LEN;  
        VOID memcpy( pValue, pAttr->pValue, MUJOEDATAPROFILE_ASYNC_LEN );
        break;
      default:
        // Should never get here!
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

  // If a charactersitic is read then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && muJoeDataProfile_AppCBs && muJoeDataProfile_AppCBs->pfnDataProfileRead )
  {
    muJoeDataProfile_AppCBs->pfnDataProfileRead( notifyApp );  
  }
  return ( status );
  
} // muJoeDataProfile_ReadAttrCB

/*********************************************************************
 * @fn      muJoeDataProfile_WriteAttrCB
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
static bStatus_t muJoeDataProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
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
  if ( (notifyApp != 0xFF ) && muJoeDataProfile_AppCBs && muJoeDataProfile_AppCBs->pfnDataProfileChange )
  {
    muJoeDataProfile_AppCBs->pfnDataProfileChange( notifyApp );  
  }
  
  return ( status );
} // muJoeDataProfile_WriteAttrCB

/*********************************************************************
 * @fn      muJoeDataProfile_SetParameter
 *
 * @brief   Set a muJoe Data Profile parameter.
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
static bStatus_t muJoeDataProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MUJOEDATAPROFILE_ASYNCBULK:
      if( len <= MUJOEDATAPROFILE_ASYNCBULK_LEN )
      {
        VOID memcpy( muJoeDataProfileAsyncBulk, value, len );
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( muJoeDataProfileAsyncBulkConfig, muJoeDataProfileAsyncBulk, FALSE,
                                    muJoeDataProfileAttrTbl, GATT_NUM_ATTRS( muJoeDataProfileAttrTbl ),
                                    INVALID_TASK_ID, muJoeDataProfile_ReadAttrCB );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    case MUJOEDATAPROFILE_SYNCBULK:
      if( len <= MUJOEDATAPROFILE_SYNCBULK_LEN )
        VOID memcpy( muJoeDataProfileSyncBulk, value, len ); 
      else
        ret = bleInvalidRange;
      break;
    case MUJOEDATAPROFILE_ASYNC:
      if( len <= MUJOEDATAPROFILE_ASYNC_LEN )
      {
        VOID memcpy( muJoeDataProfileAsync, value, len );
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( muJoeDataProfileAsyncConfig, muJoeDataProfileAsync, FALSE,
                                    muJoeDataProfileAttrTbl, GATT_NUM_ATTRS( muJoeDataProfileAttrTbl ),
                                    INVALID_TASK_ID, muJoeDataProfile_ReadAttrCB );
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
} // muJoeDataProfile_SetParameter


/*********************************************************************
 * @fn      muJoeDataProfile_GetParameter
 *
 * @brief   Get a muJoe Generic Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
static bStatus_t muJoeDataProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MUJOEDATAPROFILE_ASYNCBULK:
      VOID memcpy( value, muJoeDataProfileAsyncBulk, MUJOEDATAPROFILE_ASYNCBULK_LEN );
      break;
    case MUJOEDATAPROFILE_SYNCBULK:
      VOID memcpy( value, muJoeDataProfileSyncBulk, MUJOEDATAPROFILE_SYNCBULK_LEN );
      break;
    case MUJOEDATAPROFILE_ASYNC:
      VOID memcpy( value, muJoeDataProfileAsync, MUJOEDATAPROFILE_ASYNC_LEN );
      break;
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
} // muJoeDataProfile_GetParameter


                                             