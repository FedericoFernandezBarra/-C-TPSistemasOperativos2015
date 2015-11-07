#include "funcionesMemoria.h"
#include <commonsDeAsedio/estructuras.h>
#include <commons/string.h>

void destruirConfigMemoria(tipoConfigMemoria* estructuraDeConfiguracion) {
	free(estructuraDeConfiguracion->ipSWAP);
	free(estructuraDeConfiguracion->TLBHabilitada);
	free(estructuraDeConfiguracion);
}

tipoConfigMemoria* crearConfigMemoria() {
	tipoConfigMemoria* cfg = malloc(sizeof(tipoConfigMemoria));
	cfg->ipSWAP = string_new();
	cfg->TLBHabilitada = string_new();

	return cfg;
}

/*********************ESTRUCTURAS*************************/

tipoConfigMemoria* cargarArchivoDeConfiguracionDeMemoria(
		char* rutaDelArchivoDeConfiguracion) {

	t_config* archivoCfg = config_create(rutaDelArchivoDeConfiguracion);
	tipoConfigMemoria* cfg = crearConfigMemoria();

	validarErrorYAbortar(
			config_has_property(archivoCfg, PUERTO_ESCUCHA)
					&& config_has_property(archivoCfg, IP_SWAP)
					&& config_has_property(archivoCfg, PUERTO_SWAP)
					&& config_has_property(archivoCfg,
					MAXIMO_MARCOS_POR_PROCESO)
					&& config_has_property(archivoCfg, CANTIDAD_MARCOS)
					&& config_has_property(archivoCfg, TAMANIO_MARCO)
					&& config_has_property(archivoCfg, ENTRADAS_TLB)
					&& config_has_property(archivoCfg, TLB_HABILITADA)
					&& config_has_property(archivoCfg, RETARDO_MEMORIA),
			"Las claves del archivo de configuracion no coinciden con las que requiere el Administrador de Memoria.");

	cfg->puertoDeEscucha = config_get_int_value(archivoCfg, PUERTO_ESCUCHA);
	cfg->ipSWAP = string_duplicate(
			config_get_string_value(archivoCfg, IP_SWAP));
	cfg->puertoSWAP = config_get_int_value(archivoCfg, PUERTO_SWAP);
	cfg->maximoDeMarcosPorProceso = config_get_int_value(archivoCfg,
	MAXIMO_MARCOS_POR_PROCESO);
	cfg->cantidadDeMarcos = config_get_int_value(archivoCfg, CANTIDAD_MARCOS);
	cfg->tamanioDeMarco = config_get_int_value(archivoCfg, TAMANIO_MARCO);
	cfg->entradasDeTLB = config_get_int_value(archivoCfg, ENTRADAS_TLB);
	cfg->TLBHabilitada = string_duplicate(
			config_get_string_value(archivoCfg, TLB_HABILITADA));
	cfg->retardoDeMemoria = config_get_int_value(archivoCfg, RETARDO_MEMORIA);

	config_destroy(archivoCfg);

	return cfg;
}


void setearEstructuraMemoria(tipoEstructuraMemoria* datos) {

	datosMemoria = datos;

	datosMemoria->listaTablaPaginas = list_create();

	datosMemoria->listaRAM = list_create();

	datosMemoria->listaAccesosAPaginasRAM = list_create();

	if(estaHabilitadaLaTLB()){
	datosMemoria->listaTLB = list_create();
	//datosMemoria->listaAccesosAPaginasTLB = list_create();
	datosMemoria->tipoDeAlgoritmoTLB = FIFO;
	}

	datosMemoria->tipoDeAlgoritmoRAM = FIFO;//Hardcodeo algoritmo
	datosMemoria->listaAccesosAPaginasRAM = list_create();
}

void agregarAcceso(int nroPagina,int pid){

	tipoAccesosAPagina* accesoPagina = malloc(sizeof(tipoAccesosAPagina));

	accesoPagina->cantVecesAccedido = 0;

	accesoPagina->nroPagina = nroPagina;

	accesoPagina->pid = pid;

	list_add(datosMemoria->listaAccesosAPaginasRAM,accesoPagina);
}
/************************FUNCIONES********************************/

