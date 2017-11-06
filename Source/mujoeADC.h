////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeADC.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEADC_H
#define MUJOEADC_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////

#include "OSAL.h"
#include "hal_adc.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINE                              
////////////////////////////////////////////////////////////////////////////////

#define CCADC_VECTOR                    0x0B                    // ADC ISR Vector

////////////////////////////////////////////////////////////////////////////////
// MACROS                             
////////////////////////////////////////////////////////////////////////////////

#define enableADCInterrupt()            (IEN0 |= 0x02)                  // Set IEN0.ADCIE
#define disableADCInterrupt()           (IEN0 &= ~0x02)                 // Clr IEN0.ADCIE

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS                             
////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    CH_AIN0 = HAL_ADC_CHN_AIN0,         /* AIN0 */
    CH_AIN1 = HAL_ADC_CHN_AIN1,         /* AIN1 */
    CH_AIN2 = HAL_ADC_CHN_AIN2,         /* AIN2 */
    CH_AIN3 = HAL_ADC_CHN_AIN3,         /* AIN3 */
    CH_AIN4 = HAL_ADC_CHN_AIN4,         /* AIN4 */
    CH_AIN5 = HAL_ADC_CHN_AIN5,         /* AIN5 */
    CH_AIN6 = HAL_ADC_CHN_AIN6,         /* AIN6 */
    CH_AIN7 = HAL_ADC_CHN_AIN7,         /* AIN7 */
    CH_A0A1 = HAL_ADC_CHN_A0A1,         /* AIN0,AIN1 */
    CH_A2A3 = HAL_ADC_CHN_A2A3,         /* AIN2,AIN3 */
    CH_A4A5 = HAL_ADC_CHN_A4A5,         /* AIN4,AIN5 */
    CH_A6A7 = HAL_ADC_CHN_A6A7,         /* AIN6,AIN7 */
    CH_GND =  HAL_ADC_CHN_GND,          /* GND */
    CH_VREF = HAL_ADC_CHN_VREF,         /* Positive voltage reference */
    CH_TEMP = HAL_ADC_CHN_TEMP,         /* Temperature sensor */
    CH_VDD_DIV3 = HAL_ADC_CHN_VDD3      /* VDD/3 */
      
}adc_ch_t;

// Maps to ADCCON3.EDIV
typedef enum
{
  ADC_REZ_8 =  0x00,                            // 8 bit rez (7 bit ENOB), 64 decimation rate
  ADC_REZ_10 = 0x10,                            // 10 bit rez (9 bit ENOB), 128 decimation rate
  ADC_REZ_12 = 0x20,                            // 12 bit rez (10 bit ENOB), 256 decimation rate
  ADC_REZ_14 = 0x30                             // 14 bit rez (12 bit ENOB), 512 decimation rate
    
}adc_rez_t;

typedef struct adc_sm_def
{
  uint8                          cb_TaskID; 
  uint16                         cb_EventID;
  uint8                          analogIoMask;
  adc_rez_t                      adc_rez;
  volatile adc_ch_t              chScanned;
  volatile int16                 chConv;
  
}adc_sm_t;


////////////////////////////////////////////////////////////////////////////////
//  API FUNCTION PROTOS  
////////////////////////////////////////////////////////////////////////////////

void mujoeADC_initDriver( uint8 cb_TaskID, uint16 cb_EventID, uint8 analogIoMask );
void mujoeADC_triggerConversion( adc_ch_t ch, adc_rez_t rez );
uint16 mujoeADC_getConversion( uint8 *pChScanned );
float mujoeADC_codeToVolt( uint16 code, float vref );

#endif // #define MUJOEADC_H