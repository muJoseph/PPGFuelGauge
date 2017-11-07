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

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

static void issueResponse( uint16 rspCode );
static uint16 cmdGroup_sysGrp( uint8 cmd_id );
static uint16 cmdGroup_datGrp( uint8 cmd_id );

static bStatus_t getSamplePeriod( uint32 *pSampPeriod );

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

static uint16 cmdGroup_datGrp( uint8 cmd_id )
{
  uint16 rspVal = MUJOE_RSP_SUCCESS;
  
  switch(cmd_id)
  {
    case MUJOE_GRP_DAT_ID_STASYNCBULK:
    {
      getSamplePeriod(&(mujoeBrdSettings.asyncBulkSampPeriod));
      osal_set_event( muJoeGenMgr.asyncBulkCb.tskId, muJoeGenMgr.asyncBulkCb.evtFlg );
      break;
    }
    // Unsupported Command ID for this Command Group
    default:
      rspVal = MUJOE_RSP_INV_CMD_ID;
      break;
  }
  
  return rspVal;
} // cmdGroup_datGrp

static bStatus_t getSamplePeriod( uint32 *pSampPeriod )
{
  bStatus_t bStatus = SUCCESS;
  uint8 mailBoxBuff[20];
  bStatus =  muJoeGenProfile_readMailbox( mailBoxBuff, sizeof(mailBoxBuff) );
  
  if( bStatus == SUCCESS )
  {
     
     uint32 tSamp = (uint32)(( ( (uint32)(mailBoxBuff[0]) ) << 24 ) +
                   ( ( (uint32)(mailBoxBuff[1]) ) << 16 ) +
                   ( ( (uint32)(mailBoxBuff[2]) ) << 8 ) +
                    mailBoxBuff[3]);
     
     *pSampPeriod =  tSamp;
  }
  return bStatus;
}
