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

static bool configureGPIOPin( gpioConfig_t gpioConfig );
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
  
  // PS_HOLD
  {
     .gpio_pin = GPIOPIN_P1_3,
     .output = TRUE,
     .disablePUPDRes = TRUE, 
     .setPDRes = TRUE,
     .initState = TRUE,   
  },
};

#endif // #if defined( ASSET_TAG_BLE )

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
    if( !configureGPIOPin( gpioConfigTbl[i]) )   // If pin config failed, return failure immediately
      return FALSE;
  }
  
  return TRUE;
} // muJoeGPIO_configureGPIOs

static bool configureGPIOPin( gpioConfig_t gpioConfig )
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


