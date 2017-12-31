////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfileMgr.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "muJoeGenericProfileMgr.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL VARS 
////////////////////////////////////////////////////////////////////////////////

static muJoeGenMgr_t           muJoeGenMgr;
static uint8                   i2cRxData[20];

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

static void issueResponse( uint16 rspCode );
static uint16 cmdGroup_sysGrp( uint8 cmd_id );
static uint16 cmdGroup_datGrp( uint8 cmd_id );
static uint16 cmdGroup_mspDbgGrp( uint8 cmd_id );

static bStatus_t getAsyncSamplePeriod( uint32 *pSampPeriod );
static bStatus_t getI2cWritePayload( uint8 *pTxData, uint8 *pNumBytes );
static bStatus_t getI2cNumRxBytes( uint8 *pNumBytes );
static bStatus_t getMailboxByteField( uint8 *pBuff, uint8 buffSize, uint8 stIndex, uint8 spIndex );

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void muJoeGenMgr_initDriver( muJoeGenMgr_t s )
{
  // Init Response Handler Callback
  muJoeGenMgr.muJoeGenMgr_rspHdlrCb.pRspBuff = s.muJoeGenMgr_rspHdlrCb.pRspBuff;
  muJoeGenMgr.muJoeGenMgr_rspHdlrCb.rspHdlrCb.tskId = s.muJoeGenMgr_rspHdlrCb.rspHdlrCb.tskId;
  muJoeGenMgr.muJoeGenMgr_rspHdlrCb.rspHdlrCb.evtFlg = s.muJoeGenMgr_rspHdlrCb.rspHdlrCb.evtFlg;

  muJoeGenMgr.asyncBulkCb.tskId =       s.asyncBulkCb.tskId;
  muJoeGenMgr.asyncBulkCb.evtFlg =      s.asyncBulkCb.evtFlg;
  
} // muJoeGenMgr_initDriver

bStatus_t muJoeGenMgr_cmdWriteHandler( void )
{
  bStatus_t bStatus = SUCCESS;
  uint16 cmdVal;
  uint16 rspVal = MUJOE_RSP_SUCCESS;
  uint8  cmd_group, cmd_id;
  
  if( muJoeGenProfile_readCommand( &cmdVal ) != SUCCESS )       // Read Command Value
    return bStatus;
  cmd_group     = HI_UINT16(cmdVal);
  cmd_id        = LO_UINT16(cmdVal);
  
  switch( cmd_group )
  {
    case MUJOE_CMD_GRP_SYS:
      rspVal = cmdGroup_sysGrp( cmd_id );
      break;
    case MUJOE_CMD_GRP_DAT:
      rspVal = cmdGroup_datGrp( cmd_id );
      break;
    case MUJOE_CMD_GRP_MSPDBG:
      rspVal = cmdGroup_mspDbgGrp( cmd_id );
      break;
    // Unsupported Command Group
    default:
      rspVal = MUJOE_RSP_INV_CMD_GRP;
      break;
  }
  
  // Schedule a Response
  issueResponse( rspVal );
  
  return bStatus;
}

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void issueResponse( uint16 rspCode )
{
   *(muJoeGenMgr.muJoeGenMgr_rspHdlrCb.pRspBuff) = rspCode;
  
   osal_start_timerEx( muJoeGenMgr.muJoeGenMgr_rspHdlrCb.rspHdlrCb.tskId, 
                      muJoeGenMgr.muJoeGenMgr_rspHdlrCb.rspHdlrCb.evtFlg, 
                      MUJOE_DEFAULT_RSPDELAY );
} // issueResponse

static uint16 cmdGroup_sysGrp( uint8 cmd_id )
{
  uint16 rspVal = MUJOE_RSP_SUCCESS;
  switch(cmd_id)
  {
    case MUJOE_GRP_SYS_ID_PWRDWN:
      // Call fnc that powers down board
      break;
    // Unsupported Command ID for this Command Group
    default:
      rspVal = MUJOE_RSP_INV_CMD_ID;
      break;
  }
  
  return rspVal;
}

static uint16 cmdGroup_mspDbgGrp( uint8 cmd_id )
{
  uint16 rspVal = MUJOE_RSP_SUCCESS;
  
  // Save Current I2C frequency
  i2cClock_t i2cClockCtxSave = mujoeI2C_getSclFreq();
  mujoeI2C_setSclFreq( i2cClock_33KHZ );
  
  switch(cmd_id)
  {
    // I2C Write Bulk
    case MUJOE_GRP_MSPDBG_ID_I2CWRITE:
    {
      uint8 i2cTxData[19] = {0};
      uint8 i2cTxNumBytes = 0;
      getI2cWritePayload( i2cTxData, &i2cTxNumBytes );
      if( mujoeI2C_write( (0x48<<1), i2cTxNumBytes, i2cTxData, STOP_CMD ) != i2cTxNumBytes )
        rspVal = MUJOE_RSP_FAILURE;
      break;
    }
    // I2C Read Bulk
    case MUJOE_GRP_MSPDBG_ID_I2CREAD:
    {
      uint8 i2cRxData[20] = {0};
      uint8 i2cRxNumBytes = 0;
      getI2cNumRxBytes(&i2cRxNumBytes);
      // RX bytes from MSPFuelGauge, and write data to Mailbox characteristic
      if( mujoeI2C_read( (0x48<<1), i2cRxNumBytes, i2cRxData ) == i2cRxNumBytes )
        muJoeGenProfile_writeMailbox( i2cRxData, 20 );
      else
        rspVal = MUJOE_RSP_FAILURE;
      break;
    }
    // I2C Read Register
    case MUJOE_GRP_MSPDBG_ID_I2CREADREG:
    {
      uint8 mspfgRegAddr = 0;
      getMailboxByteField( &mspfgRegAddr, 1, 0, 0 );
      if( mspfgRegAddr < MSPFG_NUM_REGISTERS )
      {
        if( mujoeI2C_write( (0x48<<1), 1, &mspfgRegAddr, STOP_CMD ) )
        {
          // Clear RX buffer
          VOID memset( i2cRxData, 0, 20 );
          if( mujoeI2C_read( (0x48<<1), 1, i2cRxData ) )
             muJoeGenProfile_writeMailbox( i2cRxData, 20 );
          else
            rspVal = MUJOE_RSP_FAILURE;
        }
        else
         rspVal = MUJOE_RSP_FAILURE;
      }
      break;
    }
    // Unsupported Command ID for this Command Group
    default:
      rspVal = MUJOE_RSP_INV_CMD_ID;
      break;
  }
  
  // Restore I2C SCL frequency
  mujoeI2C_setSclFreq( i2cClockCtxSave );
  
  return rspVal;
  
} // cmdGroup_mspDbgGrp

