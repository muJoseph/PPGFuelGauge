////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeBoardSpecificDrivers.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEBOARDSPECIFICDRIVERS_H
#define MUJOEBOARDSPECIFICDRIVERS_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "mujoeBoardConfig.h"
#include "mujoeGPIO.h"
#include "mujoeI2C.h"
#include "mujoeADC.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////   

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool mujoeBSD_initBoard( void );

#endif // MUJOEBOARDSPECIFICDRIVERS_H