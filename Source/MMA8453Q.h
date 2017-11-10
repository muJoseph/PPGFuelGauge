////////////////////////////////////////////////////////////////////////////////
// @filename: MMA8453Q.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MMA8453Q_H
#define MMA8453Q_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"
#include "mujoeI2C.h"
#include "OSAL_Memory.h"        // for osal_mem_alloc
#include "string.h"             // for memcpy


////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define MMA845xQ_DEFAULT_I2C_WRITE_ADDR     0x38 // Default I2C write address

// Who Am I ID
#define MMA845xQ_WHO_AM_I_ID                0x3A

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

// MMA845xQ Register Map
typedef enum
{
  MMA_REG_STATUS        = 0x00,
  MMA_REG_OUT_X_MSB     = 0x01,
  MMA_REG_OUT_X_LSB     = 0x02,
  MMA_REG_OUT_Y_MSB     = 0x03,
  MMA_REG_OUT_Y_LSB     = 0x04,
  MMA_REG_OUT_Z_MSB     = 0x05,
  MMA_REG_OUT_Z_LSB     = 0x06,
  MMA_REG_SYSMOD        = 0x0B,
  MMA_REG_INT_SOURCE    = 0x0C,
  MMA_REG_WHO_AM_I      = 0x0D,
  MMA_REG_XYZ_DATA_CFG  = 0x0E,
  MMA_REG_HP_FILTER_CUTOFF = 0x0F,
  MMA_REG_PL_STATUS     = 0x10,
  MMA_REG_PL_CFG        = 0x11,
  MMA_REG_PL_COUNT      = 0x12,
  MMA_REG_PL_BF_ZCOMP   = 0x13,
  MMA_REG_PL_THS_REG    = 0x14,
  MMA_REG_FF_MT_CFG     = 0x15,
  MMA_REG_FF_MT_SRC     = 0x16,
  MMA_REG_FF_MT_THS     = 0x17,
  MMA_REG_FF_MT_COUNT   = 0x18,
  MMA_REG_TRANSIENT_CFG = 0x1D,
  MMA_REG_TRANSIENT_SRC = 0x1E,
  MMA_REG_TRANSIENT_THS = 0x1F,
  MMA_REG_TRANSIENT_COUNT = 0x20,
  MMA_REG_PULSE_CFG     = 0x21,
  MMA_REG_PULSE_SRC     = 0x22,
  MMA_REG_PULSE_THSX    = 0x23,
  MMA_REG_PULSE_THSY    = 0x24,
  MMA_REG_PULSE_THSZ    = 0x25,
  MMA_REG_PULSE_TMLT    = 0x26,
  MMA_REG_PULSE_LTCY    = 0x27,
  MMA_REG_PULSE_WIND    = 0x28,
  MMA_REG_ASLP_COUNT    = 0x29,
  MMA_REG_CTRL_REG1     = 0x2A,
  MMA_REG_CTRL_REG2     = 0x2B,
  MMA_REG_CTRL_REG3     = 0x2C,
  MMA_REG_CTRL_REG4     = 0x2D,
  MMA_REG_CTRL_REG5     = 0x2E,
  MMA_REG_OFF_X         = 0x2F,
  MMA_REG_OFF_Y         = 0x30,
  MMA_REG_OFF_Z         = 0x31,

}mma845xq_regAddr_t;     

// System Output Data Rate
typedef enum
{
  MMA_SYSODR_800HZ      = ( 0x00 << 3 ),
  MMA_SYSODR_400HZ      = ( 0x01 << 3 ),
  MMA_SYSODR_200HZ      = ( 0x02 << 3 ),
  MMA_SYSODR_100HZ      = ( 0x03 << 3 ),
  MMA_SYSODR_50HZ       = ( 0x04 << 3 ),
  MMA_SYSODR_12HZ5      = ( 0x05 << 3 ),
  MMA_SYSODR_6HZ25      = ( 0x06 << 3 ),
  MMA_SYSODR_1HZ56      = ( 0x07 << 3 )
  
}mma845xq_sysOdr_t;

// Sleep Mode Output Data Rate
typedef enum
{
  MMA_SLPODR_50HZ      = ( 0x00 << 6 ),
  MMA_SLPODR_12HZ5     = ( 0x01 << 6 ),
  MMA_SLPODR_6HZ25     = ( 0x02 << 6 ),
  MMA_SLPODR_1HZ56     = ( 0x03 << 6 ),
  
}mma845xq_slpOdr_t;

// Active Mode Power Scheme
typedef enum
{
  MMA_MODS_NRML        = 0x00, // Normal
  MMA_MODS_LNLP        = 0x01, // Low Power Low Noise
  MMA_MODS_HREZ        = 0x02, // High Resolution
  MMA_MODS_LP          = 0x03, // Low Power
  
}mma845xq_actPwrSch_t;

// Sleep Mode Power Scheme
typedef enum
{
  MMA_SMODS_NRML        = ( 0x00 << 3 ), // Normal
  MMA_SMODS_LNLP        = ( 0x01 << 3 ), // Low Power Low Noise
  MMA_SMODS_HREZ        = ( 0x02 << 3 ), // High Resolution
  MMA_SMODS_LP          = ( 0x03 << 3 ), // Low Power
  
}mma845xq_slpPwrSch_t;

typedef struct MMA845xQ_def
{
  uint8         i2cWriteAddr;
  uint8         *pBuff;
  uint8         buffSize;
  
}MMA845xQ_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool MMA8453Q_initDriver( uint8 buffSize, bool sa0 );
bool MMA8453Q_readReg( mma845xq_regAddr_t addr, uint8 *pData );
bool MMA8453Q_bulkRead( mma845xq_regAddr_t stAddr, uint8 *pData, uint8 numBytes );
bool MMA8453Q_writeReg( mma845xq_regAddr_t addr, uint8 data );

#endif // MMA8453Q_H