void tratarPeticion(int cpuAtendida) {

	tipoInstruccion* instruccion = recibirInstruccion(cpuAtendida);

	switch (instruccion->instruccion) {
	case INICIAR:
		reservarMemoriaParaProceso(*instruccion, cpuAtendida);
		break;

	case LEER:
		enviarPaginaPedidaACpu(*instruccion,cpuAtendida);
		break;

	case ESCRIBIR:
			escribirPagina(*instruccion,cpuAtendida);

		break;

	case FINALIZAR:
		quitarProceso(*instruccion, cpuAtendida);
		break;
	}
}

/*void tratarPeticiones() {

	int var;
	for (var = 1; var <= datosMemoria->maximoSocket; ++var) {

		if(FD_ISSET(var, datosMemoria->cpusATratar)){

			if(var==datosMemoria->socketCpus){
				int socketCpuEntrante = crearSocketParaAceptarSolicitudes(datosMemoria->socketCpus);
				FD_SET(socketCpuEntrante,datosMemoria->listaCpus);
				datosMemoria->maximoSocket = maximoEntre(datosMemoria->maximoSocket,socketCpuEntrante);
			}

			else 	tratarPeticion(var);
		}

	}
}*/

/**************INSTRUCCIONES*******************/

//////////////////
//INICIAR
/////////////////
void reservarMemoriaParaProceso(tipoInstruccion instruccion, int cpuATratar) {

	tipoRespuesta* respuesta;// = malloc(sizeof(tipoRespuesta));

	int dondeEstaTabla = buscarTabla(instruccion.pid);

	if(dondeEstaTabla<0){

	if(instruccion.nroPagina<=datosMemoria->configuracion->maximoDeMarcosPorProceso){

	if (puedoReservarEnSWAP(instruccion, &respuesta)) {


		tipoTablaPaginas* tablaDePaginasNueva = malloc(sizeof(tipoTablaPaginas));


		tablaDePaginasNueva->frames = list_create();

		tablaDePaginasNueva->paginasAsignadas = 0;

		tablaDePaginasNueva->paginasPedidas = instruccion.nroPagina;

		tablaDePaginasNueva->pid = instruccion.pid;


		list_add(datosMemoria->listaTablaPaginas, tablaDePaginasNueva);

			}
		}
		else respuesta = crearTipoRespuesta(MANQUEADO,"Cantidad de paginas excede el maximo por proceso");
	}
	else 	respuesta = crearTipoRespuesta(MANQUEADO,"Tabla de paginas de proceso ya existente");


	enviarRespuesta(cpuATratar, respuesta);
}

bool puedoReservarEnSWAP(tipoInstruccion instruccion, tipoRespuesta** respuesta) {

	return instruccionASwapRealizada(&instruccion, respuesta);
}

//////////////////
//LEER PAGINA
/////////////////

