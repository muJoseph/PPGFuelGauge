////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfileManager.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "muJoeGenericProfileManager.h"

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS 
////////////////////////////////////////////////////////////////////////////////

// Command Group Handler Function Pointer
//typedef rspCodeSol_t (*cmdGrpHdlr_t)( uint8 cmd_id );

typedef struct cmdGrp_def
{
    uint8               numCmdIds;
    cmdIdCallBack_t     *pCmdIdCallBackTbl;
      
}cmdGrp_t;

// Generic Profile Manager Variable Structure
typedef struct genProfileMgr_def
{
  rspCodeSol_t          rspCodeSolBuff;     // Solicited Response Buffer
  osalEvt_t             rxCmdEvt;           // OSAL Event assigned to muJoeGenManager_cmdWriteHandler()   
  osalEvt_t             txRspEvt;           // OSAL Event assigned to muJoeGenManager_txRspHandler()
  uint8                 numCmdGrpIds;       // Number of Command Group IDs
  cmdGrp_t              *pCmdGrpTbl;        // Pointer to Command Group Table    
  
} genProfileMgr_t;

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTION PROTOS 
////////////////////////////////////////////////////////////////////////////////

static void scheduleResponse( void );

////////////////////////////////////////////////////////////////////////////////
// LOCAL VARS 
////////////////////////////////////////////////////////////////////////////////

// Command Group Table
static cmdGrp_t         cmdGrTbl[NUM_CMD_GROUPIDS];

static genProfileMgr_t  genProfileMgr = 
{
   .numCmdGrpIds = NUM_CMD_GROUPIDS,
   .pCmdGrpTbl = cmdGrTbl,
   
}; // genProfileMgr

////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

bool muJoeGenManager_initDriver( genProfileMgrCfg_t cfg )
{
  genProfileMgr.rspCodeSolBuff = RSPCODE_RESERVED;
  genProfileMgr.rxCmdEvt = cfg.rxCmdEvt;
  genProfileMgr.txRspEvt = cfg.txRspEvt;
  genProfileMgr.txRspEvt.delay = 0;
  
  return TRUE;
  
} // muJoeGenManager_initDriver

bool muJoeGenManager_registerCmdIdHandlerTbl( cmdGrpIds_t cmdGrpId, cmdIdCallBack_t *pCmdIdTbl, uint8 cmdIdTblSize )
{
  if( cmdGrpId < NUM_CMD_GROUPIDS )
  {
    genProfileMgr.pCmdGrpTbl[cmdGrpId].numCmdIds = cmdIdTblSize;
    genProfileMgr.pCmdGrpTbl[cmdGrpId].pCmdIdCallBackTbl = pCmdIdTbl;
    return TRUE;
  }
  else
    return FALSE;
} // muJoeGenManager_registerCmdIdHandlerTbl

bStatus_t muJoeGenManager_rxCmdHandler( void )
{
  uint16 cmd = 0;
  uint8 cmd_grp_id = 0, cmd_id = 0;
  
  // Read Command Characteristic
  if( muJoeGenProfile_readCommand( &cmd ) != SUCCESS )      
    return FAILURE;
  
  // Seperate Command Group ID and Command ID
  cmd_grp_id = HI_UINT16(cmd);
  cmd_id = LO_UINT16(cmd);
  
  // Call appropriate Command ID Callback function
  if( cmd_grp_id < NUM_CMD_GROUPIDS )
  {
    if( cmd_id < genProfileMgr.pCmdGrpTbl[cmd_grp_id].numCmdIds )
    {
      if( genProfileMgr.pCmdGrpTbl[cmd_grp_id].pCmdIdCallBackTbl[cmd_id] != NULL )
        genProfileMgr.rspCodeSolBuff = genProfileMgr.pCmdGrpTbl[cmd_grp_id].pCmdIdCallBackTbl[cmd_id]();
    }
    else
      genProfileMgr.rspCodeSolBuff = RSPCODE_INV_CMDID;
  }
  else
    genProfileMgr.rspCodeSolBuff = RSPCODE_INV_CMDGRP;
  
  // Schedule an OSAL event to TX the Response code to host for RX'd
  // Command
  scheduleResponse();
  
  return SUCCESS;
  
} // muJoeGenManager_rxCmdHandler

bStatus_t muJoeGenManager_txRspHandler( void )
{
  if( muJoeGenProfile_writeResponse( genProfileMgr.rspCodeSolBuff ) == SUCCESS )
  {
    genProfileMgr.rspCodeSolBuff = RSPCODE_RESERVED;
    return SUCCESS;
  }
  else
    return FAILURE;
  
} // muJoeGenManager_txRspHandler

bStatus_t muJoeGenManager_readMailbox( uint8 numBytes, uint8 *pBuff )
{
  uint8 mailBoxBuff[MUJOEGENERICPROFILE_MBOX_LEN] = {0};
  
  if( muJoeGenProfile_readMailbox( mailBoxBuff, sizeof( mailBoxBuff ) ) == SUCCESS )
  {
    if( numBytes <= MUJOEGENERICPROFILE_MBOX_LEN )
    {
      VOID memcpy( pBuff, mailBoxBuff, numBytes );
      muJoeGenProfile_clearMailbox();
      return SUCCESS;
    }
    else
      return FAILURE;
  }
  else
    return FAILURE;
  
} // muJoeGenManager_readMailbox

bStatus_t muJoeGenManager_writeMailbox( uint8 numBytes, uint8 *pBuff )
{
  muJoeGenProfile_clearMailbox();
  return muJoeGenProfile_writeMailbox( pBuff, numBytes );
    
} // muJoeGenManager_writeMailbox

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Schedules an OSAL event that will issue a Response Notification for the RX'd
// Command
static void scheduleResponse( void )
{
   osal_start_timerEx( genProfileMgr.txRspEvt.taskId,
                       genProfileMgr.txRspEvt.event,
                       genProfileMgr.txRspEvt.delay ? genProfileMgr.txRspEvt.delay : GENPROFMGR_DEFAULT_RSPDELAY );
} // scheduleResponse

/*
static rspCodeSol_t cmdGrpHdlr_sys( uint8 cmd_id )
{
  rspCodeSol_t rspCodeSol = RSPCODE_FAILURE;
  
  switch( (sysGrp_cmdIds_t)cmd_id )
  { 
    case SYS_CMDID_PWRDWN:
      // Call fnc that powers down board
      rspCodeSol = RSPCODE_SUCCESS;
      break;
    // Unsupported Command ID for this Command Group
    default:
      rspCodeSol = RSPCODE_INV_CMDID;
      break;
  }
  
  return rspCodeSol;
  
} // cmdGrpHdlr_sys

static rspCodeSol_t cmdGrpHdlr_dat( uint8 cmd_id )
{
  rspCodeSol_t rspCodeSol = RSPCODE_FAILURE;
  
  switch( cmd_id )
  {
    case 0:
      break;
    case 1:
      break;
    default:
      break;
  }
  
  return rspCodeSol;
  
} // cmdGrpHdlr_dat

static rspCodeSol_t cmdGrpHdlr_mspdbg( uint8 cmd_id )
{
  rspCodeSol_t rspCodeSol = RSPCODE_FAILURE;
  
  return rspCodeSol;
  
} // cmdGrpHdlr_mspdbg
*/