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

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

bool muJoeGPIO_configureGPIOs( gpioConfig_t *gpioConfigTbl, uint8 gpioTblSize )
{
  for( uint8 i = 0; i < gpioTblSize; i++ )
  {
    if( !configureGPIOPin( gpioConfigTbl[i]) )   // If pin config failed, return failure immediately
      return FALSE;
  }
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static bool configureGPIOPin( gpioConfig_t gpioConfig )
{
  bool retVal = TRUE;
  
  // Configure a Port 0 Pin
  if( gpioConfig.gpio_pin < GPIOPIN_P1_0 )
  {
      // Configure pin as output
      if( gpioConfig.output )
        P0DIR |= 0x01 << gpioConfig.gpio_pin;
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
        P1DIR |= 0x01 << ( gpioConfig.gpio_pin - 8 );
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
      P2DIR |= 0x01;
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


