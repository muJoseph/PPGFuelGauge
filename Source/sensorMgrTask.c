////////////////////////////////////////////////////////////////////////////////
// @filename: sensorMgrTask.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * INCLUDES
 */

#include "sensorMgrTask.h"

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

boardSensorData_t               brdSensorDat;

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef struct evtCallback_def
{    
    uint32         delay;
    
}evtCallback_t;

typedef struct sensorDatColl_def
{
  bool                  nextSensor;
  uint8                 currSensor;
  uint8                 numSensors;
  uint8                 sensorState;
  uint8                 sensorFlags;
  evtCallback_t         evtCb;

}sensorDatColl_t, *p_sensorDatColl_t;

typedef void (*sensorDatCollFncTbl_t)( p_sensorDatColl_t );

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void sensorMgrTask_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void sensorMgrTask_ProcessGATTMsg( gattMsgEvent_t *pMsg );
static bool sensorMgrTask_initSensors( void );

static void MS560702_dataCollector( p_sensorDatColl_t sdc );
static void MMA8453_dataCollector( p_sensorDatColl_t sdc );
static void sensorMgrTask_dataCollector( void );
static bool sensorMgrTask_SendOSALMsg( uint8 destTaskID, sensorMgrTask_msg_t msg );

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static uint8                       sensorMgrTask_TaskID;                               // Task ID for internal task/event processing

static sensorDatColl_t             sensorDatColl = 
{
  .nextSensor = FALSE,
  .currSensor = 0,
  .numSensors = SENSORMGR_MAX_NUM_SENSORS,
};

// Sensor Data Collection Function Table
sensorDatCollFncTbl_t       sensorDatCollFncTbl[SENSORMGR_MAX_NUM_SENSORS] = 
{
  MS560702_dataCollector,
  MMA8453_dataCollector
};

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

uint8 sensorMgrTask_getTaskId( void )
{
  return sensorMgrTask_TaskID;
  
} // sensorMgrTask_getTaskId

