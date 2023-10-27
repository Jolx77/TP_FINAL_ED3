#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include <stdio.h>
#include <math.h>
uint32_t frecuencia = 10000;
uint32_t sen[1024];
uint32_t cuad[1024];
uint32_t triang[1024];
uint32_t samples;

int t;


void generarSenoidal(){
	for (int i = 0; i < samples; i++) {
	    double t = (double)i / samples;
	    uint32_t val = 512 + (512 * (sin(2.0 * 3.14159265358979323846 * t)));
	    sen[i] = val << 6;
	}

}

void generarCuadrada(){
	for(uint16_t i = 0; i<samples/2;i++){
		cuad[i]=1023 << 6;
	}
	for(uint16_t j = samples/2; j<samples;j++){
		cuad[j]=0 << 6;
	}
}

void generarTriangular(){

	for(uint16_t i = 0;i<samples/2;i++){
		uint32_t val = (double)i / (samples/2 - 1) * 1024;
		triang[i] = val << 6;
	}
	uint16_t count = (samples/2)-1;
	for(uint16_t j = (samples/2);j<samples;j++){
		triang[j] = triang[count];
		count--;
	}
}

void confPin(void){
	//Configuración pin P0.26 como AOUT
	PINSEL_CFG_Type pinsel_cfg;
	pinsel_cfg.Portnum = 0;
	pinsel_cfg.Pinnum = 26;
	pinsel_cfg.Funcnum = 2;
	pinsel_cfg.Pinmode = 0;
	pinsel_cfg.OpenDrain = 0;
	PINSEL_ConfigPin(&pinsel_cfg);
	return;
}

void confDAC(void){
	DAC_CONVERTER_CFG_Type dacCfg;
	dacCfg.CNT_ENA = SET;
	dacCfg.DMA_ENA = SET;
	t = (CLKPWR_GetPCLK(CLKPWR_PCLKSEL_DAC)/(frecuencia*samples));
	DAC_Init (LPC_DAC);
	DAC_SetDMATimeOut(LPC_DAC, t);
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCfg);
}
void delay(){
	for (uint32_t i=0;i<400000;i++){}
	return;
}

void configDMA(){

	GPDMA_LLI_Type LLI2;
	GPDMA_LLI_Type LLI3;
	GPDMA_LLI_Type LLI1;

	LLI1.SrcAddr = (uint32_t) sen;
	LLI1.DstAddr = (uint32_t) &LPC_DAC->DACR;
	LLI1.NextLLI = (uint32_t) &LLI1;
	LLI1.Control = samples
				   | (2<<18) //source width 32 bits
				   | (2<<21) //dest width 32 bits
				   | (1<<26); //source increment

	LLI2.SrcAddr = (uint32_t) cuad;
	LLI2.DstAddr = (uint32_t) &LPC_DAC->DACR;
	LLI2.NextLLI = (uint32_t) &LLI2;
	LLI2.Control = samples
				   | (2<<18) //source width 32 bits
				   | (2<<21) //dest width 32 bits
				   | (1<<26); //source increment


	LLI3.SrcAddr = (uint32_t) triang;
	LLI3.DstAddr = (uint32_t) &LPC_DAC->DACR;
	LLI3.NextLLI = (uint32_t) &LLI3;
	LLI3.Control = samples
				   | (2<<18) //source width 32 bits
				   | (2<<21) //dest width 32 bits
				   | (1<<26); //source increment

	GPDMA_Init();

	GPDMA_Channel_CFG_Type GPDMACfg;
	GPDMACfg.ChannelNum = 0;
	GPDMACfg.SrcMemAddr = (uint32_t)sen;
	GPDMACfg.DstMemAddr = 0;
	GPDMACfg.TransferSize = samples;
	GPDMACfg.TransferWidth = 0;
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg.SrcConn = 0;
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	GPDMACfg.DMALLI = (uint32_t)&LLI1;
	GPDMA_Setup(&GPDMACfg);
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

void configADC(){
	ADC_Init(LPC_ADC,200000);
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);
	ADC_BurstCmd(LPC_ADC,ENABLE);
	ADC_StartCmd(LPC_ADC,0);
	ADC_IntConfig(LPC_ADC,0,ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
}

int main(){
	samples = 1000000/frecuencia;
	if(samples > 1024){samples = 1024;}
	generarTriangular();
	generarCuadrada();
	generarSenoidal();
	confPin();
	confDAC();
	configDMA();
	GPDMA_ChannelCmd(0, ENABLE);
	while(1){
	}
	return 0;
}


