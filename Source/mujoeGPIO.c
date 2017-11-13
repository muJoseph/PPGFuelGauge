////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGPIO.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "muJoeGPIO.h"

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

static bool muJoeGPIO_configureGPIOPin( gpioConfig_t gpioConfig );
static bool muJoeGPIO_configureGPIOs( gpioConfig_t *gpioConfigTbl, uint8 gpioTblSize );

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

#if defined( ASSET_TAG_BLE )

static gpioConfig_t gpioConfigTbl[MUJOE_NUMGPIOS] = 
{
  // GLED
  {
     .gpio_pin = GPIOPIN_P0_7,
     .output = TRUE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE,     
  },
  
  // VCC_HUM
  {
     .gpio_pin = GPIOPIN_P1_1,
     .output = TRUE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE,     
  },
  
  // P2.0 (NC)
  {
     .gpio_pin = GPIOPIN_P2_0,
     .output = FALSE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE,     
  },
};

#else   // PPGFG-REV0

static gpioConfig_t gpioConfigTbl[MUJOE_PINID_NUMGPIOS] = 
{
  // PS_HOLD
  {
     .gpio_pin = GPIOPIN_P1_3,
     .output = TRUE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE,
     .initState = TRUE,   
  },
  
  // STAT_LEDn (GLED)
  {
     .gpio_pin = GPIOPIN_P1_0,
     .output = TRUE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE, 
     .initState = FALSE,   // LED = ON (active low) 
  },
  
  // CHG_LEDn (RLED)
  {
     .gpio_pin = GPIOPIN_P2_0,
     .output = TRUE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE,
     .initState = TRUE,   // LED = OFF (active low) 
  },
  
};

#endif // #if defined( ASSET_TAG_BLE )

static muJoeGPIO_intMgr_t       muJoeGPIO_intMgr;

volatile static gpioIntSrc_t    gpioIntSrc = 
{
  .p0Ints = 0,
  .p1Ints = 0,
  .p2Ints = 0,
};

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Initializes the CC254x GPIOs in accordance with gpioConfigTbl array
bool muJoeGPIO_initGPIOS( void )
{
  return muJoeGPIO_configureGPIOs( gpioConfigTbl, MUJOE_PINID_NUMGPIOS );
  
} // muJoeGPIO_initGPIOS

bool muJoeGPIO_writePin( mujoe_gpioid_t gpioid, bool state )
{
  // Check if GPIO ID is valid
  if( gpioid < MUJOE_PINID_NUMGPIOS )
  {
     // Check if pin is configured as output
     if( gpioConfigTbl[gpioid].output )
     {
        // Port 0
        if( gpioConfigTbl[gpioid].gpio_pin < GPIOPIN_P1_0 )
        {
           // Set state
           if( state )
             P0 |= 0x01 << gpioConfigTbl[gpioid].gpio_pin;
           else
             P0 &= ~(0x01 << gpioConfigTbl[gpioid].gpio_pin);
        }
        // Port 1
        else if( gpioConfigTbl[gpioid].gpio_pin < GPIOPIN_P2_0 )
        {
           // Set state
           if( state )
             P1 |= 0x01 << ( gpioConfigTbl[gpioid].gpio_pin - 8 );
           else
             P1 &= ~(0x01 << ( gpioConfigTbl[gpioid].gpio_pin - 8 ) );
        }
        // Pin 2.0
        else
        {
           // Set state
           if( state )
             P2 |= 0x01;
           else
             P2 &= ~0x01;
        }
        
        return TRUE;
     }
     // Pin configured as input, abort
     else
       return FALSE;
  }
  else
    return FALSE;
} // muJoeGPIO_writePin

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Runs thru the gpioConfigTbl array defined with mujoeBoardConfig.c and configures
// the appropriate GPIO SFRs
static bool muJoeGPIO_configureGPIOs( gpioConfig_t *gpioConfigTbl, uint8 gpioTblSize )
{
  for( uint8 i = 0; i < gpioTblSize; i++ )
  {
    if( !muJoeGPIO_configureGPIOPin( gpioConfigTbl[i]) )   // If pin config failed, return failure immediately
      return FALSE;
  }
  
  return TRUE;
} // muJoeGPIO_configureGPIOs

