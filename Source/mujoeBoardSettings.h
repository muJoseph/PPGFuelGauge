////////////////////////////////////////////////////////////////////////////////
// @filename: mujoeBoardSettings.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEBOARDSETTINGS_H
#define MUJOEBOARDSETTINGS_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

// Async Bulk Data Collection Period (ms)
#define  MUJOE_ASYNCBULK_PERIOD_1S                      1000 //ms
#define  MUJOE_ASYNCBULK_PERIOD_DEFAULT                 MUJOE_ASYNCBULK_PERIOD_1S

////////////////////////////////////////////////////////////////////////////////
// TYPEDEF
////////////////////////////////////////////////////////////////////////////////

typedef struct mujoeBrdSettings_def
{
  uint32        asyncBulkSampPeriod;
  
}mujoeBrdSettings_t;

////////////////////////////////////////////////////////////////////////////////
// EXTERN VAR
////////////////////////////////////////////////////////////////////////////////

extern mujoeBrdSettings_t mujoeBrdSettings;

#endif