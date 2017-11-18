////////////////////////////////////////////////////////////////////////////////
// @filename: mainTask.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * INCLUDES
 */


#include "mainTask.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   5000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

/*********************************************************************
 * TYPEDEFS
 */

typedef enum
{
  STATLED_OFF = 0,
  STATLED_ON,
  STATLED_FASTBLINK,
  STATLED_SLOWBLINK,
  
}statLedState_t;

typedef struct mainTask_def
{
  uint32        lengthOfAdvert;
  
}mainTask_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static statLedState_t   statLedState = STATLED_OFF;

static mainTask_t       mainTask =
{
  .lengthOfAdvert = 30000,          // ms
};

static uint16           rspBuffer;         // TEST
static uint8            asyncBulkBuff[20]; // TEST

// HipScience characteristic notification control identifiers
static uint8                            mainTask_TaskID;             // Task ID for internal task/event processing
static gaprole_States_t                 gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
// Test comment
static uint8 scanRspData[] =
{
        // complete name
  13,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'P',
  'P',
  'G',
  'F',
  'u',
  'e',
  'l',
  'G',
  'a',
  'u',
  'g',
  'e',

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),

};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "PPGFuelGauge";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void mainTask_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void mainTask_ProcessGATTMsg( gattMsgEvent_t *pMsg );
static void mainTask_ProcessSensorMgrMsg( msg_t *msg );

static void peripheralStateNotificationCB( gaprole_States_t newState );
static void muJoeGenProfileChangeCB( uint8 paramID );
static void muJoeDataProfileReadCB( uint8 paramID );

static void mainTask_initMuJoeGenMgrDriver( void );
static void mainTask_beginAdvert( uint32 timeToAdvert );
static void mainTask_endAdvert( void );
static void mainTask_pbIntHdlr( void );
static void mainTask_brdLedMgr( void );
static void mainTask_setStatLEDState( statLedState_t newState );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t mainTask_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t mainTask_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

#if defined( MUJOE_GEN_PROFILE )

// muJoe Generic GATT Profile Callbacks
static muJoeGenProfileCBs_t mainTask_muJoeGenProfileCBs =
{
  muJoeGenProfileChangeCB    // Charactersitic value change callback
};

static muJoeDataProfileCBs_t mainTask_muJoeDataProfileCBs = 
{
  NULL,                         // Characteristic value change callback
  muJoeDataProfileReadCB        // Called when a characteristic is read by central
};

#else

// Simple GATT Profile Callbacks
static simpleProfileCBs_t mainTask_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};

#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

uint8 mainTask_getTaskId( void )
{
  return mainTask_TaskID;
  
} // mainTask_getTaskId

