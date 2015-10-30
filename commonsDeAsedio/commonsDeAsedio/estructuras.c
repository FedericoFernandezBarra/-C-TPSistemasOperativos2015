/*
 * estructuras.c
 *
 *  Created on: 3/10/2015
 *      Author: utnso
 */
#include "estructuras.h"

//void* serializarPCB(tipoPCB pcb){
//
//	size_t tamanioRuta = strlen(pcb.ruta)+sizeof(char);
//
//	size_t tamanioBloque = 2*sizeof(int)+sizeof(char)+tamanioRuta+sizeof(size_t);
//
//	void* buffer = malloc(tamanioBloque+sizeof(size_t));
//
//	void* proximo = buffer;
//
//	memcpy(proximo,&tamanioBloque,sizeof(size_t));  proximo+=sizeof(size_t);
//	memcpy(proximo,&(pcb.pid),sizeof(int));           proximo+=sizeof(int);
//	memcpy(proximo,&(pcb.estado),sizeof(char));  		proximo+=sizeof(char);
//	memcpy(proximo,&(pcb.insPointer),sizeof(int));  	proximo+=sizeof(int);
//	memcpy(proximo,&tamanioRuta,sizeof(size_t));  proximo+=sizeof(size_t);
//	memcpy(proximo,pcb.ruta,tamanioRuta);
//
//	return buffer;
//}

void imprimirBufferPCB(void* buffer){

	void* proximo = buffer;

	printf("--------BUFFER-----------\n");
	printf("PCB:\n");
	printf("PID: %d\n",*((int*)proximo));     proximo+=sizeof(int);
	printf("STATE: %c\n",*((char*)proximo));	 proximo+=sizeof(char);
	printf("IP: %d\n",*((int*)proximo));		 proximo+=sizeof(int)+sizeof(size_t);
	printf("DIR: %s\n",(char*)proximo);
	printf("-------------------------\n");

}

void imprimirBufferInstruccion(void* buffer){

	void* proximo = buffer;

	printf("--------BUFFER-----------\n");
	printf("INSTRUCCION:\n");
	printf("PID: %d\n",*((int*)proximo));     proximo+=sizeof(int);
	printf("INST: %c\n",*((char*)proximo));	 proximo+=sizeof(char);
	printf("PAG: %d\n",*((int*)proximo));		 proximo+=sizeof(int)+sizeof(size_t);
	printf("TEXT: %s\n",(char*)proximo);
	printf("-------------------------\n");

}

//tipoPCB* recibirPCB(int socketEnviador){
//
//	size_t tamanioBloque;
//
//	void* bloque = recibirBloque(&tamanioBloque,socketEnviador);
//
//	tipoPCB* pcbRecibido = malloc(sizeof(tipoPCB));
//
//	deserializarPCB(bloque,pcbRecibido);
//
//	free(bloque);
//
//	return pcbRecibido;
//}
//
//void enviarPCB(int socketCliente,tipoPCB pcb){
//
//	size_t tamanioBloque = 2*(sizeof(char)+sizeof(int)+sizeof(size_t))+strlen(pcb.ruta);
//
//	void* bloque = serializarPCB(pcb);
//
//	enviarMensaje(socketCliente,bloque,tamanioBloque);
//
//	free(bloque);
//}
//
//void deserializarPCB(void* bloque, tipoPCB* pcbRecibido){
//
//	void* proximo = bloque;
//
//	size_t tamanioRuta;// = tamanioBloque-2*sizeof(int)-sizeof(char);
//
//	memcpy(&(pcbRecibido->pid),proximo,sizeof(int));         proximo+=sizeof(int);
//	memcpy(&(pcbRecibido->estado),proximo,sizeof(char));	 proximo+=sizeof(char);
//	memcpy(&(pcbRecibido->insPointer),proximo,sizeof(int));	 proximo+=sizeof(int);
//	memcpy(&tamanioRuta,proximo,sizeof(size_t));			 proximo+=sizeof(size_t);
//	pcbRecibido->ruta = malloc(tamanioRuta);
//	memcpy(pcbRecibido->ruta,proximo,tamanioRuta);
//
//}

