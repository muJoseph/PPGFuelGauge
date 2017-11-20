////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfileMgr.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEGENERICPROFILEMGR_H
#define MUJOEGENERICPROFILEMGR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "muJoeGenericProfile.h"
#include "mujoeBoardSettings.h"
#include "OSAL_Timers.h"
#include "OSAL.h"

// BEGIN FOR DEBUG
#include "mujoeI2C.h"
// END FOR DEBUG

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

// Default Response Delay (i.e. delay after completion of Command handling and Response fireback)
#define MUJOE_DEFAULT_RSPDELAY              100 // ms

// Command Groups
#define MUJOE_CMD_GRP_SYS                   0x01
#define MUJOE_CMD_GRP_DAT                   0x02
#define MUJOE_CMD_GRP_MSPDBG                0x03    

// Command IDs for Command Group "System"
#define MUJOE_GRP_SYS_ID_PWRDWN             0x01

// Command IDs for Command Group "Data"
#define MUJOE_GRP_DAT_ID_STASYNCBULK        0x01    // Start data collection and post to Async Bulk

// Command IDs for Command Group "MSP Debug"
#define MUJOE_GRP_MSPDBG_ID_I2CWRITE        0x01
#define MUJOE_GRP_MSPDBG_ID_I2CREAD         0x02           

// Solicited Response Codes
#define MUJOE_RSP_SUCCESS                   0x0001      // Command successful
#define MUJOE_RSP_FAILURE                   0xEFFF      // Command failure
#define MUJOE_RSP_INV_CMD_GRP               0x0002      // Invalid Command Group
#define MUJOE_RSP_INV_CMD_ID                0x0003      // Invalid Command ID
#define MUJOE_RSP_CMD_TIMEOUT               0x0004      // Command timed out

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef struct eventCallback_def
{
  uint8            tskId;
  uint16           evtFlg;
  
} eventCallback_t;

typedef struct muJoeGenMgr_rspHdlrCb_def
{
  uint16           *pRspBuff;    // Pointer to Response buffer
  eventCallback_t   rspHdlrCb;   // OSAL event callback for muJoe Generic Response fireback       

}muJoeGenMgr_rspHdlrCb_t;

typedef struct muJoeGenMgr_def
{
  muJoeGenMgr_rspHdlrCb_t        muJoeGenMgr_rspHdlrCb; // muJoe Generic Profile Response Hanlder Callback          
  eventCallback_t                asyncBulkCb;           // OSAL event callback for Async Bulk data collection
  
}muJoeGenMgr_t;

////////////////////////////////////////////////////////////////////////////////
// PROTOS
////////////////////////////////////////////////////////////////////////////////

void muJoeGenMgr_initDriver( muJoeGenMgr_t s );
bStatus_t muJoeGenMgr_cmdWriteHandler( void );

#endif