void enviarPaginaPedidaACpu(tipoInstruccion instruccion, int cpuATratar) {

	tipoRespuesta* respuesta = malloc(sizeof(tipoRespuesta));

	int posicionDePag = -1,dondeEstaTabla = buscarTabla(instruccion.pid);

	bool estaEnTLB = true;

	if(dondeEstaTabla>=0){

		if(estaHabilitadaLaTLB()){

			printf("Buscando pagina en TLB...\n");

			posicionDePag = dondeEstaEnTLB(instruccion.nroPagina,instruccion.pid);
		}


		if (posicionDePag<0) {

			printf("Pagina no encontrada en TLB..\n");

		posicionDePag = dondeEstaEnTabla(instruccion.nroPagina,instruccion.pid);

		estaEnTLB = false;
		}

		if(posicionDePag<0) {

			printf("Pagina no encontrada en RAM..\n");

			 bool traidoDesdeSwap = traerPaginaDesdeSwap(instruccion, respuesta);

		if(traidoDesdeSwap)
			 posicionDePag = dondeEstaEnTabla(instruccion.nroPagina,instruccion.pid);
		}


		if(posicionDePag>=0){

		respuesta->respuesta = PERFECTO;

		respuesta->informacion = malloc(datosMemoria->configuracion->tamanioDeMarco);

		respuesta->informacion = traerPaginaDesdeRam(posicionDePag);

		if(!estaEnTLB&&estaHabilitadaLaTLB())
			agregarPaginaATLB(instruccion.nroPagina,instruccion.pid,posicionDePag);

		aumentarAccesoAPaginaRAM(instruccion.nroPagina,instruccion.pid);

				}
		else{
			tipoInstruccion instruccionDeBorrado;

			instruccionDeBorrado.instruccion = FINALIZAR;

			instruccionDeBorrado.pid = instruccion.pid;

			instruccionDeBorrado.nroPagina = 0;

			instruccionDeBorrado.texto = "";

			tipoRespuesta* respuestaSwap;

			if(instruccionASwapRealizada(&instruccionDeBorrado,&respuestaSwap))//Aca tira un buen segmentation fault
				destruirProceso(instruccion.pid);

			respuesta->respuesta = MANQUEADO;

			respuesta->informacion = string_duplicate("Pagina no encontrada");
		}
	}

	else{
					tipoInstruccion instruccionDeBorrado;

					instruccionDeBorrado.instruccion = FINALIZAR;

					instruccionDeBorrado.pid = instruccion.pid;

					instruccionDeBorrado.nroPagina = 0;

					instruccionDeBorrado.texto = "";

					tipoRespuesta* respuestaSwap;

					if(instruccionASwapRealizada(&instruccionDeBorrado,&respuestaSwap))
					destruirProceso(instruccion.pid);

					respuesta = crearTipoRespuesta(MANQUEADO,"Tabla de paginas no existente");

			}

	enviarRespuesta(cpuATratar, respuesta);

	}

char* traerPaginaDesdeRam(int direccion){

	sleep(datosMemoria->configuracion->retardoDeMemoria);

	char* pagina = list_get(datosMemoria->listaRAM,direccion);

	return pagina;

}

int buscarTabla(int pid){

int var,posicionDeTabla = -1;

tipoTablaPaginas* tablaActual;

for (var = 0; var < list_size(datosMemoria->listaTablaPaginas); ++var) {

	tablaActual = list_get(datosMemoria->listaTablaPaginas,var);

	if(tablaActual->pid==pid){

		posicionDeTabla = var;

		break;
	}
}

return posicionDeTabla;

}

int dondeEstaEnTabla(int nroPagina, int pid) {

	sleep(datosMemoria->configuracion->retardoDeMemoria);

	int posicionDePagina = -1,posicionDeTabla = buscarTabla(pid);

	tipoTablaPaginas* tablaActual;

	tipoPagina* paginaActual;

	if(posicionDeTabla>=0){

	tablaActual = list_get(datosMemoria->listaTablaPaginas, posicionDeTabla);

		int i;
		for (i = 0; i < list_size(tablaActual->frames); ++i) {

			paginaActual = list_get(tablaActual->frames,i);

			if(paginaActual->numeroDePagina==nroPagina){

				posicionDePagina = paginaActual->posicionEnRAM;

				break;
			}

		}

		}

	return posicionDePagina;

}

int dondeEstaEnTLB(int nroPagina, int pid) {

	int var, posicionDePagina = -1;

	tipoTLB* estructuraTLBActual;

	for (var = 0; var < list_size(datosMemoria->listaTLB); ++var) {

		estructuraTLBActual = list_get(datosMemoria->listaTLB, var);

		if (estructuraTLBActual->numeroDePagina == nroPagina&& estructuraTLBActual->pid == pid) {

			posicionDePagina = estructuraTLBActual->posicionEnRAM;

			break;
		}
	}

	return posicionDePagina;
}

bool traerPaginaDesdeSwap(tipoInstruccion instruccion, tipoRespuesta* respuesta) {

	instruccion.instruccion = LEER;

	bool instruccionExitosa = instruccionASwapRealizada(&instruccion,&respuesta);

	if(instruccionExitosa)
	agregarPagina(instruccion.nroPagina,instruccion.pid,respuesta->informacion);

	return instruccionExitosa;

}

void aumentarAcceso(int nroPagina,int pid,t_list* listaAccesos){

	tipoAccesosAPagina* accesoActual;

	int var;
	for (var = 0; var < list_size(listaAccesos); ++var) {

		accesoActual = list_get(listaAccesos,var);

		if(accesoActual->pid==pid&&accesoActual->nroPagina==nroPagina){

			accesoActual->cantVecesAccedido++;

			break;
		}
	}
}

