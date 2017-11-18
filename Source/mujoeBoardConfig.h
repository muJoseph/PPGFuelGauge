////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeBoardConfig.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEBOARDCONFIG_H
#define MUJOEBOARDCONFIG_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////   

// gpioPin_t.pinCfg bit map
#define PINCFG_OUTPUT                     0x01  // If set, pin is OUTPUT, INPUT otherwise
#define PINCFG_DISABLE_PUPDRES            0x02  // If set, internal PU/PD resistor disabled (only valid when PINCFG_OUTPUT = 0) 
#define PINCFG_ENABLE_INT                 0x08  // If set, GPIO interrupt is enabled, disabled otherwise
#define PINCFG_INIT_HIGH                  0x10  // If set, GPIO output is initialized HIGH, LOW otherwise ( only valid when PINCFG_OUTPUT = 0 )

// For readability...
#define PINCFG_INPUT                      0x00
#define PINCFG_ENABLE_PUPDRES             0x00
#define PINCFG_DISABLE_INT                0x00
#define PINCFG_INIT_LOW                   0x00

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

// Note: IDs must match the index of the respective "gpioPin_t" struct
// within the "gpioPinTable" array as defined in the mujoeBoardConfig.c
typedef enum 
{
  PINID_PS_HOLD = 0,
  PINID_STATUS_LED,
  PINID_CHG_LED,
  PINID_PB_INTn,
  PINID_PB_OUTn,
  PINID_ACCEL_INT1,
  PINID_ACCEL_INT2,
  PINID_MSP_INT,
  PINID_CHGn,
  PINID_VCC_LOn,
  PINID_NUMGPIOS,
  
}mujoegpio_pinid_t;

typedef void (*pinIntCb_t)( void );

typedef struct gpioPin_def
{
  uint8         port;
  uint8         pin;
  uint8         cfg;
  pinIntCb_t    IntCb;
  
}gpioPin_t;

////////////////////////////////////////////////////////////////////////////////
// EXTERN VARS
////////////////////////////////////////////////////////////////////////////////

extern gpioPin_t       gpioPinTable[PINID_NUMGPIOS];

#endif // MUJOEBOARDCONFIG_H