static uint16 cmdGroup_datGrp( uint8 cmd_id )
{
  uint16 rspVal = MUJOE_RSP_SUCCESS;
  
  switch(cmd_id)
  {
    case MUJOE_GRP_DAT_ID_STASYNCBULK:
    {
      getAsyncSamplePeriod(&(mujoeBrdSettings.asyncBulkSampPeriod));
      osal_set_event( muJoeGenMgr.asyncBulkCb.tskId, muJoeGenMgr.asyncBulkCb.evtFlg );
      break;
    }
    case MUJOE_GRP_DAT_ID_STASENSORACQ:
      osal_set_event( sensorMgrTask_getTaskId(), SENSORMGR_DATA_COLLECTOR_EVT );
      break;
    // Unsupported Command ID for this Command Group
    default:
      rspVal = MUJOE_RSP_INV_CMD_ID;
      break;
  }
  
  return rspVal;
} // cmdGroup_datGrp

static bStatus_t getAsyncSamplePeriod( uint32 *pSampPeriod )
{
  bStatus_t bStatus = SUCCESS;
  uint8 mailBoxBuff[20];
  bStatus =  muJoeGenProfile_readMailbox( mailBoxBuff, sizeof(mailBoxBuff) );
  
  if( bStatus == SUCCESS )
  {
     
     *pSampPeriod = (uint32)(( ( (uint32)(mailBoxBuff[0]) ) << 24 ) +
                   ( ( (uint32)(mailBoxBuff[1]) ) << 16 ) +
                   ( ( (uint32)(mailBoxBuff[2]) ) << 8 ) +
                    mailBoxBuff[3]);
     
     //*pSampPeriod =  tSamp;
  }
  return bStatus;
} // getAsyncSamplePeriod

static bStatus_t getMailboxByteField( uint8 *pBuff, uint8 buffSize, uint8 stIndex, uint8 spIndex )
{
  bStatus_t bStatus = SUCCESS;
  
  // Start OR Stop index is/are out of bounds, abort
  if( stIndex >= 20 || spIndex >= 20 )
    return FAILURE;
  
  uint8 numBytes = 0;
  // Compute number of bytes to read from Mailbox Characteristic
  if( stIndex < spIndex )
    numBytes = spIndex - stIndex + 1;
  else if( stIndex == stIndex )
    numBytes = 1;
  // Stop index is less than start index, abort
  else
    return FAILURE;
  
  // Output buffer size insufficient, abort
  if( buffSize < numBytes )
    return FAILURE;
  
  // Read Mailbox Characteristic
  uint8 mailBoxBuff[20] = {0};
  bStatus = muJoeGenProfile_readMailbox( mailBoxBuff, 20 );
  
  if( bStatus == SUCCESS )
  {
    // Copy data into output buffer
    for( uint8 i = stIndex; i <= spIndex; i++ )
    {
      pBuff[i - stIndex] = mailBoxBuff[i];
    }
  }
  return bStatus;
  
} // getMailboxByteField

static bStatus_t getI2cNumRxBytes( uint8 *pNumBytes )
{
  bStatus_t bStatus = SUCCESS;
  uint8 mailBoxBuff[20] = {0};
  bStatus = muJoeGenProfile_readMailbox( mailBoxBuff, 20 );
  
  if( bStatus == SUCCESS )
  {
     // Parse out the number of bytes to RX
     *pNumBytes = mailBoxBuff[0];
     // Clamp number of bytes to RX to a max of 20
     if( *pNumBytes > 20 ){*pNumBytes = 20;}
     muJoeGenProfile_clearMailbox();
  }
  return bStatus;
  
} // getI2cNumRxBytes

static bStatus_t getI2cWritePayload( uint8 *pTxData, uint8 *pNumBytes )
{
  bStatus_t bStatus = SUCCESS;
  uint8 mailBoxBuff[20] = {0};
  bStatus =  muJoeGenProfile_readMailbox( mailBoxBuff, 20 );
  
  if( bStatus == SUCCESS )
  {
     // Parse out the number of bytes to TX, and TX payload
     *pNumBytes = mailBoxBuff[0];
     VOID memcpy( pTxData, &mailBoxBuff[1], *pNumBytes );
     muJoeGenProfile_clearMailbox();
  }
  return bStatus;
} // getI2cWritePayload
