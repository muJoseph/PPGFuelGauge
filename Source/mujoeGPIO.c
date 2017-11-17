////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGPIO.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "muJoeGPIO.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef struct
{
  gpioPin_t        *pGpioPinTbl;        // Pointer to GPIO cfg table
  osalEvt_t        intMgrEvt;           // OSAL event assigned to muJoeGPIO_interruptMgr    
}mueJoeGPIO_t;

typedef struct gpioIntSrc_def
{
  uint8         pxInts[3];      // Index 0 = Port 0, Index 1 = Port 1, Index 2 = Port 2
  
}gpioIntSrc_t;

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

static bool muJoeGPIO_pinWrite( gpioPin_t gpioPin, bool high );
static bool muJoeGPIO_cfgPin( gpioPin_t gpioPin );

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

// BEGIN TEST
volatile static gpioIntSrc_t    gpioIntSrc = 
{
  .pxInts = {0},
};
// END TEST

static mueJoeGPIO_t     mueJoeGPIO = 
{
  .pGpioPinTbl = NULL,       
  .intMgrEvt.taskId = 0,
  .intMgrEvt.event = 0,
};

//static gpioPin_t        *pGpioPinTbl = NULL;    // Local ptr to pin config table defined in mujoeBoardConfig.c

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void muJoeGPIO_assignIntMgrOSALEvt( uint8 taskId, uint16 event )
{
  mueJoeGPIO.intMgrEvt.taskId = taskId;
  mueJoeGPIO.intMgrEvt.event = event;
  
} // muJoeGPIO_assignIntMgrOSALEvt

bool muJoeGPIO_registerIntCallback( mujoegpio_pinid_t pinId, pinIntCb_t cb )
{
  // Driver uninitialized or pinId invalid, abort
  if( ( mueJoeGPIO.pGpioPinTbl == NULL ) || ( pinId >= PINID_NUMGPIOS ) )
     return FALSE;
  
  mueJoeGPIO.pGpioPinTbl[pinId].IntCb = cb;
  return TRUE;
  
} // muJoeGPIO_registerIntCallback

bool muJoeGPIO_writePin( mujoegpio_pinid_t pinId, bool high )
{
  // Driver uninitialized or pinId invalid, abort
  if( ( mueJoeGPIO.pGpioPinTbl == NULL ) || ( pinId >= PINID_NUMGPIOS ) )
     return FALSE;
  
  return muJoeGPIO_pinWrite(  mueJoeGPIO.pGpioPinTbl[pinId], high );

} // muJoeGPIO_writePin

bool muJoeGPIO_cfgInternalResistor( uint8 port, bool pullDown )
{
  // Check if port is valid
  if( port > 2 )
    return FALSE;
  
  // Configure as Pull-down resistor
  if( pullDown )
    P2INP |= ( 0x01 << ( 5 + port ) );
  // Configure as Pull-up resistor
  else
    P2INP &= ~( 0x01 << ( 5 + port ) );
  
  return TRUE;
  
} // muJoeGPIO_cfgInternalResistor

bool mujoeGPIO_initHardware( gpioPin_t *gpioPinTbl, uint8 numPins )
{
  // Store cfg table addr in local var
  mueJoeGPIO.pGpioPinTbl = gpioPinTbl;

  for( uint8 i = 0; i < numPins; i++ )
  {
    if( !muJoeGPIO_cfgPin( gpioPinTbl[i] ) )
      return FALSE;
  }
  
  return TRUE;
  
} // mujoeGPIO_initHardware

void muJoeGPIO_interruptMgr( void )
{
  // Go thru ea. pin of ea. port and call respective callback
  for( uint8 i = 0; i < 3; i++ )
  {
    for( uint8 j = 0; j < 8; j++ )
    {
      if( gpioIntSrc.pxInts[i] & ( 0x01 << j ) )
      {
          uint8 port = i;
          uint8 pin = j;
          // Go thru GPIO config array and find entry that matches port and pin of
          // interrupt source
          for( uint8 k = 0; k < PINID_NUMGPIOS; k++ )
          {
            if( ( mueJoeGPIO.pGpioPinTbl[k].port == port ) && ( mueJoeGPIO.pGpioPinTbl[k].pin == pin ) )
            {
                if( mueJoeGPIO.pGpioPinTbl[k].IntCb != NULL )
                {
                  // Call callback fnc
                  mueJoeGPIO.pGpioPinTbl[k].IntCb();
                  // Clear interrupt flag for respective port and pin
                  gpioIntSrc.pxInts[port] &= ~( 0x01 << pin );
                }
                break;
            }
          }
      }
    }  
  }
  
} // muJoeGPIO_interruptMgr

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static bool muJoeGPIO_pinWrite( gpioPin_t gpioPin, bool high )
{
  switch( gpioPin.port )
  {
    case 0:
      if( high )
        P0 |= (0x01 << gpioPin.pin);
      else
        P0 &= ~(0x01 << gpioPin.pin);
      break;
    case 1:
      if( high )
        P1 |= (0x01 << gpioPin.pin);
      else
        P1 &= ~(0x01 << gpioPin.pin);
      break;
    case 2:
      if( high )
        P2 |= (0x01 << gpioPin.pin);
      else
        P2 &= ~(0x01 << gpioPin.pin);
      break;
    default:
      return FALSE;
      break;
  }
  
  return TRUE;
} // muJoeGPIO_pinWrite

