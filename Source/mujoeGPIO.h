////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGPIO.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEGPIO_H
#define MUJOEGPIO_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "OSAL.h"
#include "hal_mcu.h"
#include "mujoeBoardConfig.h"
#include "mujoeToolBox.h"
//#include "string.h"     // For memcpy

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

// GPIO ISR Vectors
//#define PORT0_VECTOR                    0x6B
//#define PORT1_VECTOR                    0x7B
//#define PORT2_VECTOR                    0x33

////////////////////////////////////////////////////////////////////////////////
// MACROS 
////////////////////////////////////////////////////////////////////////////////

#define triggerFallEdgeP0()             (PICTL|=0x01)           // Set Edge trigger for P0 interrupts
#define triggerRiseEdgeP0()             (PICTL&=~0x01)

#define triggerFallEdgeP1Low()          (PICTL|=0x02)           // Interrupt Edge Trigger cntrl for P1.3 to P1.0
#define triggerRiseEdgeP1Low()          (PICTL&=~0x02)

#define triggerFallEdgeP1High()         (PICTL|=0x04)           // Interrupt Edge Trigger control for P1.7 to P1.4
#define triggerRiseEdgeP1High()         (PICTL&=~0x04)

#define enablePort0Interrupts()         (IEN1|=0x20)            // Port 0 Interrupt enable/disable
#define disablePort0Interrupts()        (IEN1&=~0x20)

#define enablePort1Interrupts()         (IEN2|=0x10)            // Port 1  Interrupt enable/disable
#define disablePort1Interrupts()        (IEN2&=~0x10)

// Disable P0.X Pin Source interrupt
#define disableP0PinInterrupt(PIN){\
                              P0IFG=~(PIN);\
                              P0IEN&=~(PIN);\
                              P0IFG=~(PIN);\
                              }

// Enable P0.X Pin Source Interrupt
#define enableP0PinInterrupt(PIN){\
                              P0IFG=~(PIN);\
                              P0IEN|=(PIN);\
                              P0IFG=~(PIN);\
                              }


// Disable P1.X Pin Source Interrupt
#define disableP1PinInterrupt(PIN){\
                              P1IFG=~(PIN);\
                              P1IEN&=~(PIN);\
                              P1IFG=~(PIN);\
                              }

// Enable P1.X Pin Source Interrupt
#define enableP1PinInterrupt(PIN){\
                              P1IFG=~(PIN);\
                              P1IEN|=(PIN);\
                              P1IFG=~(PIN);\
                              }

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS 
////////////////////////////////////////////////////////////////////////////////

void muJoeGPIO_assignIntMgrOSALEvt( uint8 taskId, uint16 event );
bool muJoeGPIO_registerIntCallback( mujoegpio_pinid_t pinId, pinIntCb_t cb );
void muJoeGPIO_interruptMgr( void );
bool muJoeGPIO_writePin( mujoegpio_pinid_t pinId, bool high );
bool muJoeGPIO_cfgInternalResistor( uint8 port, bool pullDown );
bool mujoeGPIO_initHardware( gpioPin_t *gpioPinTbl, uint8 numPins );

#endif // MUJOEGPIO_H