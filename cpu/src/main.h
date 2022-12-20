/*
 * main.h
 *
 *  Created on: Sep 27, 2022
 *      Author: utnso
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include "socket.h"
#include "cpuConfig.h"
#include "comunicacion.h"

extern int cpuServerInterrupt;
extern int cpuServerDispatch;

typedef struct{
	uint16_t nro_entrada;
	uint16_t pid;
	uint32_t segmento;
	uint32_t pagina;
	uint32_t marco;
	clock_t ultima_referencia;
}TLB_t;

typedef struct{
	uint32_t marco;
	uint32_t desplazamiento;
}marco_t;

void dispatchCpu();
void interruptCpu ();

op_code iniciar_ciclo_instruccion(PCB_t* pcb);
INSTRUCCION* fetch(t_list* instrucciones, uint32_t pc);
int decode(INSTRUCCION* instruccion_ejecutar );
int check_interrupt();
void interrupcion();
int execute(INSTRUCCION* instruccion_ejecutar,uint32_t registros[4], uint16_t pid);
int ejecutarMOV_IN(uint32_t dir_logica, uint16_t pid);
int ejecutarMOV_OUT(uint32_t dir_logica,uint32_t valor, uint16_t pid);

//TLB
void inicializar_tlb();
void limpiar_tlb();
void imprimir_tlb();
TLB_t *crear_entrada_tlb(uint32_t segmento, uint32_t pagina, uint32_t marco);
marco_t  traducir_direccion(uint32_t dir_logica,int operacion, uint16_t pid);
uint32_t presente_en_tlb(uint32_t numero_segmento, uint32_t numero_pagina, uint16_t pid);
void reemplazo_tlb_FIFO(uint32_t numero_segmento, uint32_t numero_pagina, uint32_t marco );
bool menor(TLB_t* a,TLB_t* b);
void reemplazo_tlb_LRU(uint32_t numero_segmento, uint32_t numero_pagina, uint32_t marco );
bool marco_en_tlb(uint32_t marco,uint32_t segmento, uint32_t pagina);


#endif /* SRC_MAIN_H_ */
