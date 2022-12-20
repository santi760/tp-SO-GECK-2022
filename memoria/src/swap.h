/*
 * swap.h
 *
 *  Created on: Nov 21, 2022
 *      Author: utnso
 */

#ifndef SRC_SWAP_H_
#define SRC_SWAP_H_

#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include <stdint.h>
#include "memoriaConfig.h"



void inicializar_swap();

// crear archivo
int crear_swap(uint32_t tamanio_en_bytes);

extern int fd;

// borrar archivo
void borrar_swap(int fd);

// actualizar marco en archivo
void actualizar_marco_en_swap(int fd, uint32_t nro_marco, void* marco, uint32_t tamanio_marcos);

// leer marco
void* leer_marco_en_swap(int fd, uint32_t nro_marco, uint32_t tamanio_marcos);

#endif
