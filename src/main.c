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
#include "math.h"
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



int main(void) {
	generarTriangular();
	generarCuadrada();
	generarSenoidal();


    return 0 ;
}