void aumentarAccesoAPaginaTLB(int nroPagina,int pid){

	if(datosMemoria->tipoDeAlgoritmoTLB==LRU)
		aumentarAcceso(nroPagina,pid,datosMemoria->listaAccesosAPaginasRAM);//aca estaba tLB ahora quedo redundante :'(
}

void aumentarAccesoAPaginaRAM(int nroPagina,int pid){

	if(datosMemoria->tipoDeAlgoritmoRAM==LRU)
		aumentarAcceso(nroPagina,pid,datosMemoria->listaAccesosAPaginasRAM);
}


void quitarPaginasDeTLB(int pid) {

	int var, posicionEnTLB;
	for (var = 0; var < cantidadDePaginasAsignadas(pid); ++var) {

		posicionEnTLB = dondeEstaEnTLB(var, pid);

		if (posicionEnTLB >= 0)
			list_remove(datosMemoria->listaTLB,posicionEnTLB);
	}

}
int cantidadDePaginasAsignadas(int pid){

	int var,cantidadDePaginasAsignadas = 0;

	tipoTablaPaginas* tablaActual;
	/*

	tipoAdministracionPaginas* instanciaAdministracionActual;

	for (var = 0; var < list_size(datosMemoria->administradorPaginas); ++var) {

	instanciaAdministracionActual = list_get(datosMemoria->administradorPaginas,var);

	if(instanciaAdministracionActual->pid==pid){

		cantidadDePaginasAsignadas = instanciaAdministracionActual->paginasAsignadas;

		break;
		}
	}*/
	for (var = 0; var < list_size(datosMemoria->listaTablaPaginas); ++var) {

		tablaActual = list_get(datosMemoria->listaTablaPaginas,var);

		if(tablaActual->pid==pid){

			cantidadDePaginasAsignadas = tablaActual->paginasAsignadas;
		}

	}

	return cantidadDePaginasAsignadas;

}

void quitarTabla(int pid) {

	int var,dondeEstaTabla = buscarTabla(pid);

	tipoTablaPaginas* tablaDeProcesoABorrar = list_get(datosMemoria->listaTablaPaginas,dondeEstaTabla);

	tipoPagina* paginaActual;

	for (var = 0; var < cantidadDePaginasAsignadas(pid); ++var) {

		paginaActual = list_get(tablaDeProcesoABorrar->frames,var);

		liberarPaginaDeRAM(dondeEstaEnTabla(paginaActual->numeroDePagina,pid));

		free(paginaActual);
	}

	free(tablaDeProcesoABorrar);

	list_remove(datosMemoria->listaTablaPaginas,dondeEstaTabla);
}

void liberarPaginaDeRAM(int posicionEnRam){

	char* pagina = list_get(datosMemoria->listaRAM,posicionEnRam);

	free(pagina);

	list_remove(datosMemoria->listaRAM,posicionEnRam);
}

//////////////////
//ESCRIBIR PAGINA
/////////////////

bool estaHabilitadaLaTLB(){

	return string_equals_ignore_case(datosMemoria->configuracion->TLBHabilitada,"SI");
}

