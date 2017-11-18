////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeADC.c
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE                              
////////////////////////////////////////////////////////////////////////////////

#include "mujoeADC.h"

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
////////////////////////////////////////////////////////////////////////////////

static adc_sm_t                 adc_sm;

////////////////////////////////////////////////////////////////////////////////
// STATIC FNC PROTOS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void mujoeADC_initDriver( uint8 cb_TaskID, uint16 cb_EventID, uint8 analogIoMask )
{
  adc_sm.cb_TaskID = cb_TaskID;
  adc_sm.cb_EventID = cb_EventID;
  adc_sm.analogIoMask = analogIoMask;
  adc_sm.chScanned = (adc_ch_t)0;
  adc_sm.chConv = 0;
  
  P0INP |= adc_sm.analogIoMask;                         // Disable internal PU/PD res (i.e. put in 3-state)
  APCFG |= adc_sm.analogIoMask;                         // Enable Analog IO on specified pins
  
} // mujoeADC_initDriver

void mujoeADC_triggerConversion( adc_ch_t ch, adc_rez_t rez )
{
  adc_sm.adc_rez = rez;                                 // Store Rez in local var 
  ADCCON3 = HAL_ADC_REF_AVDD + adc_sm.adc_rez + ch;     // Set ADC referance to AVDD5, decimation rate and channel to scan
  
} //mujoeADC_triggerConversion

uint16 mujoeADC_getConversion( uint8 *pChScanned )
{
  // Treat small negative as 0
  if ( adc_sm.chConv < 0 )
    adc_sm.chConv = 0;

  switch ( adc_sm.adc_rez )
  {
    case ADC_REZ_8:
      adc_sm.chConv >>= 8;
      break;
    case ADC_REZ_10:
      adc_sm.chConv >>= 6;
      break;
    case ADC_REZ_12:
      adc_sm.chConv >>= 4;
      break;
    case ADC_REZ_14:
    default:
      adc_sm.chConv >>= 2;
      break;
  }
  
  *pChScanned = (uint8)adc_sm.chScanned;
  return (uint16)adc_sm.chConv;
  
} // mujoeADC_getConversion

float mujoeADC_codeToVolt( uint16 code, float vref )
{
  float volt;                   // volt = code * vref/ [ ( 2^(N-1) ) - 1 ] where N is number of rez bits
  
  switch( adc_sm.adc_rez )
  {
    case ADC_REZ_8:
      volt = vref/(127.0);
      break;
    case ADC_REZ_10:
      volt = vref/(511.0);
      break;
    case ADC_REZ_12:
      volt = vref/(2047.0);
      break;
    case ADC_REZ_14:
    default:
      volt = vref/(8191.0);
      break;
  }
  
  volt *= code;
  
  return volt;
  
} // mujoeADC_codeToVolt

////////////////////////////////////////////////////////////////////////////////
// INTERRUPT SERVICE ROUTINE
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// ADC ISR
////////////////////////////////////////////////////////////////////////////////
HAL_ISR_FUNCTION( CC2541_ADC_ISR, CCADC_VECTOR )
{
  HAL_ENTER_ISR();
  adc_sm.chConv = (int16)(ADCL);
  adc_sm.chConv |= (int16)(ADCH << 8);
  adc_sm.chScanned = (adc_ch_t)(ADCCON2 & 0x0F);
  osal_set_event( adc_sm.cb_TaskID, adc_sm.cb_EventID );
  HAL_EXIT_ISR();
  return;
}