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

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef struct evtCallback_def
{   
    uint16         evt;
    uint32         delay;
    
}evtCallback_t;

typedef struct sensorDatColl_def
{
  bool                  forceStateChange;
  bool                  nextSensor;
  uint8                 currSensor;
  uint8                 numSensors;
  uint8                 sensorState;
  uint8                 sensorFlags;
  evtCallback_t         evtCb;

}sensorDatColl_t, *p_sensorDatColl_t;

//typedef void (*sensorDatCollFncTbl_t)( p_sensorDatColl_t );   // DEFAULT
typedef void (*sensorDatCollFncTbl_t)( uint8 * );

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void sensorMgrTask_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void sensorMgrTask_ProcessGATTMsg( gattMsgEvent_t *pMsg );
static bool sensorMgrTask_initSensors( void );

//static void MS560702_dataCollector( p_sensorDatColl_t sdc );
static void MS560702_dataCollector( uint8 *pFlgs );
static void MMA8453_dataCollector( p_sensorDatColl_t sdc );
//static void MSPFuelGauge_dataCollector( p_sensorDatColl_t sdc );
static void MSPFuelGauge_dataCollector( uint8 *pFlgs );


static void sensorMgrTask_dataCollector( void );
static bool sensorMgrTask_SendOSALMsg( uint8 destTaskID, sensorMgrTask_msg_t msg );

// sensorMgrTask State Management functions
static uint8 sensorMgrTask_getSensorState( void );
static void sensorMgrTask_goToNextSensor( void );
static void sensorMgrTask_goToSensorState( uint8 sensorState, uint32 delay );
static void sensorMgrTask_setSensorState( uint8 sensorState );

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static uint8                       sensorMgrTask_TaskID;                        // Task ID for internal task/event processing

static boardSensorData_t           brdSensorDat;                                // Sensor Data for this board

static sensorDatColl_t             sensorDatColl = 
{
  .forceStateChange = FALSE,
  .nextSensor = FALSE,
  .currSensor = 0,
  .numSensors = SENSORMGR_MAX_NUM_SENSORS,
};