/*********************************************************************
 * @fn      mainTask_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void mainTask_Init( uint8 task_id )
{
  mainTask_TaskID = task_id;

  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    // For other hardware platforms, device starts advertising upon initialization
    //uint8 initial_advertising_enable = FALSE;
    uint8 initial_advertising_enable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  
#if defined( MUJOE_GEN_PROFILE ) 
  uint8 status = MuJoeGenericProfile_AddService(); // muJoe Generic GATT Profile
  while( status != SUCCESS );                      // Trap if adding service was unsuccessful
  status = MuJoeDataProfile_AddService();          // muJoe Data GATT Profile
  while( status != SUCCESS );                      // Trap if adding service was unsuccessful
#else
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
#endif
  

#if defined( MUJOE_GEN_PROFILE )
  
   // Init muJoe Generic Service Characteristic Values
   muJoeGenProfile_writeCommand( 0x0000 );
   muJoeGenProfile_writeResponse( 0x0000 );
   muJoeGenProfile_writeDeviceInfo( 0xAAAA, 0xBBBB );
   muJoeGenProfile_clearMailbox();
   
   // Init muJoe Data Service Characteristic Values
   muJoeDataProfile_clearAsyncBulk();
   muJoeDataProfile_clearSyncBulk();
   
#else
  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
  }
#endif

  bool initBoardstatus = mujoeBSD_initBoard();                   // Init board
  while( !initBoardstatus );                                     // Trap MCU if init failed
  // Assign the OSAL evt in which muJoeGPIO_interruptMgr is called
  muJoeGPIO_assignIntMgrOSALEvt( mainTask_TaskID, MAIN_GPIOINTMGR_EVT );
  bool stat = muJoeGPIO_registerIntCallback( PINID_PB_INTn, mainTask_pbIntHdlr );
  while( !stat );                                               // Trap MCU

#if defined( MUJOE_GEN_PROFILE )
  // Register callback with muJoeGenericProfile
  VOID muJoeGenProfile_RegisterAppCBs( &mainTask_muJoeGenProfileCBs );
  // Register callback with muJoeDataProfile
  VOID muJoeDataProfile_RegisterAppCBs( &mainTask_muJoeDataProfileCBs );
#else
  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs( &mainTask_SimpleProfileCBs );
#endif

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

#if defined ( DC_DC_P0_7 )

  // Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
  HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );

#endif // defined ( DC_DC_P0_7 )

  // Init App level drivers
  mainTask_initMuJoeGenMgrDriver();     // TEST
  
  // Setup a delayed profile startup
  osal_set_event( mainTask_TaskID, MAIN_START_DEVICE_EVT );

}

/*********************************************************************
 * @fn      mainTask_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 mainTask_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  // OSAL Message Received event
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    if ( (pMsg = osal_msg_receive( mainTask_TaskID )) != NULL )
    {
      mainTask_ProcessOSALMsg( (osal_event_hdr_t *)pMsg ); 
      VOID osal_msg_deallocate( pMsg );                  // Release the OSAL message
    }
    return (events ^ SYS_EVENT_MSG);                     // return unprocessed events
  }

  // Start Device Event 
  if ( events & MAIN_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &mainTask_PeripheralCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &mainTask_BondMgrCBs );

    // Set timer for Initializing onboard sensors
    osal_start_timerEx( sensorMgrTask_getTaskId(), SENSORMGR_INIT_SENSORS_EVT, 200 );
    
    return ( events ^ MAIN_START_DEVICE_EVT );
  }

  // Command Characteristic Write Handler Event 
  if( events & MAIN_CMD_WRITE_EVT )
  {
     muJoeGenMgr_cmdWriteHandler();
     return ( events ^ MAIN_CMD_WRITE_EVT );
  }
  
  // Response Characteristic Noti Handler Event 
  if( events & MAIN_RSP_NOTI_EVT )
  {
     muJoeGenProfile_writeResponse( rspBuffer );
     return ( events ^ MAIN_RSP_NOTI_EVT );
  }
  
  // Async Bulk Data Transfer Event 
  if( events & MAIN_ASYNCBULK_EVT )
  {
     // Restart timer
     if ( SBP_PERIODIC_EVT_PERIOD )
        osal_start_timerEx( mainTask_TaskID, 
                            MAIN_ASYNCBULK_EVT, 
                            mujoeBrdSettings.asyncBulkSampPeriod );
     
     // BEGIN TEST
     if( muJoeDataProfile_writeAsyncBulk( asyncBulkBuff, sizeof( asyncBulkBuff ) ) == SUCCESS )
       asyncBulkBuff[0]++;
     // END TEST
     
     return ( events ^ MAIN_ASYNCBULK_EVT );
  }
  
  // GPIO Interrupt Manager event
  if ( events & MAIN_GPIOINTMGR_EVT )
  {
    muJoeGPIO_interruptMgr();
    return (events ^ MAIN_GPIOINTMGR_EVT);
  }

  // Start BLE Advertisement event
  if ( events & MAIN_ADVBEGIN_EVT )
  {
    mainTask_beginAdvert( mainTask.lengthOfAdvert );
    return (events ^ MAIN_ADVBEGIN_EVT);
  }
  
  // Stop BLE Advertisement event
  if ( events & MAIN_ADVEND_EVT )
  {
     mainTask_endAdvert();
     return (events ^ MAIN_ADVEND_EVT);
  }
  
  // Board LED Manager event
  if ( events & MAIN_BRD_LEDMGR_EVT )
  {
    mainTask_brdLedMgr();
    return (events ^ MAIN_BRD_LEDMGR_EVT);
  }
  
  // Discard unknown events
  return 0;
}

// TODO: Abstract this further so multiple LED states and periods can
//       be tracked and managed simultaneously
static void mainTask_brdLedMgr( void )
{
  uint32 ledPeriod = 0;
  
  switch( statLedState )
  {
    case STATLED_OFF:
      muJoeGPIO_writePin( PINID_STATUS_LED, TRUE );
      break;
    case STATLED_ON:
      muJoeGPIO_writePin( PINID_STATUS_LED, FALSE );
      break;
    case STATLED_FASTBLINK:
      muJoeGPIO_togglePin( PINID_STATUS_LED );
      ledPeriod = 250;
      break;
    case STATLED_SLOWBLINK:
      muJoeGPIO_togglePin( PINID_STATUS_LED );
      ledPeriod = 500;
      break;
    default:
      break;
  }
  
  if( ledPeriod )
    osal_start_timerEx( mainTask_TaskID, MAIN_BRD_LEDMGR_EVT, ledPeriod );
  
}// mainTask_brdLedMgr

static void mainTask_setStatLEDState( statLedState_t newState )
{
  statLedState = newState;
  osal_stop_timerEx( mainTask_TaskID, MAIN_BRD_LEDMGR_EVT );
  osal_set_event( mainTask_TaskID, MAIN_BRD_LEDMGR_EVT );
  
} // mainTask_setStatLEDState

static void mainTask_pbIntHdlr( void )
{
  muJoeGPIO_togglePin( PINID_CHG_LED );
  osal_set_event( mainTask_TaskID, MAIN_ADVBEGIN_EVT ); 
  
} // mainTask_pbIntHdlr

static void mainTask_endAdvert( void )
{
   uint8 advertEnable = FALSE;
   if( GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertEnable ) != SUCCESS )
     osal_set_event( mainTask_TaskID, MAIN_ADVEND_EVT );  
   
} // mainTask_endAdvert

// timeToAdvert = How long to advertise in ms. Will advertise indefinately if zero
static void mainTask_beginAdvert( uint32 timeToAdvert )
{
   uint8 advertEnable = TRUE;
   if( GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertEnable ) == SUCCESS )
     if( timeToAdvert )
       osal_start_timerEx( mainTask_TaskID, MAIN_ADVEND_EVT, timeToAdvert );
   else
     osal_set_event( mainTask_TaskID, MAIN_ADVBEGIN_EVT );
   
} // mainTask_beginAdvert

static void mainTask_initMuJoeGenMgrDriver( void )
{
  muJoeGenMgr_t muJoeGenMgr;
  muJoeGenMgr.asyncBulkCb.evtFlg = MAIN_ASYNCBULK_EVT;
  muJoeGenMgr.asyncBulkCb.tskId = mainTask_getTaskId();
  muJoeGenMgr.muJoeGenMgr_rspHdlrCb.rspHdlrCb.tskId = mainTask_getTaskId();
  muJoeGenMgr.muJoeGenMgr_rspHdlrCb.rspHdlrCb.evtFlg = MAIN_RSP_NOTI_EVT;
  muJoeGenMgr.muJoeGenMgr_rspHdlrCb.pRspBuff = &rspBuffer;
  
  muJoeGenMgr_initDriver( muJoeGenMgr );
  
} // mainTask_initMuJoeGenMgrDriver

/*********************************************************************
 * @fn      mainTask_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void mainTask_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {    
    case GATT_MSG_EVENT:
      // Process GATT message
      mainTask_ProcessGATTMsg( (gattMsgEvent_t *)pMsg );
      break;
    // Process Messages from SensorManager Task
    case SENSORMGRTASK:
      mainTask_ProcessSensorMgrMsg( (msg_t *)pMsg );
      break;
    default:
      // do nothing
      break;
  }
} // mainTask_ProcessOSALMsg

static void mainTask_ProcessSensorMgrMsg( msg_t *msg )
{
  sensorMgrTask_msg_t sensorMgrTask_msg = msg->sensorMgrTask;
  
  switch( sensorMgrTask_msg )
  {
    case SENSORMGR_HWINIT_DONE:
      muJoeGPIO_writePin( PINID_CHG_LED, FALSE );
      enableP1PinInterrupt(0x20);       // TEST: Unmask P1.5 interrupt
      break;
    default:
      break;
  }
  
}//mainTask_ProcessSensorMgrMsg

/*********************************************************************
 * @fn      mainTask_ProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 */