/*********************************************************************
 * @fn      sensorMgrTask_Init
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
void sensorMgrTask_Init( uint8 task_id )
{
  sensorMgrTask_TaskID = task_id;
  
  MS560702_initDriver(FALSE);   // Init BAR Drivers, CSB = GND
  bool stat = CAT24C512_initDriver( 64, FALSE, FALSE, FALSE );
  while( !stat );               // TRAP MCU if init failed
  stat = MMA8453Q_initDriver( 10, FALSE );
  while( !stat );               // TRAP MCU if init failed
  
  // BEGIN TEST
  uint8 breakVal;
  //stat = mspfg_ping();
  stat = mspfg_commTest();
  if( stat )
    breakVal = 100;
  else
    breakVal = 200;
  // END TEST

} // sensorMgrTask_Init

/*********************************************************************
 * @fn      sensorMgrTask_ProcessEvent
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
uint16 sensorMgrTask_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( sensorMgrTask_TaskID )) != NULL )
    {
      sensorMgrTask_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  
  // Init Sensors Event ////////////////////////////////////////////////////////
  if( events & SENSORMGR_INIT_SENSORS_EVT )
  {
    bool stat = sensorMgrTask_initSensors();
    while( !stat );     // Trap MCU if failure
    //osal_set_event( sensorMgrTask_TaskID, SENSORMGR_DATA_COLLECTOR_EVT );       // TEST
    return (events ^ SENSORMGR_INIT_SENSORS_EVT);
  }
  
  // Sensor Data Collector Event ///////////////////////////////////////////////
  if( events & SENSORMGR_DATA_COLLECTOR_EVT )
  {
    sensorMgrTask_dataCollector();
    return (events ^ SENSORMGR_DATA_COLLECTOR_EVT);
  }

  // Discard unknown events
  return 0;
} // sensorMgrTask_ProcessEvent


////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void sensorMgrTask_dataCollector( void )
{
  // Call the current sensor within the queu data collection fnc
  if( sensorDatColl.currSensor < sensorDatColl.numSensors )
    sensorDatCollFncTbl[sensorDatColl.currSensor](&sensorDatColl);
  
  if( sensorDatColl.nextSensor )
  {
    sensorDatColl.currSensor++;
    // If end of sensor queue reached, restart
    if( sensorDatColl.currSensor == sensorDatColl.numSensors )  
      sensorDatColl.currSensor = 0;
    
    // Reset sensor specific state machine var
    sensorDatColl.sensorState = 0;
    sensorDatColl.sensorFlags = 0;
    VOID memset( &(sensorDatColl.evtCb), 0, sizeof( evtCallback_t ) );
    
    sensorDatColl.nextSensor = FALSE;
  }
  
  // Schedule next event to continue data collection
  if( sensorDatColl.evtCb.delay )
  {
    osal_start_timerEx( sensorMgrTask_TaskID, 
                        SENSORMGR_DATA_COLLECTOR_EVT, 
                        sensorDatColl.evtCb.delay );
    sensorDatColl.evtCb.delay = 0;
  }
  else
    osal_set_event( sensorMgrTask_TaskID, SENSORMGR_DATA_COLLECTOR_EVT );
  
} // sensorMgrTask_dataCollector

static void MS560702_dataCollector( p_sensorDatColl_t sdc )
{
   switch( sdc->sensorState )
   {
     // Trigger Pressure Conversion
     case 0:
       if( sdc->sensorFlags & 0x01 )
         MS560702_trigTemperatureConv( MS5_OSR_4096 );
       else
         MS560702_trigPressureConv( MS5_OSR_4096 );
       sdc->evtCb.delay = 100;
       sdc->sensorState = 1;  
       break;
     // Poll and fetch Conversion
     case 1:
     {
        uint32 adcConv;
        if( MS560702_readAdcConv( &adcConv ) )
        {
            if( sdc->sensorFlags & 0x01 )
            {
              brdSensorDat.ppgfg.barTempCode = adcConv;
              sdc->nextSensor = TRUE;
            }
            else
            {
              brdSensorDat.ppgfg.barPresCode = adcConv;
              sdc->sensorFlags |= 0x01;
              sdc->sensorState = 0;
            }
        }
        else
        {
          sdc->sensorState = 1;
          sdc->evtCb.delay = 100;
        }
        break;
     }
     default:
        break;
   }
   
} // MS560702_dataCollector

static void MMA8453_dataCollector( p_sensorDatColl_t sdc )
{
  // Add data colleciton state machine
  
} // MMA8453_dataCollector

static bool sensorMgrTask_initSensors( void )
{
  // Init Barometer IC
  if( !MS560702_initHardware() )
    return FALSE;
  // Init Accelerometer IC
  if( !MMA845Q_initHardware() )
    return FALSE;
  // Init EEPROM IC
  if( !CAT24C512_initHardware() )
    return FALSE;
  
  // BEGIN TEST
  sensorMgrTask_SendOSALMsg( mainTask_getTaskId(), SENSORMGR_HWINIT_DONE );     // TEST
  
  return TRUE;
  
} // sensorMgrTask_initSensors

static bool sensorMgrTask_SendOSALMsg( uint8 destTaskID, sensorMgrTask_msg_t msg )
{
  taskMsgrMsg_t taskMsg;
  taskMsg.hdr.event = SENSORMGRTASK;
  taskMsg.msg.sensorMgrTask = msg;
  return mujoeTaskMsgr_sendMsg( mainTask_getTaskId(), taskMsg );
  
} // sensorMgrTask_SendOSALMsg

/*********************************************************************
 * @fn      sensorMgrTask_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void sensorMgrTask_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {     
    case GATT_MSG_EVENT:
      // Process GATT message
      sensorMgrTask_ProcessGATTMsg( (gattMsgEvent_t *)pMsg );
      break;
    default:
      // do nothing
      break;
  }
} // sensorMgrTask_ProcessOSALMsg

/*********************************************************************
 * @fn      sensorMgrTask_ProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 */
static void sensorMgrTask_ProcessGATTMsg( gattMsgEvent_t *pMsg )
{  
  GATT_bm_free( &pMsg->msg, pMsg->method );
} // sensorMgrTask_ProcessGATTMsg

/*********************************************************************
*********************************************************************/