void escribirPagina(tipoInstruccion instruccion,int cpuATratar){

	tipoRespuesta* respuesta = malloc(sizeof(tipoRespuesta));

	int posicionDePag = -1,dondeEstaTabla = buscarTabla(instruccion.pid);

	bool estaEnTLB = true;

	if(dondeEstaTabla>=0){

		tipoTablaPaginas* tablaDeProceso = list_get(datosMemoria->listaTablaPaginas,dondeEstaTabla);

	if(strlen(instruccion.texto)<datosMemoria->configuracion->tamanioDeMarco&&instruccion.nroPagina<tablaDeProceso->paginasPedidas){

		if(estaHabilitadaLaTLB())
			posicionDePag = dondeEstaEnTLB(instruccion.nroPagina,instruccion.pid);


		if (posicionDePag<0) {

		posicionDePag = dondeEstaEnTabla(instruccion.nroPagina,instruccion.pid);

		estaEnTLB = false;
		}

		if(posicionDePag<0) {

			instruccion.instruccion = LEER;

		 bool traidoDesdeSwap = traerPaginaDesdeSwap(instruccion, respuesta);

		 if(traidoDesdeSwap)
		 posicionDePag = dondeEstaEnTabla(instruccion.nroPagina,instruccion.pid);
		}

		respuesta->respuesta = PERFECTO;

		//respuesta->informacion = malloc(datosMemoria->configuracion->tamanioDeMarco);

		//memcpy(respuesta->informacion,instruccion.texto,/*datosMemoria->configuracion->tamanioDeMarco);/*/strlen(instruccion.texto));

		respuesta->informacion = string_duplicate(instruccion.texto);

		if(posicionDePag>=0){

		char* paginaAModificar; //= traerPaginaDesdeRam(posicionDePag);

		//free(paginaAModificar);

		paginaAModificar = malloc(datosMemoria->configuracion->tamanioDeMarco);

		memcpy(paginaAModificar,instruccion.texto,strlen(instruccion.texto));

		char* paginaModificada = list_replace(datosMemoria->listaRAM,posicionDePag,paginaAModificar);//Deberia liberar memoria?? ,no entiendo nada!!

		free(paginaModificada);

		modificarBitDeModificacion(instruccion.nroPagina,instruccion.pid);

		aumentarAccesoAPaginaRAM(instruccion.nroPagina,instruccion.pid);

		if(!estaEnTLB&&estaHabilitadaLaTLB())
			agregarPaginaATLB(instruccion.nroPagina,instruccion.pid,posicionDePag);

				}
		else{
			agregarPagina(instruccion.nroPagina,instruccion.pid,instruccion.texto);

			agregarAcceso(instruccion.nroPagina,instruccion.pid);
		}

		}

	else{

		tipoInstruccion instruccionDeBorrado;

		instruccionDeBorrado.instruccion = FINALIZAR;//Charlar conForronan porque es un error

		instruccionDeBorrado.pid = instruccion.pid;

		instruccionDeBorrado.nroPagina = 0;

		instruccionDeBorrado.texto = "";

		tipoRespuesta* respuestaSwap;

		if(instruccionASwapRealizada(&instruccionDeBorrado,&respuestaSwap))
		destruirProceso(instruccion.pid);

		respuesta->respuesta = MANQUEADO;

		if(instruccion.nroPagina>tablaDeProceso->paginasPedidas)
		respuesta->informacion = "Tamaño de pagina demasiado grande";

		else
			respuesta->informacion = "Numero de pagina demasiado grande";

		}
	}

				else{
					///Aca si no existe la tabla en
					//RAM no deberia de existir en SWAP

					//destruirProceso(instruccion.pid);//ESTO HAY QUE DESCOMENTARLO PERO TIRA ERROR!

					respuesta->respuesta = MANQUEADO;

					respuesta->informacion = "Tabla de paginas no existente";
				}

	enviarRespuesta(cpuATratar, respuesta);
	}

void modificarBitDeModificacion(int nroPagina,int pid){

	int dondeEstaTabla = buscarTabla(pid),var;

	tipoTablaPaginas* instanciaTabla = list_get(datosMemoria->listaTablaPaginas,dondeEstaTabla);

	tipoPagina* paginaActual;

	for (var = 0; var < list_size(instanciaTabla->frames); ++var) {

		paginaActual = list_get(instanciaTabla->frames,var);

		if(paginaActual->numeroDePagina==nroPagina){

			paginaActual->modificado = true;//!paginaActual->modificado;

			break;
		}

	}

}


////////////////////
//FINALIZAR PROCESO
////////////////////

bool instruccionASwapRealizada(tipoInstruccion* instruccion,tipoRespuesta** respuesta) {

	enviarInstruccion(datosMemoria->socketSWAP, instruccion);

	*respuesta = recibirRespuesta(datosMemoria->socketSWAP);

	printf("recibi respuesta de swap\n");

	printf("el estado de respuesta es %c\n",(*respuesta)->respuesta);

	printf("La info de respuesta es: %s\n",(*respuesta)->informacion);

	if((*respuesta)->respuesta==NULL)
		printf("No se puede leer estado de respuesta\n");

	return ((*respuesta)->respuesta == PERFECTO);
}