tipoPCB* recibirPCB(int socketEnviador){

	size_t tamanioRuta;

	tipoPCB* respuesta = malloc(sizeof(tipoPCB));

	recibirMensajeCompleto(socketEnviador, &(respuesta->pid), sizeof(int));

	recibirMensajeCompleto(socketEnviador, &(respuesta->insPointer), sizeof(int));

	recibirMensajeCompleto(socketEnviador, &(respuesta->estado), sizeof(char));

	recibirMensajeCompleto(socketEnviador, &tamanioRuta, sizeof(size_t));

	respuesta->ruta = malloc(tamanioRuta);

	recibirMensajeCompleto(socketEnviador, respuesta->ruta, tamanioRuta);

	return respuesta;

}

void enviarPCB(int socketCliente, tipoPCB* PCB)
{
		size_t tamanioRuta = strlen(PCB->ruta) + sizeof(char);

		enviarMensaje(socketCliente, &(PCB->pid), sizeof(int));

		enviarMensaje(socketCliente, &(PCB->insPointer), sizeof(int));

		enviarMensaje(socketCliente, &(PCB->estado), sizeof(char));

		enviarMensaje(socketCliente, &tamanioRuta, sizeof(int));

		enviarMensaje(socketCliente, PCB->ruta, tamanioRuta);
}

tipoInstruccion* recibirInstruccion(int socketEnviador){

	/*size_t tamanioBloque;

	void* buffer= recibirBloque(&tamanioBloque,socketEnviador);

	tipoInstruccion* instruccionRecibida = malloc(sizeof(tipoInstruccion));

	deserializarInstruccion(buffer,instruccionRecibida);

	return instruccionRecibida;*/

	tipoInstruccion* instruccion = malloc(sizeof(tipoInstruccion));

	size_t tamanioTexto;

	recibirMensajeCompleto(socketEnviador,&(instruccion->pid),sizeof(int));
	recibirMensajeCompleto(socketEnviador,&(instruccion->instruccion),sizeof(char));
	recibirMensajeCompleto(socketEnviador,&(instruccion->nroPagina),sizeof(int));
	recibirMensajeCompleto(socketEnviador,&tamanioTexto,sizeof(size_t));
	instruccion->texto = malloc(tamanioTexto);
	recibirMensajeCompleto(socketEnviador,instruccion->texto,tamanioTexto);

	return instruccion;
}

void deserializarInstruccion(void* buffer,tipoInstruccion* instruccion){

	void* proximo = buffer;

	size_t tamanioTexto;

	memcpy(&(instruccion->pid),proximo,sizeof(int));         	proximo+=sizeof(int);
	memcpy(&(instruccion->instruccion),proximo,sizeof(char));	proximo+=sizeof(char);
	memcpy(&(instruccion->nroPagina),proximo,sizeof(int));	 	proximo+=sizeof(int);
	memcpy(&tamanioTexto,proximo,sizeof(size_t));			 	proximo+=sizeof(size_t);
	instruccion->texto = malloc(tamanioTexto);
	memcpy(instruccion->texto,proximo,tamanioTexto);

}

void* serializarRespuesta(tipoRespuesta respuesta){

	size_t tamanioInformacion = strlen(respuesta.informacion)+sizeof(char);

	size_t tamanioBloque = tamanioInformacion+sizeof(char)+sizeof(size_t);

	void* buffer = malloc(tamanioBloque+sizeof(size_t));

	void* proximo = buffer;

	memcpy(proximo,&tamanioBloque,sizeof(size_t));          		proximo+=sizeof(size_t);
	memcpy(proximo,&(respuesta.respuesta),sizeof(char));			proximo+=sizeof(char);
	memcpy(proximo,&tamanioInformacion,sizeof(size_t));          	proximo+=sizeof(size_t);
	memcpy(proximo,respuesta.informacion,tamanioInformacion);

	return buffer;
}

void* serializarInstruccion(tipoInstruccion instruccion){

	size_t tamanioTexto = strlen(instruccion.texto)+sizeof(char);
	size_t tamanioBloque = 2*sizeof(int)+sizeof(char)+tamanioTexto+sizeof(size_t);

		void* buffer = malloc(tamanioBloque+sizeof(size_t));

		void* proximo = buffer;

		memcpy(proximo,&tamanioBloque,sizeof(size_t));         	proximo+=sizeof(size_t);
		memcpy(proximo,&(instruccion.pid),sizeof(int));			proximo+=sizeof(int);
		memcpy(proximo,&(instruccion.instruccion),sizeof(char));	proximo+=sizeof(char);
		memcpy(proximo,&(instruccion.nroPagina),sizeof(int));		proximo+=sizeof(int);
		memcpy(proximo,&tamanioTexto,sizeof(size_t));         	proximo+=sizeof(size_t);
		memcpy(proximo,instruccion.texto,tamanioTexto);

		return buffer;
}

