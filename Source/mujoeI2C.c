////////////////////////////////////////////////////////////////////////////////
// @filename: mujoeI2C.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "mujoeI2C.h"

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VAR
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// LOCAL VAR
////////////////////////////////////////////////////////////////////////////////

static i2cClock_t SCL_CLK_FREQ_BUFFER;

////////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOS                           
////////////////////////////////////////////////////////////////////////////////

static uint8 masterStartI2C( uint8 addr, uint8 R_Wn );
static void enableI2C( void );
static void disableI2C( void );

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS                             
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// @fn          mujoeI2C_initHardware
//
// @brief       Initialize the I2C bus as a Master.
//
// input parameters
//
// @param       address - I2C slave address.
// @param       clockRate - I2C clock rate.
//
// output parameters
//
// None.
//
// @return      None.
//
////////////////////////////////////////////////////////////////////////////////
void mujoeI2C_initHardware( i2cClock_t clockRate )
{
  SCL_CLK_FREQ_BUFFER = clockRate;  // Store Clock Rate
  I2CWC = 0x00;                     // I2CWC.OVR = 0, I2C functionality enabled on pins 2 and 3 of CC2541 (wrapper disabled)         
  I2CADDR = 0;                      // No multi-master support at this time
  
  I2CCFG  &= ~I2C_CLOCK_MASK;       // Clear I2CCFG bits CR2, CR1, CR0
  I2CCFG  |= clockRate;             // Set/clear the appopriate combination of CR2,C1,C0   
  
  I2CCFG |= I2C_ENS1;               // Enable the I2C module
  
} // mujoeI2C_initHardware



////////////////////////////////////////////////////////////////////////////////
// @fn          mujoeI2C_read
//
// @brief       Read from the I2C bus as a Master.
//
// @param       len - Number of bytes to read.
// @param       pBuf - Pointer to the data buffer to put read bytes.
//
// @return      The number of bytes successfully read.
//
////////////////////////////////////////////////////////////////////////////////
uint8 mujoeI2C_read( uint8 addr, uint8 len, uint8 *pBuf )
{
  uint8 cnt = 0;

  if ( masterStartI2C(addr,I2C_MST_RD_BIT) != mstAddrAckR )
  {
    len = 0;
  }
                                                // ***NOTE: All bytes are ACK'd except for the last one which is NACK'd. If only
                                                // 1 byte is being read, a single NACK will be sent. Thus, we only want
                                                // to enable ACK if more than 1 byte is going to be read.
  if (len > 1)
  {
    I2CCFG |=  I2C_AA;
  }

  while (len > 0)
  {
                                                // ***NOTE: slave devices require NACK to be sent after reading last byte
    if (len == 1)
    {
      I2CCFG &= ~I2C_AA;
    }
                                                // Stop clock-stretching 
    I2CCFG &= ~I2C_SI;                          // Clear interrupt flag  
    while ((I2CCFG & I2C_SI) == 0);             // Wait for interrupt flag to be set
    
    (*pBuf++) = I2CDATA;                        // Read incoming data from I2C I/O SFR (I2CDATA)
    cnt++;                                      // Increment count, decrement amount of total bytes to read (i.e. the remaining amt of bytes to read)
    len--;

    if (I2CSTAT != mstDataAckR)                 // Check to see if:
    {                                           //    - data byte has been received; and ACK has been returned OR
      if (I2CSTAT != mstDataNackR)              //    - data byte has been received; not-ACK has been returned.
      {                                         // If neither: something went wrong, don't count last byte
        cnt--;
      }
      break;
    }
  }
  
  {
                                                  // *NOTE: Must set STOP before clearing I2CC.SI bit
    I2CCFG |= I2C_STO;                            // Set STOP flag                
    I2CCFG &= ~I2C_SI;                            // Clear interrupt flag
    while ((I2CCFG & I2C_STO) != 0);              // Wait until STOP flag is cleared by hardware after transmit has completed
  }

  return cnt;                                   // The number of bytes successfully read.
} // mujoeI2C_read