void quitarProceso(tipoInstruccion instruccion, int cpuaATratar) {

	tipoRespuesta* respuesta;

	if (instruccionASwapRealizada(&instruccion, &respuesta)) {

		destruirProceso(instruccion.pid);
	}

	enviarRespuesta(cpuaATratar,respuesta);

}

void destruirProceso(int pid) {

	quitarTabla(pid);

	if(estaHabilitadaLaTLB())
	quitarPaginasDeTLB(pid);

	//quitarAdministracionDePaginas(pid);
}


/*void quitarAdministracionDePaginas(int pid){

	int var;

	tipoAdministracionPaginas* instanciaAdministracionPagina;

	for (var = 0; var < list_size(datosMemoria->administradorPaginas); ++var) {

		instanciaAdministracionPagina = list_get(datosMemoria->administradorPaginas,var);

		if(instanciaAdministracionPagina->pid==pid){
			list_remove(datosMemoria->administradorPaginas,var);
			break;
		}
	}
}*/

//**************************************************************************************************************
//**********************************FUNCIONES DE REEMPLAZO******************************************************
//**************************************************************************************************************

void agregarPaginaATLB(int nroPagina,int pid,int posicionEnRam){

	if(TLBLlena()){

		int posicionAReemplazar = cualReemplazarTLB();

		list_remove(datosMemoria->listaTLB,posicionAReemplazar);
	}

	tipoTLB* instanciaTLB = malloc(sizeof(tipoTLB));

	instanciaTLB->numeroDePagina = nroPagina;

	instanciaTLB->pid = pid;

	instanciaTLB->posicionEnRAM = posicionEnRam;

	list_add(datosMemoria->listaTLB,instanciaTLB);

	printf("Pagina agregada a tlb con:\n");

	printf("pid:%d\n",instanciaTLB->pid);
	printf("nroPagina:%d\n",instanciaTLB->numeroDePagina);
	printf("posicionRAM:%d\n",instanciaTLB->posicionEnRAM);

}

void agregarPaginaATabla(int nroPagina,int pid,int posicionEnRam){

	int dondeEstaTabla = buscarTabla(pid);

	tipoTablaPaginas* tablaDeProceso = list_get(datosMemoria->listaTablaPaginas,dondeEstaTabla);

	tipoPagina* instanciaPagina = malloc(sizeof(tipoPagina));

	instanciaPagina->modificado = false;

	instanciaPagina->numeroDePagina =  nroPagina;

	instanciaPagina->posicionEnRAM = posicionEnRam;

	list_add(tablaDeProceso->frames,instanciaPagina);
}