void enviarInstruccion(int socketCliente,tipoInstruccion* instruccion){

	/*size_t tamanioBloque = 2*(sizeof(char)+sizeof(int)+sizeof(size_t))+strlen(instruccion.texto);

	void* bloque = serializarInstruccion(instruccion);

	enviarMensaje(socketCliente,bloque,tamanioBloque);

	free(bloque);*/

	printf("ya pase\n");
	size_t tamanioTexto = strlen(instruccion->texto)+sizeof(char);
	printf("ya pase\n");
	enviarMensaje(socketCliente,&(instruccion->pid),sizeof(int));
	printf("ya pase\n");
	enviarMensaje(socketCliente,&(instruccion->instruccion),sizeof(char));
	printf("ya pase\n");
	enviarMensaje(socketCliente,&(instruccion->nroPagina),sizeof(int));
	printf("ya pase\n");
	enviarMensaje(socketCliente,&tamanioTexto,sizeof(size_t));
	printf("ya pase\n");
	enviarMensaje(socketCliente,instruccion->texto,tamanioTexto);

}


void deserializarRespuesta(void* buffer,tipoRespuesta* respuesta){

	size_t tamanioInformacion;

	void* proximo = buffer;

	memcpy(&(respuesta->respuesta),proximo,sizeof(char));		proximo+=sizeof(char);
	memcpy(&tamanioInformacion,proximo,sizeof(char));			proximo+=sizeof(size_t);
	respuesta->informacion = malloc(tamanioInformacion);
	memcpy(respuesta->informacion,proximo,tamanioInformacion);

}

void enviarRespuesta(int socketCliente,tipoRespuesta* respuesta){

	/*size_t tamanioBloque = 2*(sizeof(size_t)+sizeof(char))+strlen(respuesta.informacion);

	void* buffer = serializarRespuesta(respuesta);

	enviarMensaje(socketCliente,buffer,tamanioBloque);

	free(buffer);*/

	size_t tamanioInfo = strlen(respuesta->informacion)+sizeof(char);

	enviarMensaje(socketCliente,&(respuesta->respuesta),sizeof(char));
	enviarMensaje(socketCliente,&tamanioInfo,sizeof(size_t));
	enviarMensaje(socketCliente,respuesta->informacion,tamanioInfo);
}

tipoRespuesta* recibirRespuesta(int socketEnviador){

	/*size_t tamanioBloque;

	void* buffer = recibirBloque(&tamanioBloque,socketEnviador);

	tipoRespuesta* respuesta = malloc(sizeof(tipoRespuesta));

	deserializarRespuesta(buffer,respuesta);

	free(buffer);

	return respuesta;*/

	size_t tamanioInfo;

	tipoRespuesta* respuesta = malloc(sizeof(tipoRespuesta));

	recibirMensajeCompleto(socketEnviador,&(respuesta->respuesta),sizeof(char));
	recibirMensajeCompleto(socketEnviador,&tamanioInfo,sizeof(size_t));
	respuesta->informacion = malloc(tamanioInfo);
	recibirMensajeCompleto(socketEnviador,respuesta->informacion,tamanioInfo);

	return respuesta;

}

/*******************Funciones para tipoRespuesta*************************/

tipoRespuesta* crearTipoRespuesta(char respuesta, char* informacion){//esto es al dope, no, yo lo uso en el swap

	printf("pase");
	tipoRespuesta* aux = malloc(sizeof(tipoRespuesta)+strlen(informacion));

	printf("pase");
	aux->respuesta = respuesta;

	aux->informacion = informacion;

	return aux;
}

tipoInstruccion* crearTipoInstruccion(int pid,char instruccion,int nroPagina,char* texto ){//esto es al dope, no, yo lo uso en el swap

	tipoInstruccion* aux = malloc(sizeof(tipoInstruccion)+strlen(texto));

	aux->instruccion = instruccion;

	aux->nroPagina = nroPagina;

	aux->pid = pid;

	aux->texto = texto;

	return aux;
}

void destruirTipoRespuesta(tipoRespuesta* respuesta){
	free(respuesta->informacion);
	free(respuesta);
}

void destruirTipoInstruccion(tipoInstruccion* instruccion){
	free(instruccion->texto);
	free(instruccion);
}


