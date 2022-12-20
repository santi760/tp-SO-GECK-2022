/*
 * pcb.h
 *
 *  Created on: 24 sept 2022
 *      Author: utnso
 */

#ifndef SRC_PCB_H_
#define SRC_PCB_H_

#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<commons/collections/list.h>

typedef struct{
  uint32_t tamSegmento;
  uint32_t id_tabla_pagina;
}TABLA_SEGMENTO;

typedef struct {
	uint16_t pid;
	t_list* instrucciones;
	uint32_t pc;
	uint32_t registro_cpu[4];
	t_list* segmentos;
	t_list* nros_segmentos;
	int cliente_fd;
}PCB_t;

PCB_t* pcb_create();

void pcb_set(PCB_t* pcb,uint16_t pid, t_list* instrucciones, uint32_t pc, uint32_t registro_cpu[4], t_list* segmentos, int cliente);

int pcb_find_index(t_list* lista, uint16_t pid);

void pcb_destroy(PCB_t* pcb);


#endif /* SRC_PCB_H_ */