bool agregarPagina(int nroPagina,int pid,char* pagina){

	printf("Entrando a agregar pagina...\n");

	bool operacionExitosa = true;

	int posicionEnRam;

	if(RAMLlena()){

			int posicionAReemplazar = cualReemplazarRAM();

			printf("Encontre que pagina reemplazar..\n");

			tipoAccesosAPagina* instanciaAccesoAReemplazar =  list_get(datosMemoria->listaAccesosAPaginasRAM,posicionAReemplazar);

			printf("La pagina a reemplazar es la pag %d del proceso %d...\n",instanciaAccesoAReemplazar->nroPagina,instanciaAccesoAReemplazar->pid);
			/*tipoInstruccion* instruccion = malloc(sizeof(tipoInstruccion));

			instruccion->nroPagina = instanciaAccesoAReemplzar->nroPagina;

			instruccion->pid = instanciaAccesoAReemplzar->pid;

			instruccion->instruccion = ESCRIBIR;//Esto hay que charlarlo con Forronan

			instruccion->texto = malloc(datosMemoria->configuracion->tamanioDeMarco);

			//memcpy(instruccion->texto,)

			tipoRespuesta* respuestaSwap;

			operacionExitosa = instruccionASwapRealizada(instruccion,respuestaSwap);*/

			//if(operacionExitosa){
			if(estaHabilitadaLaTLB()){
				posicionEnRam = dondeEstaEnTLB(instanciaAccesoAReemplazar->nroPagina,instanciaAccesoAReemplazar->pid);

				if(posicionEnRam>=0)
					quitarPaginaDeTLB(instanciaAccesoAReemplazar->nroPagina,instanciaAccesoAReemplazar->pid);

				printf("Quitando pagina de TLB...\n");
			}



			else {
				posicionEnRam = dondeEstaEnTabla(instanciaAccesoAReemplazar->nroPagina,instanciaAccesoAReemplazar->pid);
			}


			quitarPaginaDeTabla(instanciaAccesoAReemplazar->nroPagina, instanciaAccesoAReemplazar->pid);

			printf("Quitando pagina de Tabla..\n");

			quitarAcceso(nroPagina,pid);

			printf("Acceso quitado..\n");

			tipoInstruccion* instruccion = malloc(sizeof(tipoInstruccion));

			printf("Haciendo malloc de insyruccion\n");

			instruccion->nroPagina = instanciaAccesoAReemplazar->nroPagina;

			instruccion->pid = instanciaAccesoAReemplazar->pid;

			instruccion->instruccion = ESCRIBIR;//Esto hay que charlarlo con Forronan

			instruccion->texto = malloc(datosMemoria->configuracion->tamanioDeMarco);

			printf("Haciendo malloc de marco\n");

			char* paginaVieja = (char*)list_get(datosMemoria->listaRAM,posicionEnRam);

			printf("La pagina tiene:%s\n",paginaVieja);

			instruccion->texto = string_duplicate(paginaVieja);//memcpy(instruccion->texto,paginaVieja,datosMemoria->configuracion->tamanioDeMarco);

			printf("HAciendo memcpy de pagina..\n");

			tipoRespuesta* respuestaSwap;

			printf("Mandando solicitud de escritura a SWAP...\n");

			operacionExitosa = instruccionASwapRealizada(instruccion,&respuestaSwap);

			if(operacionExitosa){

				agregarPaginaATabla(nroPagina,pid,posicionEnRam);

				list_replace_and_destroy_element(datosMemoria->listaRAM,posicionEnRam,pagina,free);

				if(estaHabilitadaLaTLB())
					agregarPaginaATLB(nroPagina,pid,posicionEnRam);

				agregarAcceso(nroPagina,pid);
			}

			else {
				//error en swap de guardado de pagina!!
			}

	}
	else{

		list_add(datosMemoria->listaRAM,pagina);

		posicionEnRam = list_size(datosMemoria->listaRAM)-1;

		agregarPaginaATabla(nroPagina,pid,posicionEnRam);

		if(estaHabilitadaLaTLB())
			agregarPaginaATLB(nroPagina,pid,posicionEnRam);
	}

	return operacionExitosa;
}


void quitarPaginaDeTLB(int nroPagina,int pid){

	int var;

	tipoTLB* instanciaTLB;

	for (var = 0; var < list_size(datosMemoria->listaTLB); ++var) {

		instanciaTLB= list_get(datosMemoria->listaTLB,var);

		if(instanciaTLB->numeroDePagina==nroPagina&&instanciaTLB->pid==pid){

			list_remove(datosMemoria->listaTLB,var);

			break;
		}
	}

}

void quitarAcceso(int nroPagina,int pid){

	tipoAccesosAPagina* accesoActual;

	int var;

	for (var = 0; var < list_size(datosMemoria->listaAccesosAPaginasRAM); ++var) {

		accesoActual = list_get(datosMemoria->listaAccesosAPaginasRAM,var);

		if(accesoActual->nroPagina==nroPagina&&accesoActual->pid==pid){

			list_remove(datosMemoria->listaAccesosAPaginasRAM,var);

			break;

		}

	}

}

void limpiarTLB(){
	int var;
	for (var = 0; var < list_size(datosMemoria->listaTLB); ++var) {
		list_remove(datosMemoria->listaTLB,0);//var);
	}
}

void limpiarRam(){
	int var;
	for (var = 0; var < list_size(datosMemoria->listaRAM); ++var) {//aca decia algo de modificar bits en tabla de paginas
		liberarPaginaDeRAM(var);
	}
}

void volcarRamALog(){
//Aca decia que hay que usar fork, pero me parece quilombo para nada
}