static bool muJoeGPIO_cfgPin( gpioPin_t gpioPin )
{
  // If port OR pin are invalid, abort
  if( ( gpioPin.port > 2 ) || ( gpioPin.pin > 7 ) )
    return FALSE;

  // Configure as OUTPUT
  if( gpioPin.cfg & PINCFG_OUTPUT )
  {
    // Set respective bit within the respective PxDIR SFR
    SFRIO( SFR_PXDIR_BASE_ADDR + gpioPin.port ) |= ( 0x01 << gpioPin.pin );

    // Initialize output state
    if( gpioPin.cfg & PINCFG_INIT_HIGH )
      muJoeGPIO_pinWrite( gpioPin, TRUE );
    else
      muJoeGPIO_pinWrite( gpioPin, FALSE );
  }
  // Configure as INPUT
  else
  {
    // Clear respective bit within the respective PxDIR SFR
    SFRIO( SFR_PXDIR_BASE_ADDR + gpioPin.port ) &= ~( 0x01 << gpioPin.pin );
    
    // Disable internal pull-up/pull-down resistor (3-state)
    if( gpioPin.cfg & PINCFG_DISABLE_PUPDRES )
    {
        // Port 0
        if( gpioPin.port == 0 )
          P0INP |= ( 0x01 << gpioPin.pin );
        // Port 1
        else if( gpioPin.port == 1 )
          P1INP |= ( 0x01 << gpioPin.pin );
        // Pin P2.0
        else
          P2INP |= 0x01;
    }
    // Enable internal pull-up/pull-down resistor
    else
    {
        // Port 0
        if( gpioPin.port == 0 )
          P0INP &= ~( 0x01 << gpioPin.pin ); 
        // Port 1
        else if( gpioPin.port == 1 )
          P1INP &= ~( 0x01 << gpioPin.pin );
        // Pin P2.0
        else
          P2INP &= ~0x01;
    }
  }
  
  return TRUE;
  
} // muJoeGPIO_cfgPin

////////////////////////////////////////////////////////////////////////////////
// INTERRUPT SERVICE ROUTINES
////////////////////////////////////////////////////////////////////////////////

// PORT 1 ISR //////////////////////////////////////////////////////////////////
HAL_ISR_FUNCTION( PORT1_ISR , P1INT_VECTOR )    // TEST
//HAL_ISR_FUNCTION( PORT1_ISR , PORT1_VECTOR )  // DEFAULT
{
  HAL_ENTER_ISR();
  
  // P1.0
  if( P1IFG & 0x01 )                        
  { 
     gpioIntSrc.pxInts[1] |= 0x01;
     P1IFG = ~0x01;            // Clear P1.0 Source Interrupt Flag. Note register has R/W0 access , i.e. writing ones to bits does not do anything                          
  }
  
  // P1.1
  if( P1IFG & 0x02 )
  {
     gpioIntSrc.pxInts[1] |= 0x02;
     P1IFG = ~0x02; 
  }
  
  // P1.2
  if( P1IFG & 0x04 )
  {
     gpioIntSrc.pxInts[1] |= 0x04;
     P1IFG = ~0x04; 
  }
  
  // P1.3
  if( P1IFG & 0x08 )
  {
     gpioIntSrc.pxInts[1] |= 0x08;
     P1IFG = ~0x08; 
  }
  
  // P1.4
  if( P1IFG & 0x10 )
  {
     gpioIntSrc.pxInts[1] |= 0x10;
     P1IFG = ~0x10; 
  }
  
  // P1.5
  if( P1IFG & 0x20 )
  {
     gpioIntSrc.pxInts[1] |= 0x20;
     P1IFG = ~0x20; 
  }
  
  // P1.6
  if( P1IFG & 0x40 )
  {
     gpioIntSrc.pxInts[1] |= 0x40;
     P1IFG = ~0x40; 
  }
  
  // P1.7
  if( P1IFG & 0x80 )
  {
     gpioIntSrc.pxInts[1] |= 0x80;
     P1IFG = ~0x80; 
  }
  
  // Notify app of interrupt
  if( mueJoeGPIO.intMgrEvt.taskId )
    osal_set_event( mueJoeGPIO.intMgrEvt.taskId, mueJoeGPIO.intMgrEvt.event );
  
  P1IF = 0;                                                                     // Clear Port 1 flag in Interrupt Flags 5 SFR
  //IRCON2 &= ~IRCON2_P1IF;                                                     // Clear Port 1 flag in Interrupt Flags 5 SFR
  HAL_EXIT_ISR();
  return;
}