////////////////////////////////////////////////////////////////////////////////
// @fn          mujoeI2C_write
//
// @brief       Write to the I2C bus as a Master.
//
// input parameters
//
// @param       len - Number of bytes to write.
// @param       pBuf - Pointer to the data buffer to write.
// @param       stp - If set, STOP command issued at the end of transaction
//
// output parameters
//
// None.
//
// @return      The number of bytes successfully written.
//
////////////////////////////////////////////////////////////////////////////////
uint8 mujoeI2C_write( uint8 addr, uint8 len, uint8 *pBuf, uint8 stp )
{
  if ( ( masterStartI2C(addr,0) ) != mstAddrAckW )   // Attempt to send an I2C bus START and Slave Address 
                                                     // (stored in global variable "i2cAddr" as an I2C bus Master.
  {
    len = 0;                                    // If ACK was not received, something went wrong, jump to end of function and return to caller
    stp = 0x01;
  }

  for (uint8 cnt = 0; cnt < len; cnt++)         // Send byte(s) to slave as I2C Master
  {
    I2CDATA = *pBuf++;                          // Load outgoing byte into I2C Serial I/0 SFR                 
    I2CCFG &= ~I2C_SI;                          // Clear Interrupt flag        
    while ((I2CCFG & I2C_SI) == 0);             // Wait until SI interrupt Flag

    if (I2CSTAT != mstDataAckW)                 // If the I2C status code is something else other than: Ack...
    {
      if (I2CSTAT == mstDataNackW)              // If the I2C status code was NOT-Ack...
      {
        len = cnt + 1;                          // Last byte (of total byte package) was transmitted, update total bytes transmitted and break from loop
      }
      else                                      // If the I2C status code is something else other than: NOT-Ack...
      {
        len = cnt;                              // Do NOT count last byte transmitted as successful and break from loop
        stp = 0x01;
      }
      break;
    }
  }
  
  if( stp == 0x01 )                               // Check to see if STOP command flag set
  {                                               // *NOTE: Must set STOP before clearing I2CC.SI bit
    I2CCFG |= I2C_STO;                            // Set STOP flag                
    I2CCFG &= ~I2C_SI;                            // Clear interrupt flag
    while ((I2CCFG & I2C_STO) != 0);              // Wait until STOP flag is cleared by hardware after transmit has comple
  }
  
  return len;                                     // Return the number of bytes successfully transmitted
} // mujoeI2C_write

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS                             
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// @fn      masterStartI2C
//
// @brief   Attempt to send an I2C bus START and Slave Address as an I2C bus Master.
//
// @param   R_Wn - The LSB of the Slave Address as Read/~Write.
//
// @return  The I2C status of the START request or of the Slave Address Ack.
//
////////////////////////////////////////////////////////////
static uint8 masterStartI2C( uint8 addr, uint8 R_Wn )
{
 
  mujoeI2C_initHardware( SCL_CLK_FREQ_BUFFER );  // I2C Settings are not recalled after returning from sleep, must restore via SW
  
  {
     I2CCFG = (I2CCFG & ~I2C_SI) | I2C_STA;  // Clear any pending I2C interrupt flag and set START flag
     while ((I2CCFG & I2C_SI) == 0);         // Wait for interrupt flag to be set 
     I2CCFG &= ~I2C_STA;                     // Clear START flag
  }
  
  if ( (I2CSTAT == mstStarted) || (I2CSTAT == mstRepStart) )   // A (re)start condition has been transmitted 
  {
    I2CDATA = (addr | R_Wn);            // Load SLA (slave address) + R/Wn into I2CDATA (I2C serial data I/O SFR)
                                        // Stop clock-stretching
    I2CCFG &= ~I2C_SI;                  // Clear interrupt flag         
    while ( (I2CCFG & I2C_SI) == 0 );   // Wait for interrupt flag to be set   
  }

  return I2CSTAT;
} // masterStartI2C

////////////////////////////////////////////////////////////
// @fn      enableI2C
//
// @brief   Enable I2C module of CC2541
//
// @param   void
//
// @return  void
//
////////////////////////////////////////////////////////////
static void enableI2C( void )
{
  I2CCFG |= I2C_ENS1; 
} // enableI2C

////////////////////////////////////////////////////////////
// @fn      disableI2C
//
// @brief   Disable I2C module of CC2541
//
// @param   void
//
// @return  void
//
////////////////////////////////////////////////////////////
static void disableI2C( void )
{
  I2CCFG &= ~I2C_ENS1; 
} // disableI2C