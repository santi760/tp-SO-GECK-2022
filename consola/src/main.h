/*
 * main.h
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "socket.h"
#include "protocolo.h"
#include "consolaConfig.h"



#define LONGITUD_MAXIMA_LINEA 30   // Para el archivo a leer

void parseo_instrucciones(char* path_instrucciones, t_list* listaIntrucciones);



#endif /* SRC_MAIN_H_ */
