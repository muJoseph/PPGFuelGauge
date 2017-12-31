////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfileManager.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEGENERICPROFILEMANAGER_H
#define MUJOEGENERICPROFILEMANAGER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "mujoeGenericProfile.h"
#include "mujoeToolBox.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define GENPROFMGR_DEFAULT_RSPDELAY              100 // ms

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

// Supported Command Groups
typedef enum
{
  CMDGRPID_SYS = 0x00,
  CMDGRPID_DAT,
  CMDGRPID_MSPDBG,
  NUM_CMD_GROUPIDS
    
}cmdGrpIds_t;

// System Group Command IDs
typedef enum
{
  SYS_CMDID_PWRDWN = 0x00,
  SYS_NUMID                       // Number of Command IDs within System Command Group
    
}sysGrp_cmdIds_t;

// Data Group Command IDs
typedef enum
{
  DAT_CMDID_STASYNCBULK = 0x00,    // Start data collection and post to Async Bulk
  DAT_CMDID_STARTDATACOLLECT,      // Start sensor data collection and post via Async Bulk Characteristic
  DAT_CMDID_STOPDATACOLLECT,       // Stop all sensor data collection
  DAT_NUMID                        // Number of Command IDs within Data Command Group
  
}datGrp_cmdIds_t;

typedef enum
{
  MSPDBG_CMDID_I2CWRITE = 0x00,    // Write 'n' number of registers to MSPFuelGauge 
  MSPDBG_CMDID_I2CREAD,            // Read 'n' number of registers from MSPFuelGauge
  MSPDBG_CMDID_I2CREADREG,         // Read a register from MSPFuelGauge
  MSPDBG_NUMID                     // Number of Command IDs within MSP Debug Command Group     
  
}mspdbgGrp_cmdIds_t;

// Solicited Response Codes
typedef enum
{
  RSPCODE_RESERVED                  = 0x0000,                // Unused Response code
  RSPCODE_SUCCESS                   = 0x0001,                // Command successful
  RSPCODE_FAILURE                   = 0xEFFF,                // Command failed
  RSPCODE_INV_CMDGRP                = 0x0002,                // Invalid Command Group
  RSPCODE_INV_CMDID                 = 0x0003,                // Invalid Command ID
  RSPCODE_CMD_TIMEOUT               = 0x0004,                // Command Timed out (therefore failed)

}rspCodeSol_t;

// Command ID Callback Function Pointer
typedef rspCodeSol_t (*cmdIdCallBack_t)( void );

// Generic Profile Manager Driver Configuration Struct
typedef struct genProfileMgrCfg_def
{
  osalEvt_t             rxCmdEvt;           // OSAL Event assigned to muJoeGenManager_rxCmdHandler()   
  osalEvt_t             txRspEvt;           // OSAL Event assigned to muJoeGenManager_txRspHandler()
  
} genProfileMgrCfg_t;

////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool muJoeGenManager_initDriver( genProfileMgrCfg_t cfg );
bool muJoeGenManager_registerCmdIdHandlerTbl( cmdGrpIds_t cmdGrpId, cmdIdCallBack_t *pCmdIdTbl, uint8 cmdIdTblSize );
bStatus_t muJoeGenManager_readMailbox( uint8 numBytes, uint8 *pBuff );
bStatus_t muJoeGenManager_writeMailbox( uint8 numBytes, uint8 *pBuff );
bStatus_t muJoeGenManager_rxCmdHandler( void );
bStatus_t muJoeGenManager_txRspHandler( void );

#endif // #ifndef MUJOEGENERICPROFILEMANAGER_H