static void mainTask_ProcessGATTMsg( gattMsgEvent_t *pMsg )
{  
  GATT_bm_free( &pMsg->msg, pMsg->method );
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
      }
      break;
    case GAPROLE_ADVERTISING:
      mainTask_setStatLEDState( STATLED_FASTBLINK ); 
      break;       
    case GAPROLE_CONNECTED:  
      mainTask_setStatLEDState( STATLED_ON );        
      break;
    case GAPROLE_CONNECTED_ADV:
      break;      
    case GAPROLE_WAITING:
      // If last GAPROLE state was Connected, then a disconnect occurred, therefore:
      // Suppress re-start of advertisement
      //if( gapProfileState == GAPROLE_CONNECTED )
      //  mainTask_endAdvert();    
      mainTask_setStatLEDState( STATLED_ON ); 
      break;
    case GAPROLE_WAITING_AFTER_TIMEOUT:
      break;
    case GAPROLE_ERROR:
      break;
    default:
      break;
  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif
}

/*********************************************************************
 * @fn      muJoeGenProfileChangeCB
 *
 * @brief   Callback from muJoeGenericProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void muJoeGenProfileChangeCB( uint8 paramID )
{
  switch( paramID )
  {
    case MUJOEGENERICPROFILE_COMMAND:
      osal_set_event( mainTask_TaskID, MAIN_CMD_WRITE_EVT );
      break;
    case MUJOEGENERICPROFILE_MAILBOX:
      break;
    default:
      // should not reach here!
      break;
  }
} // muJoeGenProfileChangeCB

/*********************************************************************
 * @fn      muJoeDataProfileReadCB
 *
 * @brief   Callback from muJoeDataProfile indicating a characteristic read (by central)
 *
 * @param   paramID - parameter ID of the characteristic that was read
 *
 * @return  none
 */
static void muJoeDataProfileReadCB( uint8 paramID )
{
  switch( paramID )
  {
    case MUJOEDATAPROFILE_SYNCBULK:
      // Do stuff here...
      break;
    default:
      // should not reach here!
      break;
  }
  
} // muJoeDataProfileReadCB

/*********************************************************************
*********************************************************************/
