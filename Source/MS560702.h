////////////////////////////////////////////////////////////////////////////////
// @filename: MS560702.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MS560702_H
#define MS560702_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "hal_types.h"
#include "mujoeI2C.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////

#define MS560702_DEFAULT_I2C_WRITE_ADDR     0xEE // Default I2C write address

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  MS5_CMD_RESET =       0x1E, // Chip reset command
  MS5_CMD_ADC_READ =    0x00, // ADC read command
  MS5_CMD_ADC_CONV =    0x40, // ADC conversion command
  MS5_CMD_ADC_D1 =      0x00, // ADC D1 conversion (Pressure)
  MS5_CMD_ADC_D2 =      0x10, // ADC D2 conversion (Temperature)
  MS5_CMD_ADC_256 =     0x00, // ADC OSR=256
  MS5_CMD_ADC_512 =     0x02, // ADC OSR=512
  MS5_CMD_ADC_1024 =    0x04, // ADC OSR=1024
  MS5_CMD_ADC_2048 =    0x06, // ADC OSR=2048
  MS5_CMD_ADC_4096 =    0x08, // ADC OSR=4096
  MS5_CMD_PROM_RD =     0xA0  // Prom read command
  
}MS560702_cmds_t;

typedef enum
{
  MS5_OSR_256 =         0x00,
  MS5_OSR_512 =         0x02,
  MS5_OSR_1024 =        0x04,
  MS5_OSR_2048 =        0x06,
  MS5_OSR_4096 =        0x08,
  
} MS560702_osr_t;

typedef enum
{
  MS5_PROM_RESERVED_ADDR = 0x00,
  MS5_PROM_COEFF1_ADDR,
  MS5_PROM_COEFF2_ADDR,
  MS5_PROM_COEFF3_ADDR,
  MS5_PROM_COEFF4_ADDR,
  MS5_PROM_COEFF5_ADDR,
  MS5_PROM_COEFF6_ADDR,
  MS5_PROM_CRC_ADDR
  
}MS560702_promCoeffAddr_t;

typedef struct MS560702_def
{
  uint8         i2cWriteAddr;
  uint16        prom[8];        // Index 0: Mfg reserved, Indices 1-6: Coefficients, Index 7: CRC for coefficients
  //uint16        coeffTbl[7];    // Indices 0-5: Coefficients, Index 6: CRC for coefficients
  //uint8         coeffCrc;
  
}MS560702_t;

////////////////////////////////////////////////////////////////////////////////
// API FUNCTION PROTOS
////////////////////////////////////////////////////////////////////////////////

bool MS560702_initDriver( bool csbState );
bool MS560702_initHardware( void );
bool MS560702_trigPressureConv( MS560702_osr_t osr );
bool MS560702_trigTemperatureConv( MS560702_osr_t osr );
bool MS560702_readAdcConv( uint32 *pAdcCode );


#endif // MS560702