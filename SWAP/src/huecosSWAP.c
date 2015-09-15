/*
 * huecosSWAP.c
 *
 *  Created on: 13/9/2015
 *      Author: utnso
 */


#include "estructurasSWAP.h"
#include "huecosSWAP.h"
#include <commons/collections/list.h>

/////////////////////LISTAS DE HUECOS///////////////////////
tipoHuecoLibre* crearHuecoLibre(int inicio,int cantidadDePaginas){

	tipoHuecoLibre* aux = malloc(sizeof(tipoHuecoLibre));
	aux->baseDeMProc = inicio;
	aux->cantidadDePaginasQueOcupa = cantidadDePaginas;

	return aux;
}

void destruirHuecoLibre(tipoHuecoLibre* huecoLibre){
	free(huecoLibre);
}

t_list* inicializarListaDeHuecosLibres(int cantidadDePaginas){

	t_list*	listaDeHuecosLibres = list_create();
	list_add(listaDeHuecosLibres,crearHuecoLibre(0,cantidadDePaginas));
	return listaDeHuecosLibres;
}



tipoHuecoUtilizado* crearHuecoUtilizado(int pidProceso,int inicio,int cantidadDePaginas){

	tipoHuecoUtilizado* aux = malloc(sizeof(tipoHuecoUtilizado));
	aux->pid = pidProceso;
	aux->baseDeMProc = inicio;
	aux->cantidadDePaginasQueOcupa = cantidadDePaginas;

	return aux;
}

void destruirHuecoUtilizado(tipoHuecoUtilizado* huecoUtilizado){
	free(huecoUtilizado);
}

t_list* inicializarListaDeHuecosUtilizados(){

	t_list*	listaDeHuecosUtilizados = list_create();
	return listaDeHuecosUtilizados;
}


/////////////////////FUNCIONES PRINCIPALES//////////////
int reservarEspacio(t_list* listaDeHuecosUtilizados,int pidProceso,int cantDePaginasSolicitadas,tipoConfigSWAP* configuracion){
	int indiceDeHuecoPrevio;

	//if(tengoEspacioDisponible)
	if (tengoEspacioDisponible(listaDeHuecosUtilizados,configuracion->cantidadDePaginas)) {
		//	if(tengoEspacioContiguo)
		if ((indiceDeHuecoPrevio = tengoEspacioContiguoDisponible(listaDeHuecosUtilizados,cantDePaginasSolicitadas)) == 0) {
			//compactar
			compactar();

		}
		//	asignarEspacio
		asignarEspacio(listaDeHuecosUtilizados,pidProceso,cantDePaginasSolicitadas,indiceDeHuecoPrevio);
		return 1;
	}
	//else

	//	error

	return 0;
}

void liberarEspacio(t_list* listaDeHuecosUtilizados,int pidProceso){
	//buscarHuecoDePIDyBorrarHuecoDeLista
	tipoHuecoUtilizado* aux;
	int i;

	for (i = 0; i < list_size(listaDeHuecosUtilizados); ++i) {
		aux = list_get(listaDeHuecosUtilizados,i);
		if (aux->pid == pidProceso) {
			list_remove(listaDeHuecosUtilizados,i);
		}
	}

}

char* leerPagina(t_list* listaDeHuecosUtilizados,int pidProceso,int dirLogicaDePagina,int tamanioDePagina,FILE* particion){
	char* contenidoDePagina;

	//buscarPIDEnListaDeHuecos
	tipoHuecoUtilizado* huecoDelProceso = buscarHuecoDePID(listaDeHuecosUtilizados,pidProceso);

	//traducirDireccionLogicaEnFisica
	int direccionFisicaEnParticion = traducirDireccionLogicaAFisica(huecoDelProceso,dirLogicaDePagina);
		//direccionFisicaEnParticion es la posicion donde comienza la pagina en el archivo de particion

	//buscarEnContenidoEnParticion
	contenidoDePagina = string_duplicate(leerBloqueMapeado(particion,direccionFisicaEnParticion,tamanioDePagina));

	//retornarContenidoDePagina
	return contenidoDePagina;
}

void escribirPagina(t_list* listaDeHuecosUtilizados,int pidProceso,char* contenidoAEscribir,int dirLogicaDePagina,int tamanioDePagina, FILE* particion){
	//buscarPIDEnListaDeHuecos
	tipoHuecoUtilizado* huecoDelProceso = buscarHuecoDePID(listaDeHuecosUtilizados,pidProceso);

	//traducirDireccionLogicaAFisica
	int direccionFisicaEnParticion = traducirDireccionLogicaAFisica(huecoDelProceso,dirLogicaDePagina);

	//escribirEnParticion
	escribirBloqueMapeado(particion,contenidoAEscribir,direccionFisicaEnParticion,tamanioDePagina);
}