void limpiarTabla(){

	int var;

	/*tipoTablaPaginas* instanciaTabla;

	for (var = list_size(datosMemoria->listaTablaPaginas); var >=0; --var) {

		instanciaTabla = list_get(datosMemoria->listaTablaPaginas,var);

		quitarTabla(instanciaTabla->pid);
	}*/

	for (var = 0; var < list_size(datosMemoria->listaTablaPaginas); ++var) {

		list_remove(datosMemoria->listaTablaPaginas,0);

	}

}

void limpiarListaAccesos(){
	int var;
	for (var = list_size(datosMemoria->listaAccesosAPaginasRAM); var>0; ++var) {
		list_remove(datosMemoria->listaAccesosAPaginasRAM,var-1);
	}
}

void quitarPaginaDeTabla(int nroPagina,int pid){

	int var;

	int dondeEstaTabla = buscarTabla(pid);

	tipoTablaPaginas* instanciaTabla = list_get(datosMemoria->listaTablaPaginas,dondeEstaTabla);

	tipoPagina* instanciaPagina;

	for (var = 0; var < list_size(instanciaTabla->frames); ++var) {

		instanciaPagina = list_get(instanciaTabla->frames,var);

		if(instanciaPagina->numeroDePagina==nroPagina){

			list_remove(instanciaTabla->frames,var);

			break;
		}
	}

}


int cualReemplazarRAM(){

	int cualReemplazar;

	switch (datosMemoria->tipoDeAlgoritmoRAM) {

	case FIFO:
		cualReemplazar = cualReemplazarRAMFIFO();
			break;

	case LRU:
		cualReemplazar = cualReemplazarRAMLRU();
			break;
	}

	return cualReemplazar;
}

bool RAMLlena(){

	return (list_size(datosMemoria->listaRAM)>=datosMemoria->configuracion->cantidadDeMarcos);
}

bool TLBLlena(){

	return list_size(datosMemoria->listaTLB)>=datosMemoria->configuracion->entradasDeTLB;
}

int cualReemplazarRAMFIFO(){
	return 0;
}

int cualReemplazarRAMLRU(){

	return cualReemplazarPorLRU(datosMemoria->listaAccesosAPaginasRAM);

}

int cualReemplazarTLBFIFO(){

	int cualReemplazar,var;

	tipoAccesosAPagina* accesoElegido;

	for (var = 0; var < list_size(datosMemoria->listaAccesosAPaginasRAM); ++var) {

		accesoElegido = list_get(datosMemoria->listaAccesosAPaginasRAM,var);

		cualReemplazar = dondeEstaEnTLB(accesoElegido->nroPagina,accesoElegido->pid);

		if(cualReemplazar>=0)
			break;
	}

	return var;//cualReemplazar;
}

int cualReemplazarTLBLRU(){

	int cualReemplazar = cualReemplazarPorLRU(datosMemoria->listaAccesosAPaginasRAM);

	//tipoAccesosAPagina* accesoElegido = list_get(datosMemoria->listaAccesosAPaginasRAM,cualReemplazar);

	//cualReemplazar = dondeEstaEnTLB(accesoElegido->nroPagina,accesoElegido->pid);

	return cualReemplazar;
}

int cualReemplazarTLB(){

	int cualReemplazar;

switch (datosMemoria->tipoDeAlgoritmoTLB) {

	case FIFO:
		cualReemplazar = cualReemplazarTLBFIFO();
		break;

	case LRU:
		cualReemplazar = cualReemplazarTLBLRU();
		break;
}

	return cualReemplazar;
}

/*int cualReemplazarPorFIFO(t_list* listaAEvaluar){

	return 0;
}*/

int cualReemplazarPorLRU(t_list* listaAEvaluar){

	int var;

	tipoAccesosAPagina* instanciaAccesoActual;

	tipoAccesosAPagina* instanciaAccesoLRU = list_get(listaAEvaluar,0);

	int posicionLRU = 0;

	for (var = 1; var < list_size(listaAEvaluar); ++var) {

		instanciaAccesoActual = list_get(listaAEvaluar,var);

		if(instanciaAccesoActual->cantVecesAccedido<instanciaAccesoLRU->cantVecesAccedido){

			instanciaAccesoLRU = instanciaAccesoActual;

			posicionLRU = var;
		}
	}

	return posicionLRU;
}
