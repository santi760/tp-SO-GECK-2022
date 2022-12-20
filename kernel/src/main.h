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
#include "socket.h"
#include "kernelConfig.h"
#include "comunicacion.h"

extern int kernelServer;

extern int interrupt_fd, dispatch_fd, memoria_fd;

#endif /* SRC_MAIN_H_ */
