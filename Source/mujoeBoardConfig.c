////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeBoardConfig.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "mujoeBoardConfig.h"

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// LOCAL VARS
////////////////////////////////////////////////////////////////////////////////

gpioPin_t       gpioPinTable[PINID_NUMGPIOS] = 
{
  // PS_HOLD
  {
     .port = 0,
     .pin = 3,
     .cfg = PINCFG_OUTPUT + PINCFG_INIT_HIGH,
     .IntCb = NULL,
  },
  
  // STAT_LEDn (GLED)
  {
     .port = 1,
     .pin = 0,
     .cfg = PINCFG_OUTPUT + PINCFG_INIT_LOW,    // LED = ON (active low) 
     .IntCb = NULL,
  },
  
  // CHG_LEDn (RLED)
  {
     .port = 2,
     .pin = 0,
     .cfg = PINCFG_OUTPUT + PINCFG_INIT_HIGH,    // LED = OFF (active low) 
     .IntCb = NULL,
  },
  
  // PB_INTn
  {
     .port = 1,
     .pin = 5,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
  
  // PB_OUTn
  {
     .port = 1,
     .pin = 4,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
  
  // ACCEL_INT1
  {
     .port = 1,
     .pin = 2,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
  
  // ACCEL_INT2
  {
     .port = 0,
     .pin = 7,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
  
  // MSP_INT
  {
     .port = 0,
     .pin = 6,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
  
  // CHGn
  {
     .port = 1,
     .pin = 7,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
  
  // VCC_LOn
  {
     .port = 1,
     .pin = 6,
     .cfg = PINCFG_INPUT + PINCFG_ENABLE_PUPDRES,
     .IntCb = NULL,
  },
};
