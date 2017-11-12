////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGPIO.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEGPIO_H
#define MUJOEGPIO_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"
#include "iocc2541.h"
#include "mujoeBoardConfig.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS 
////////////////////////////////////////////////////////////////////////////////

// Note: IDs must match the index of the respective "gpioConfig_t" struct
// within the "gpioConfigTbl" array as defined in the mujoeGPIO.c
typedef enum 
{
  MUJOE_PINID_STATUS_LED =      0,
  MUJOE_PINID_CHG_LED,
  MUJOE_PINID_PS_HOLD,
  MUJOE_PINID_NUMGPIOS,
}mujoe_gpioid_t;

typedef enum
{
  GPIOPIN_P0_0 = 0,
  GPIOPIN_P0_1, // 1
  GPIOPIN_P0_2, // 2
  GPIOPIN_P0_3, // 3
  GPIOPIN_P0_4, // 4
  GPIOPIN_P0_5, // 5
  GPIOPIN_P0_6, // 6
  GPIOPIN_P0_7, // 7
  GPIOPIN_P1_0, // 8
  GPIOPIN_P1_1, // 9
  GPIOPIN_P1_2, // 10
  GPIOPIN_P1_3, // 11
  GPIOPIN_P1_4, // 12
  GPIOPIN_P1_5, // 13
  GPIOPIN_P1_6, // 14
  GPIOPIN_P1_7, // 15
  GPIOPIN_P2_0, // 16
  NUM_GPIOPINS  // 17
    
}gpio_pin_t;

typedef struct gpioConfig_def
{
  gpio_pin_t            gpio_pin;
  bool                  output;
  bool                  disablePUPDRes; // If TRUE, GPIO is in tri-state. If FALSE, internal PU/PD res is enabled
  bool                  setPDRes;       // If TRUE, then internal PU/PD is cfg'd as pull-down resistor. If FALSE, pull-up res. (Note: disablePUPDRes must be FALSE for this to take affect)        
  bool                  initState;      // If TRUE, pin is set HIGH, set LOW if FALSE. (NOTE: Only valid when "output" struct member is TRUE)
}gpioConfig_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS 
////////////////////////////////////////////////////////////////////////////////

bool muJoeGPIO_initGPIOS( void );
bool muJoeGPIO_writePin( mujoe_gpioid_t gpioid, bool state );

#endif