// Sensor Data Collection Function Table
static const sensorDatCollFncTbl_t       sensorDatCollFncTbl[SENSORMGR_MAX_NUM_SENSORS] = 
{
  MS560702_dataCollector,
  MSPFuelGauge_dataCollector,
  //MMA8453_dataCollector,
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

// Returns the current Sensor Data structure contents
boardSensorData_t sensorMgrTask_getSensorData( void )
{
  return brdSensorDat;
  
} // sensorMgrTask_getSensorData

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static uint8 sensorMgrTask_getSensorState( void )
{
  return sensorDatColl.sensorState;
  
} // sensorMgrTask_getSensorState

// Force a transition to the next sensor within the sensor manager queue (i.e. current
// sensor data acquistion cycle has completed )
static void sensorMgrTask_goToNextSensor( void )
{
  sensorDatColl.nextSensor = TRUE;
  
} // sensorMgrTask_goToNextSensor

// Go to next state
// If delay = 0, next state is called immediately
// If delay != 0, next state is called after a delay of length "delay", where delay is millisec
static void sensorMgrTask_goToSensorState( uint8 sensorState, uint32 delay )
{
  sensorDatColl.sensorState = sensorState;
  sensorDatColl.forceStateChange = TRUE;
  sensorDatColl.evtCb.delay = delay;
  
} // sensorMgrTask_goToSensorState

// Sets the sensor state, but DOES not transition to it.
// To be used when state transition is to be triggered by an external
// event such as an interrupt.
static void sensorMgrTask_setSensorState( uint8 sensorState )
{
  sensorDatColl.sensorState = sensorState;
  
} // sensorMgrTask_setSensorState

static void sensorMgrTask_dataCollector( void )
{
  // Call the current sensor within the queu data collection fnc
  if( sensorDatColl.currSensor < sensorDatColl.numSensors )
    sensorDatCollFncTbl[sensorDatColl.currSensor]( &(sensorDatColl.sensorFlags) );
  
  // Manage sensor to sensor transition
  if( sensorDatColl.nextSensor )
  {
    // Inc sensor index to next sensor
    sensorDatColl.currSensor++;
    // If end of sensor queue reached, restart
    if( sensorDatColl.currSensor == sensorDatColl.numSensors )  
      sensorDatColl.currSensor = 0;
    
    // Reset sensor specific state machine var
    sensorDatColl.sensorState = 0;
    sensorDatColl.sensorFlags = 0;
    VOID memset( &(sensorDatColl.evtCb), 0, sizeof( evtCallback_t ) );
    
    // Force a state change with a delay equal to that of the sample period
    sensorDatColl.forceStateChange = TRUE;
    sensorDatColl.evtCb.delay = 1000;   // TODO: Update this to reflect the actual sample period
    
    sensorDatColl.nextSensor = FALSE;
  }
  
  // Manage Sensor State Transition
  if( sensorDatColl.forceStateChange )
  {
    if( sensorDatColl.evtCb.delay )
    {
      osal_start_timerEx( sensorMgrTask_TaskID, 
                          SENSORMGR_DATA_COLLECTOR_EVT, 
                          sensorDatColl.evtCb.delay );
      sensorDatColl.evtCb.delay = 0;
    }
    else
      osal_set_event( sensorMgrTask_TaskID, SENSORMGR_DATA_COLLECTOR_EVT );
    
    sensorDatColl.forceStateChange = FALSE;
  }
    
} // sensorMgrTask_dataCollector

//static void MS560702_dataCollector( p_sensorDatColl_t sdc )
static void MS560702_dataCollector( uint8 *pFlgs )
{
   //switch( sdc->sensorState )
   switch( sensorMgrTask_getSensorState() )
   {
     // Trigger Pressure Conversion
     case 0:
       if( *pFlgs & 0x01 )
         MS560702_trigTemperatureConv( MS5_OSR_4096 );
       else
         MS560702_trigPressureConv( MS5_OSR_4096 );
       
       sensorMgrTask_goToSensorState(1,100);
       break;
     // Poll and fetch Conversion
     case 1:
     {
        uint32 adcConv;
        if( MS560702_readAdcConv( &adcConv ) )
        {
            if( *pFlgs & 0x01 )
            {
              brdSensorDat.ppgfg.barTempCode = adcConv;
              sensorMgrTask_goToNextSensor();
            }
            else
            {
              brdSensorDat.ppgfg.barPresCode = adcConv;
              *pFlgs |= 0x01;
              sensorMgrTask_goToSensorState( 0, 0 );
            }
        }
        else
          sensorMgrTask_goToSensorState( 1, 100 );
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

//static void MSPFuelGauge_dataCollector( p_sensorDatColl_t sdc )
static void MSPFuelGauge_dataCollector( uint8 *pFlgs )
{
   switch( sensorMgrTask_getSensorState() )
   {
     // Trigger Single-shot Measurement 
     case 0:
       if( mspfg_sendCommand( MSPFG_CMD_SINGLESHOT_DATA ) )
         sensorMgrTask_setSensorState(1);
       else                             // Command not sent, try again
         sensorMgrTask_goToSensorState( 0, 100 );
       
       break;
     // Read Conversion data
     case 1:
     {
        //mspfg_data_t     mspfg_data;
        if( mspfg_getAllData( &brdSensorDat.ppgfg.mspfg ) )
        {
          if( mspfg_clearIntFlag() )
            sensorMgrTask_goToNextSensor();
          else
            sensorMgrTask_goToSensorState( 1, 100 );
        }
        else                            // Unable to read data, try again...
          sensorMgrTask_goToSensorState( 1, 100 );
        break;
     }
     default:
        break;
   }
  
} // MSPFuelGauge_dataCollector

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
  // Init MSPFG IC
  mspfgCfg_t cfg = { .enHwInterrupt = TRUE, };
  if( !mspfg_initHardware( cfg ) )
    return FALSE;
  
  // Notify mainTask that onboard ICs have been initialized via an OSAL message
  sensorMgrTask_SendOSALMsg( mainTask_getTaskId(), SENSORMGR_HWINIT_DONE );     
  
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