static bool muJoeGPIO_configureGPIOPin( gpioConfig_t gpioConfig )
{
  bool retVal = TRUE;
  
  // Configure a Port 0 Pin
  if( gpioConfig.gpio_pin < GPIOPIN_P1_0 )
  {
      // Configure pin as output
      if( gpioConfig.output )
      {
        P0DIR |= 0x01 << gpioConfig.gpio_pin;
        
        // Configure initial output pin state
        if( gpioConfig.initState )
          P0 |= 0x01 << gpioConfig.gpio_pin;
        else
          P0 &= ~(0x01 << gpioConfig.gpio_pin);
      }
      // Configure pin as input
      else
      {
        P0DIR &= ~(0x01 << gpioConfig.gpio_pin);
        
        // Disable internal PU/PD resistor
        if( gpioConfig.disablePUPDRes )
            P0INP |= 0x01 << gpioConfig.gpio_pin;
        // Enable internal PU/PD resistor
        else
        {
            P0INP &= ~(0x01 << gpioConfig.gpio_pin);
            
            // Configure internal resistor as Pulldown
            if( gpioConfig.setPDRes )
              P2INP |= 0x20;
            // Configure internal resistor as Pullup
            else
              P2INP &= ~0x20;
        }
      }
  }
  // Configure a Port 1 Pin
  else if( gpioConfig.gpio_pin < GPIOPIN_P2_0 )
  {
       // Configure pin as output
      if( gpioConfig.output )
      {
        P1DIR |= 0x01 << ( gpioConfig.gpio_pin - 8 );
        
        // Configure initial output pin state
        if( gpioConfig.initState )
          P1 |= 0x01 << ( gpioConfig.gpio_pin - 8 );
        else
          P1 &= ~(0x01 << ( gpioConfig.gpio_pin - 8 ) );
      }
      // Configure pin as input
      else
      {
        P1DIR &= ~(0x01 << ( gpioConfig.gpio_pin - 8 ) );
        
        // Disable internal PU/PD resistor
        if( gpioConfig.disablePUPDRes )
            P1INP |= 0x01 << ( gpioConfig.gpio_pin - 8 );
        // Enable internal PU/PD resistor
        else
        {
            P1INP &= ~(0x01 << ( gpioConfig.gpio_pin - 8 ) );
            
            // Configure internal resistor as Pulldown
            if( gpioConfig.setPDRes )
              P2INP |= 0x40;
            // Configure internal resistor as Pullup
            else
              P2INP &= ~0x40;
        }
      }
  }
  // Configure Port 2.0 Pin
  else
  {
    // Configure P2.0 pin as output
    if( gpioConfig.output )
    {
      P2DIR |= 0x01;
      
      // Configure initial output pin state
      if( gpioConfig.initState )
        P2 |= 0x01;
      else
        P2 &= ~0x01;
    }
    // Configure P2.0 pin as input
    else
    {
      P2DIR &= ~0x01;
      
      // Disable internal PU/PD resistor
      if( gpioConfig.disablePUPDRes )
        P2INP |= 0x01;
      // Enable internal PU/PD resistor
      else
      {
        P2INP &= ~0x01;
        
        // Configure internal resistor as Pulldown
        if( gpioConfig.setPDRes )
          P2INP |= 0x80;
        // Configure internal resistor as Pullup
        else
          P2INP &= ~0x80;
      }
    }
  }
          
  return retVal;
}

gpioIntSrc_t muJoeGPIO_getIntSource( void )
{
  return gpioIntSrc;
  
}// muJoeGPIO_getIntSource

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
     gpioIntSrc.p1Ints |= 0x01;
     P1IFG = ~0x01;            // Clear P1.0 Source Interrupt Flag. Note register has R/W0 access , i.e. writing ones to bits does not do anything                          
  }
  
  // P1.1
  if( P1IFG & 0x02 )
  {
     gpioIntSrc.p1Ints |= 0x02;
     P1IFG = ~0x02; 
  }
  
  // P1.2
  if( P1IFG & 0x04 )
  {
     gpioIntSrc.p1Ints |= 0x04;
     P1IFG = ~0x04; 
  }
  
  // P1.3
  if( P1IFG & 0x08 )
  {
     gpioIntSrc.p1Ints |= 0x08;
     P1IFG = ~0x08; 
  }
  
  // P1.4
  if( P1IFG & 0x10 )
  {
     gpioIntSrc.p1Ints |= 0x10;
     P1IFG = ~0x10; 
  }
  
  // P1.5
  if( P1IFG & 0x20 )
  {
     gpioIntSrc.p1Ints |= 0x20;
     P1IFG = ~0x20; 
  }
  
  // P1.6
  if( P1IFG & 0x40 )
  {
     gpioIntSrc.p1Ints |= 0x40;
     P1IFG = ~0x40; 
  }
  
  // P1.7
  if( P1IFG & 0x80 )
  {
     gpioIntSrc.p1Ints |= 0x80;
     P1IFG = ~0x80; 
  }
  
  // Notify app of interrupt
  osal_set_event( muJoeGPIO_intMgr.osalCbEvt.taskId, muJoeGPIO_intMgr.osalCbEvt.evt );
  
  P1IF = 0;                                                                     // Clear Port 1 flag in Interrupt Flags 5 SFR
  //IRCON2 &= ~IRCON2_P1IF;                                                     // Clear Port 1 flag in Interrupt Flags 5 SFR
  HAL_EXIT_ISR();
  return;
}


