////////////////////////////////////////////////////////////////////////////////
// @filename: mujoeI2C.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEI2C_H
#define MUJOEI2C_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////
   
#include "hal_types.h"
#include "iocc2541.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

// I2CCFG SFR-- I2C Control (Pg. 179)

#define I2C_ENS1            0x40   //I2C Module Enable Bit. Set to enable
#define I2C_STA             0x20   //START Flag. When set, HW detects when I2C is free and generates START condition
#define I2C_STO             0x10   //STOP Flag. When set and in master mode, a STOP condition is transmitted on I2C bus.
                                   //HW is cleared when transmit has completed succesfully.
#define I2C_SI              0x08   //Interrupt Flag

#define I2C_AA              0x04   //Assert Acknowledge flag for I2C module
                                   //When set, an acknowledge is returned when:
                                   //      -Slave address is recognized
                                   //      -General call is recognized, when I2C module is enabled
                                   //      -Data byte received while in master/slave receive mode
                                   //When clear, an acknowledge is returned when:
                                   //      -Data byte is received while in master/slave receive mode

                                   //Clock Rates Defined at 32MHz
#define BIT_FREQ_123KHZ     0x00   // CLK/256
#define BIT_FREQ_144KHZ     0x01   // CLK/244
#define BIT_FREQ_165KHZ     0x02   // CLK/192
#define BIT_FREQ_197KHZ     0x03   // CLK/160
#define BIT_FREQ_33KHZ      0x80   // CLK/960
#define BIT_FREQ_267KHZ     0x81   // CLK/120
#define BIT_FREQ_533KHZ     0x82   // CLK/60

#define I2C_MST_RD_BIT      0x01   // Master RD/WRn bit to be OR'ed with Slave address.

#define I2C_CLOCK_MASK      0x83   

#define I2C_PXIFG           P2IFG
#define I2C_IF              P2IF
#define I2C_IE              BV(1)

#define STOP_CMD            0x01   // Issue STOP command at the end of I2C write 
#define REPEAT_CMD          0x00   // DO NOT issue a STOP command at the end of I2C write

////////////////////////////////////////////////////////////////////////////////
//                              TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

//  HAL_I2C_MASTER mode status definitions
typedef enum
{                                
  mstStarted   = 0x08,            // START Condition has been transmitted                  
  mstRepStart  = 0x10,            // Repeated START condition has been transmitted
  mstAddrAckW  = 0x18,            // SLA (Slave Address) + W has been transmitted; ACK has been received
  mstAddrNackW = 0x20,            // SLA (Slave Address) + W has been transmitted; not-ACK has been received
  mstDataAckW  = 0x28,            // Data byte is transmitted; ACK is received.
  mstDataNackW = 0x30,            // Data byte in I2CDATA has been transmitted; not-ACK has been received
  mstLostArb   = 0x38,            // Arbitration lost in SLA + R/Wn or data bytes (not-ACK received)                           
  mstAddrAckR  = 0x40,            // SLA + R has been transmitted, ACK has been received.
  mstAddrNackR = 0x48,            // SLA + R has been transmitted; not-ACK has been received.
  mstDataAckR  = 0x50,            // Data byte has been received; ACK has been returned.
  mstDataNackR = 0x58,            // Data byte has been received; not-ACK has been returned.
  unknownErr   = 0xF8,            // Indicates that there is no relevant state information available and that I2CCFG.SI = 0

} i2cStatus_t;

// 2C bus clock rate definitions at 32 MHz.
typedef enum
{
  i2cClock_123KHZ = 0x00,
  i2cClock_144KHZ = 0x01,
  i2cClock_165KHZ = 0x02,
  i2cClock_197KHZ = 0x03,
  i2cClock_33KHZ  = 0x80,
  i2cClock_267KHZ = 0x81,
  i2cClock_533KHZ = 0x82
    
} i2cClock_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

void mujoeI2C_initHardware( i2cClock_t clockRate );
uint8 mujoeI2C_read( uint8 addr, uint8 len, uint8 *pBuf );
uint8 mujoeI2C_write( uint8 addr, uint8 len, uint8 *pBuf, uint8 stp );

#endif // #define MUJOEI2C_H