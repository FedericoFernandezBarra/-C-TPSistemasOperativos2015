/*
 * funcionesMemoria.h
 *
 *  Created on: 4/9/2015
 *      Author: utnso
 */

#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_

#include <commons/string.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <commonsDeAsedio/error.h>
#include <commonsDeAsedio/cliente-servidor.h>
#include <commonsDeAsedio/estructuras.h>


#define PUERTO_ESCUCHA "PUERTO_ESCUCHA"
#define IP_SWAP "IP_SWAP"
#define PUERTO_SWAP "PUERTO_SWAP"
#define MAXIMO_MARCOS_POR_PROCESO "MAXIMO_MARCOS_POR_PROCESO"
#define CANTIDAD_MARCOS "CANTIDAD_MARCOS"
#define TAMANIO_MARCO "TAMANIO_MARCO"
#define ENTRADAS_TLB "ENTRADAS_TLB"
#define TLB_HABILITADA "TLB_HABILITADA"
#define RETARDO_MEMORIA "RETARDO_MEMORIA"

/*******************ESTRUCTURAS*************************/

typedef struct{
	int puertoDeEscucha;
	char* ipSWAP;
	int puertoSWAP;
	int maximoDeMarcosPorProceso;
	int cantidadDeMarcos;
	int tamanioDeMarco;
	int entradasDeTLB;
	char* TLBHabilitada;
	int retardoDeMemoria;
}tipoConfigMemoria;

typedef struct{
	t_list* listaRAM;
	t_list* listaTLB;
	int socketSWAP;
	int maximoSocket;
	tipoConfigMemoria* configuracion;
	fd_set* cpusATratar;
}tipoEstructuraMemoria;

typedef struct{
	int instruccion;
	int numeroDePagina;
	int posicionEnRAM;
}tipoTLB;

typedef struct{
	int instruccion;
	int numeroDePagina;
	void* contenido;
}tipoRAM;

/***************************FUNCIONES*********************************************/

tipoConfigMemoria* crearConfigMemoria();

void destruirConfigMemoria(tipoConfigMemoria* estructuraDeConfiguracion);

tipoConfigMemoria* cargarArchivoDeConfiguracionDeMemoria(char* rutaDelArchivoDeConfiguracion);

void tratarPeticion(tipoEstructuraMemoria* datosMemoria,int cpuAtendida);

void tratarPeticiones(tipoEstructuraMemoria* datosMemoria);

/**************INSTRUCCIONES*******************/

//////////////////
//INICIAR
/////////////////

int puedoReservarEnSWAP(tipoInstruccion instruccion, int socketSwap, tipoRespuesta* respuesta);

int puedoReservarEnRAM(tipoInstruccion instruccion, tipoEstructuraMemoria* datosMemoria);

void reservarMemoriaEnRam(tipoInstruccion instruccion, tipoEstructuraMemoria* datosMemoria);

int estaHabilitadaLaTLB(tipoConfigMemoria* configuracion);

int obtenerPosicionEnRAM(t_list* listaRAM, tipoRAM* instanciaRAM);

void reservarMemoriaEnTLB(tipoTLB* instanciaTLB, t_list* listaTLB, tipoConfigMemoria* configuracion);

void reservarMemoriaParaProceso(tipoInstruccion instruccion,tipoEstructuraMemoria* datosMemoria, int cpuATratar);

//////////////////
//LEER PAGINA
/////////////////

int buscarPaginaEnRam(tipoInstruccion instruccion, char* contenidoDePagina);

int buscarPaginaEnSwap(tipoInstruccion instruccion, char* contenidoDePagina, int socketSwap);

void enviarPaginaACPU(tipoInstruccion instruccion, char* contenidoDePagina);

void pedirPaginaDesdeSwapARam(tipoInstruccion instruccion, char* contenidoDePagina, int socketSwap);

void leerPagina(tipoInstruccion instruccion, int socketSwap);

#endif /* FUNCIONESMEMORIA_H_ */
