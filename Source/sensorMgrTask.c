////////////////////////////////////////////////////////////////////////////////
// @filename: sensorMgrTask.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * INCLUDES
 */

#include "sensorMgrTask.h"

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// EXTERN VAR
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// EXTERN FNC
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

static uint8                sensorMgrTask_TaskID;                               // Task ID for internal task/event processing

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void sensorMgrTask_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void sensorMgrTask_ProcessGATTMsg( gattMsgEvent_t *pMsg );
static bool sensorMgrTask_initSensors( void );

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
    //while( !stat );     // Trap MCU if failure
    return (events ^ SENSORMGR_INIT_SENSORS_EVT);
  }

  // Discard unknown events
  return 0;
} // sensorMgrTask_ProcessEvent


////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static bool sensorMgrTask_initSensors( void )
{
  
  if( !MS560702_initHardware() )
    return FALSE;
  
  
  return TRUE;
  
} // sensorMgrTask_initSensors

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
