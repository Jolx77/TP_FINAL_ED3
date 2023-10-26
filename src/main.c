/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#endif

#include <cr_section_macros.h>
uint16_t sen[1024];
uint16_t cuad[1024];
uint16_t triang[1024];
uint16_t frecuencia = 1000;

void generarSenoidal(){
	for(uint16_t i = 0; i < 1024;i++){
		sen[i] = 512 + (i * sin(2*i*3.14159*frecuencia/1024));
	}
}

void generarCuadrada(){
	for(uint16_t i = 0; i<512;i++){
		cuad[i]=1023;
	}
	for(uint16_t j = 512; j<1024;j++){
		cuad[j]=0;
	}
}

void generarTriangular(){
	for(uint16_t i = 0;i<512;i++){
		triang[i]=i;
	}
	for(uint16_t i = 0;i<512;i++){
		triang[i]=512-i;
	}
}

void configADC(){
	ADC_Init(LPC_ADC,200000);
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);
	ADC_BurstCmd(LPC_ADC,ENABLE);
	ADC_StartCmd(LPC_ADC,0);
	ADC_IntConfig(LPC_ADC,0,ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
}

void configDAC(){
	DAC_Init(LPC_DAC);
	DAC_SetDMATimeOut(LPC_DAC,(CLKPWR_GetPCLKSEL(CLKPWR_PCLKSEL_DAC)/(frecuencia*1024)));
}

void configTimer1(){
	TIM_TIMERCFG_Type timcfg;
	timcfg.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timcfg.PrescaleValue = 100;
	TIM_Init(LPC_TIM1,TIM_TIMER_MODE,&timcfg);
	TIM_Cmd(LPC_TIM1,ENABLE);
}

void configTimer0(){
	TIM_TIMERCFG_Type timcfg;
	timcfg.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timcfg.PrescaleValue = 25000;
	TIM_MATCHCFG_Type matcfg;
	matcfg.MatchValue = 1249;
	matcfg.IntOnMatch = ENABLE;
	matcfg.MatchChannel = 0;
	matcfg.ResetOnMatch = ENABLE;
	matcfg.StopOnMatch = DISABLE;
	matcfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&timcfg);
	TIM_ConfigMatch(LPC_TIM0,&matcfg);
	TIM_Cmd(LPC_TIM1,ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}


void configDMA(){
	GPDMA_Init();
	GPDMA_LLI_Type LLISen;
	GPDMA_LLI_Type LLICuad;
	GPDMA_LLI_Type LLITriang;
	GPDMA_Channel_CFG_Type dmacfg;
	dmacfg.ChannelNum = 0;
	dmacfg.DstConn = GPDMA_CONN_DAC;
	dmacfg.SrcConn = 0;
	dmacfg.DstMemAddr = 0;
	dmacfg.SrcMemAddr = &sen;
	dmacfg.TransferSize = 1024;
	dmacfg.TransferWidth = 0;
	dmacfg.DMALLI = &LLISen;

	LLISen.DstAddr = &sen;
	LLISen.SrcAddr = LPC_DAC->DACR;
	LLISen.Control = (1024 | (2<<19) | (2<<21) | (1<<26) & ~(1<<27) & ~(1<<31));
	LLISen.NextLLI = &LLISen;

	LLISen.DstAddr = &cuad;
	LLISen.SrcAddr = LPC_DAC->DACR;
	LLISen.Control = (1024 | (2<<19) | (2<<21) | (1<<26) & ~(1<<27) & ~(1<<31));
	LLISen.NextLLI = &LLICuad;

	LLISen.DstAddr = &triang;
	LLISen.SrcAddr = LPC_DAC->DACR;
	LLISen.Control = (1024 | (2<<19) | (2<<21) | (1<<26) & ~(1<<27) & ~(1<<31));
	LLISen.NextLLI = &LLITriang;
	GPDMA_Setup(&dmacfg);
	GPDMA_ChannelCmd(0,ENABLE);
}

void calcFrec(){
//	if(((ADC_GlobalGetData(LPC_ADC)>>4)&0xFFF)>promf){

	//}
}

int main(void) {
	generarTriangular();
	generarCuadrada();
	generarSenoidal();
	DAC_Init(LPC_DAC);
	//configDAC();
	//configDMA();
	while(1){
		for(int i =0;i<1024;i++){
			DAC_UpdateValue(LPC_DAC,sen[i]);
		}
	}

    return 0 ;
}