/////////////////FUNCIONES SECUNDARIAS//////////////
int tengoEspacioDisponible(t_list* listaDeHuecosUtilizados,int cantDePaginasTotal){
	if (paginasDisponibles(listaDeHuecosUtilizados,cantDePaginasTotal) > 0) {
		return SI;
	} else {
		return NO;
	}
}//OK

int tengoEspacioContiguoDisponible(t_list* listaDeHuecosUtilizados,int cantDePaginasSolicitadas){

	int resultado = 0;
	tipoHuecoUtilizado* aux1;
	tipoHuecoUtilizado* aux2;

	int i=0;
	while(i<list_size(listaDeHuecosUtilizados) && resultado==0){

		aux1 = list_get(listaDeHuecosUtilizados,i);
		aux2 = list_get(listaDeHuecosUtilizados,i+1);

		if (cantDePaginasSolicitadas <= espacioEntreDosHuecosUtilizados(aux1,aux2)) {
			resultado = i;
		}

		i++;
	}

	return resultado;
}

int paginasDisponibles(t_list* listaDeHuecosUtilizados,int cantDePaginasTotal){
	int pagDisponibles = 0;
	int i;
	tipoHuecoUtilizado* aux1;
	tipoHuecoUtilizado* aux2;
	if (list_is_empty(listaDeHuecosUtilizados))
		return cantDePaginasTotal;

	for (i = 0; i < list_size(listaDeHuecosUtilizados); ++i) {
		if (cantDePaginasTotal != i) {

			aux1 = list_get(listaDeHuecosUtilizados,i);
			aux2 = list_get(listaDeHuecosUtilizados,i+1);

			pagDisponibles += espacioEntreDosHuecosUtilizados(aux1,aux2);
		}
	}
	return pagDisponibles;
}//OK

int espacioDisponible(t_list* listaDeHuecosLibres,int tamanioDePagina){
	int espacioDisponible = 0;
	int i;
	tipoHuecoLibre* aux;

	for (i = 0; i < list_size(listaDeHuecosLibres); i++) {
		aux = list_get(listaDeHuecosLibres,i);
		espacioDisponible+= (tamanioDePagina*(aux->cantidadDePaginasQueOcupa));
	}

	return espacioDisponible;
}//OK

void compactar(){
}

void asignarEspacio(t_list* listaDeHuecosUtilizados,int pidProceso,int cantDePaginasSolicitadas,int indiceHuecoPrevio){
	//crearEstructuraOcupado
	tipoHuecoUtilizado* aux = list_get(listaDeHuecosUtilizados,indiceHuecoPrevio);
	tipoHuecoUtilizado* huecoNuevo = crearHuecoUtilizado(pidProceso,paginaMaxima(aux),cantDePaginasSolicitadas);

	//agregarAListaOcupados
	list_add_in_index(listaDeHuecosUtilizados,indiceHuecoPrevio+1,huecoNuevo);

}



///////FUNCIONES AUXILIARES/////
int espacioEntreDosHuecosUtilizados(tipoHuecoUtilizado* h1, tipoHuecoUtilizado* h2){
	return (h2->baseDeMProc - (h1->baseDeMProc + h1->cantidadDePaginasQueOcupa));
}

int paginaMaxima(tipoHuecoUtilizado* hueco){
	return (hueco->baseDeMProc + hueco->cantidadDePaginasQueOcupa);
}

tipoHuecoUtilizado* buscarHuecoDePID(t_list* listaDeHuecosUtilizados,int pidProcesoBuscado){
	tipoHuecoUtilizado* aux;
	int i;

	for (i = 0; i < list_size(listaDeHuecosUtilizados); ++i) {
		aux = list_get(listaDeHuecosUtilizados,i);
		if(aux->pid == pidProcesoBuscado)
			break;
	}
	return aux;
}

int traducirDireccionLogicaAFisica(tipoHuecoUtilizado* hueco,int dirLogicaDePagina){
	int direccionFisica;

	direccionFisica = hueco->baseDeMProc + dirLogicaDePagina;

	return direccionFisica;
}



void imprimirListaDeHuecos(t_list* lista){
	int i;

	for (i = 0; i < list_size(lista); ++i) {
		imprimirHueco(list_get(lista,i));
	}
}

void imprimirHueco(tipoHuecoUtilizado* hueco){

	printf("PID: %d   paginaInicial: %d cantidadDePaginas: %d\n",hueco->pid,hueco->baseDeMProc,hueco->cantidadDePaginasQueOcupa);

}
