#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "pthread.h" //Biblioteca de hilos
#include "utiles.h"  //Biblioteca de funciones utiles
#include <sys/inotify.h>
#include <commons/config.h> //Biblioteca de la catedra
#include "commons/temporal.h" //Biblioteca de la catedra
#include <unistd.h>
#include "semaphore.h"//Biblioteca de semaforos
#include "parser/metadata_program.h"

#define BUFF_SIZE 1000 //Tamaño del buffer de datos que se transfieren entre procesos

#define ENVIO_PROGRAMA 1
#define PROGRAMA_TERMINADO 2
#define PROGRAMA_BLOQUEADO 3
#define PROGRAMA_ABORTADO 4
#define RESERVAR_MEMORIA_PCB 5

//PCB: Estructura que se usa para guardar la informacion de un programa
typedef struct _pcb
{
    int id;
    char *segmentoCodigo;
    char *segmentoStack;
    char *cursorStack;
    t_intructions* indiceCodigo;
    char* indiceEtiquetas;
    t_puntero_instruccion programCounter;
    int tamContextoActual;
    int tamIndiceEtiquetas;
    int tamIndiceCodigo;
    char bloqueado[10];
    int unidadBloqueo;
    int peso;
    struct _PCB * pNext;
}_PCB;
typedef _PCB * nodoPCB;
nodoPCB programasNew;
nodoPCB programasReady;
nodoPCB programasExec;
nodoPCB programasBlock;
nodoPCB programasEnd;
nodoPCB listaAuxiliarProgramas, listaAntProgramas;

//Estructura que se usa para guardar la informacion de las cpu
typedef struct _cpu
{
    int id;
    int numSocket;
    int disponible;
    nodoPCB programaEjecutando;
    struct _CPU * pNext;
}_CPU;
typedef _CPU * nodoCPU;
nodoCPU listaCPU, listaAuxiliarCPU;

//Estructura que se usa para guardar la informacion de los hilos E/S
typedef struct _hiloIO
{
    char id[15];
    int retardo;
    nodoPCB programaEjecutando;
    struct _CPU * pNext;
}_HILOIO;
typedef _HILOIO * nodoHiloIO;
nodoHiloIO listaHilosIO, listaAuxHilosIO;

//Definicion de funciones
void *PLP(void *);
void *PCP(void *);
void levantarArchivoKernel();
void inicializarListas();
nodoPCB enviarListaProgramas(nodoPCB PCB,char tipoLista[]);
void agregarCPU(int id,int numSocket);
void borrarCPU(int numSocket);
nodoPCB armarPCB(char programaRecibido[]);
void agregarHiloIO(nodoHiloIO hIO);
void *entradaSalida(void *id);
int conectarUMV();
int enviarPCB_UMV(nodoPCB pcb);

//Variables globales
char *string, **id_hio, ipKernel[20], ipUMV[20], direccion[100];
char **retardo_hio, **semaforos, **valor_semaforos;
int puertoCpu, puertoProg, puertoUMV, multiProg, quantum, retardo;
int contador, tamStack, idProg, idCPU, indiceHiloIO;
pthread_t hiloPLP,hiloPCP,hiloEntradaSalida[10];
nodoPCB programa;
nodoHiloIO hiloIO;
Msg mensaje;
pthread_mutex_t semHiloIO, semPrograma, semCPU;
sem_t semIO;
int socketPrograma[30],socketUMV;
t_medatada_program *datos;

//INICIO
int main(int argc, char* argv[]) {
	//Se lee la direccion del archivo y se lo asigna a la variable direccion
	strcpy(direccion,argv[2]);
	string=(char*)malloc(sizeof(char)*100);//Reservo memoria
	id_hio=(char**)malloc(sizeof(char)*100);//Reservo memoria
	retardo_hio=(char**)malloc(sizeof(char)*100);//Reservo memoria
	semaforos=(char**)malloc(sizeof(char)*100);//Reservo memoria
	valor_semaforos=(char**)malloc(sizeof(char)*100);//Reservo memoria
	datos=malloc(sizeof(t_medatada_program));
	pthread_mutex_init (&semHiloIO, NULL);
	pthread_mutex_init (&semPrograma, NULL);
	pthread_mutex_init (&semCPU, NULL);
	sem_init(&semIO,0,0);
	inicializarListas();
	levantarArchivoKernel();//Levanto el archivo kernel
	pthread_create(&hiloPLP,NULL,PLP,NULL);//Creo el hilo para recibir programas
	pthread_create(&hiloPCP,NULL,PCP,NULL);//Creo el hilo para recibir CPU
	pthread_join(hiloPLP,NULL);
	pthread_join(hiloPCP,NULL);
	return 0;
}

//Levanta el archivo de configuracion
void levantarArchivoKernel(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"ip_kernel");
	strcpy(ipKernel,string);
	puertoCpu=config_get_int_value(archivoConf,"puerto_cpu");
	puertoProg=config_get_int_value(archivoConf,"puerto_programa");
	string=config_get_string_value(archivoConf,"ip_umv");
	strcpy(ipUMV,string);
	puertoUMV=config_get_int_value(archivoConf,"puerto_umv");
	multiProg=config_get_int_value(archivoConf,"multiprogramacion");
	quantum=config_get_int_value(archivoConf,"quantum");
	retardo=config_get_int_value(archivoConf,"retardo");
	id_hio=config_get_array_value(archivoConf,"id_hio");
	retardo_hio=config_get_array_value(archivoConf,"retardo_hio");
	semaforos=config_get_array_value(archivoConf,"semaforos");
	valor_semaforos=config_get_array_value(archivoConf,"valor_semaforos");
	tamStack=config_get_int_value(archivoConf,"tam_stack");
	conectarUMV();
	while(*id_hio!=NULL){
	hiloIO= malloc(sizeof(_HILOIO));//Creo un nodo
	strcpy(hiloIO->id,*id_hio);
	hiloIO->retardo=atoi(*retardo_hio);
	hiloIO->programaEjecutando=NULL;
	hiloIO->pNext = NULL;
	pthread_mutex_lock (&semHiloIO);
	agregarHiloIO(hiloIO);
	pthread_mutex_unlock (&semHiloIO);
	pthread_create(&hiloEntradaSalida[indiceHiloIO++],NULL,entradaSalida,*id_hio);
	id_hio++;
	retardo_hio++;}
	config_destroy(archivoConf);
}

void *PLP(void *var){
	//Variables de comunicacion
		int socketEscuchaProgramas;
	    char buffer[BUFF_SIZE];
		struct sockaddr_in socketInfo;
		int nbytesRecibidos;
		int optval = 1;

	//Preparo el socket para recibir conexiones
		if ((socketEscuchaProgramas = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return EXIT_FAILURE;
		}

		setsockopt(socketEscuchaProgramas, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		socketInfo.sin_family = AF_INET;
		socketInfo.sin_addr.s_addr = INADDR_ANY;//Mi ip
		socketInfo.sin_port = htons(puertoProg);//Mi puerto

		if (bind(socketEscuchaProgramas, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
			perror("Error al bindear socket escucha");
			return EXIT_FAILURE;
		}

		//Preparado para escuchar conexiones
		if (listen(socketEscuchaProgramas, 10) != 0) {
			perror("Error al poner a escuchar socket");
			return EXIT_FAILURE;

		}
		printf("Recibiendo programas\n");

		while(1){
		    //Acepta una conexion
			if ((socketPrograma[idProg] = accept(socketEscuchaProgramas, NULL, 0)) < 0) {
				perror("Error al aceptar conexion entrante");
				return EXIT_FAILURE;
		}

		    //Recibe el buffer enviado
			if ((nbytesRecibidos = recv(socketPrograma[idProg], buffer, BUFF_SIZE,0))> 0) {
		        buffer[nbytesRecibidos]='\0';
		        printf("Se ha recibido un programa para enviar a CPU\n");
		        pthread_mutex_lock (&semPrograma);
		        programa=armarPCB(buffer);
		        enviarPCB_UMV(programa);
		        if(multiProg>0){
		        	programasReady=enviarListaProgramas(programa,"Ready");
		        	multiProg--;}
		        else
		        	programasNew=enviarListaProgramas(programa,"New");
		        pthread_mutex_unlock (&semPrograma);
			}
		}
}
void *PCP(void *var){
       //Variables de comunicacion
       int socketEscuchaCPU, socketCPU[100];
       char buffer[BUFF_SIZE];
       struct sockaddr_in socketInfo;
       int nbytesRecibidos;
       int optval = 1;
       int nroSocketCPU=0;

       //Variables de Select
       fd_set descriptoresLectura;
       struct timeval esperaSelect;

       //Variables
       int i,j;

       //Preparo el socket para recibir conexiones
		if ((socketEscuchaCPU = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return EXIT_FAILURE;
		}

		setsockopt(socketEscuchaCPU, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		socketInfo.sin_family = AF_INET;
		socketInfo.sin_addr.s_addr = INADDR_ANY;//Mi ip
		socketInfo.sin_port = htons(puertoCpu);//Mi puerto

		if (bind(socketEscuchaCPU, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
			perror("Error al bindear socket escucha");
			return EXIT_FAILURE;
		}

		//Preparado para escuchar conexiones
		if (listen(socketEscuchaCPU, 10) != 0) {
			perror("Error al poner a escuchar socket");
			return EXIT_FAILURE;

		}
		printf("Recibiendo CPU\n");

		while(1){
		pthread_mutex_lock (&semCPU);
		pthread_mutex_lock (&semPrograma);
		//Busco si hay programas en la cola de Ready y si hay CPU para ejecutarlos
		listaAuxiliarCPU=listaCPU;
		while((programasReady!=NULL)&&(listaAuxiliarCPU!=NULL)){
		  if(listaAuxiliarCPU->disponible==1){
			 listaAuxiliarCPU->disponible=0;
			 listaAuxiliarCPU->programaEjecutando=programasReady;
			 programasReady=programasReady->pNext;
			 mensaje.id=ENVIO_PROGRAMA;
			 strcpy(mensaje.payload,listaAuxiliarCPU->programaEjecutando->segmentoCodigo);
			 empaquetarMensaje(mensaje,buffer);

			    //Le mando el programa a la CPU
		  		if (send(socketCPU[listaAuxiliarCPU->numSocket], buffer, strlen(buffer), 0) >= 0)
		  		   printf("Programa %d enviado a la CPU %d\n",listaAuxiliarCPU->programaEjecutando->id,
		  				 listaAuxiliarCPU->id);
		  		else
		  		   perror("Error al enviar datos. CPU no encontrado.\n");}
		listaAuxiliarCPU=listaAuxiliarCPU->pNext;}
		pthread_mutex_unlock (&semPrograma);
		pthread_mutex_unlock (&semCPU);

		//Explicacion del select: http://www.chuidiang.com/clinux/sockets/socketselect.php
		FD_ZERO (&descriptoresLectura);
		FD_SET (socketEscuchaCPU, &descriptoresLectura);
		for (i=0; i<idCPU; i++)
		    FD_SET (socketCPU[i], &descriptoresLectura);

		esperaSelect.tv_sec=0; //Espera para el select en segundos
		esperaSelect.tv_usec=retardo;//Espera para el select en milisegundos

		select (100+1, &descriptoresLectura, NULL, NULL, &esperaSelect);

		//Se trata los socket de las CPU
		for (i=0; i<idCPU; i++)
		{
		    if (FD_ISSET (socketCPU[i], &descriptoresLectura))
		    {
		        if ((nbytesRecibidos = recv(socketCPU[i], buffer, BUFF_SIZE,0))> 0)
		        {
		          mensaje=desempaquetarMensaje(buffer);
		          pthread_mutex_lock (&semCPU);
		          pthread_mutex_lock (&semPrograma);
		          //Busco el nodo de CPU que me envio el programa
		          listaAuxiliarCPU=listaCPU;
		          while(listaAuxiliarCPU!=NULL){
		        	if(listaAuxiliarCPU->numSocket==i)
		        		break;
		          listaAuxiliarCPU=listaAuxiliarCPU->pNext;}

		          printf("Programa %d termino de ejecutar\n",listaAuxiliarCPU->programaEjecutando->id);
		           //Dependiendo del id respondo de una manera distinta
		           switch(mensaje.id){
		            case ENVIO_PROGRAMA:{
		        	    programasReady=enviarListaProgramas(listaAuxiliarCPU->programaEjecutando,"Ready");
		        	    listaAuxiliarCPU->programaEjecutando=NULL;
		        	    listaAuxiliarCPU->disponible=1;
		        	    break;
		            }
		            case PROGRAMA_TERMINADO:{
		        	   programasEnd=enviarListaProgramas(listaAuxiliarCPU->programaEjecutando,"End");
		        	   mensaje.id=PROGRAMA_TERMINADO;
		        	   empaquetarMensaje(mensaje,buffer);
		        	   //Le aviso al programa que la ejecucion fue terminada
		        	   if (send(socketPrograma[listaAuxiliarCPU->programaEjecutando->id], buffer, strlen(buffer), 0) >= 0)
		        	      printf("Programa %d terminado\n",listaAuxiliarCPU->programaEjecutando->id);
		        	   else
		        	      perror("Error al enviar datos. Programa no encontrado.\n");
		        	   listaAuxiliarCPU->programaEjecutando=NULL;
		        	   listaAuxiliarCPU->disponible=1;
		        	   if(programasNew!=NULL){
		        		   programasReady=enviarListaProgramas(programasNew,"Ready");
		        		   programasNew=programasNew->pNext;}
		        	   else
		        		   multiProg++;
		        	   break;
		           }
		            case PROGRAMA_BLOQUEADO:{
		               for(j=0;mensaje.payload[j]!=':';)
		            	  string[j++]=mensaje.payload[j];
		               string[j++]='\0';
		               strcpy(listaAuxiliarCPU->programaEjecutando->bloqueado,string);
		               listaAuxiliarCPU->programaEjecutando->unidadBloqueo=mensaje.payload[j]-'0';
		               programasBlock=enviarListaProgramas(listaAuxiliarCPU->programaEjecutando,"Block");
		               listaAuxiliarCPU->programaEjecutando=NULL;
		               listaAuxiliarCPU->disponible=1;
		               sem_post(&semIO);
		               break;
		            }
		          }
		        }
		        else
		        {
		         //Se ha cerrado la conexion con alguna cpu
		          printf("La CPU en la conexion %d se desconecto del sistema\n",i);
		          j=i;
		          while(j<nroSocketCPU){
		             socketCPU[j]=socketCPU[j+1];
                     j++;}
		          borrarCPU(i);
		          nroSocketCPU--;
		          printf("Numero de CPU actuales: %d\n",nroSocketCPU);
		        }
		        pthread_mutex_unlock (&semPrograma);
		        pthread_mutex_unlock (&semCPU);
		    }
		}

		//Se trata el socket que recibe las CPU
		if (FD_ISSET (socketEscuchaCPU, &descriptoresLectura)) {
		    //Acepta una conexion
			if ((socketCPU[nroSocketCPU] = accept(socketEscuchaCPU, NULL, 0)) < 0) {
			   perror("Error al aceptar conexion entrante");
			   return EXIT_FAILURE;}

			//Recibe el buffer enviado
			if ((nbytesRecibidos = recv(socketCPU[nroSocketCPU], buffer, BUFF_SIZE,0))> 0) {
				buffer[nbytesRecibidos]='\0';
				if(strcmp(buffer,"disponible")!=0)
				   printf("Mensaje recibido desconocido\n");
				else{
				   printf("Hay una CPU nueva!\n");
				   pthread_mutex_lock (&semCPU);
				   agregarCPU(idCPU,nroSocketCPU);
				   pthread_mutex_unlock (&semCPU);
				   idCPU++;
				   nroSocketCPU++;
				   printf("Numero de CPU actuales: %d\n",nroSocketCPU);
			       }
		   }
	   }
    }
}


//Inserto una programa en la lista elegida
nodoPCB enviarListaProgramas(nodoPCB PCB,char tipoLista[10]){

	nodoPCB lista,listaAnt;

    if(strcmp(tipoLista,"Ready")==0)
        lista=programasReady;

    if(strcmp(tipoLista,"End")==0)
        lista=programasEnd;

    if(strcmp(tipoLista,"Block")==0)
        lista=programasBlock;

    if(strcmp(tipoLista,"New")==0){
    	lista=listaAnt=programasNew;
    	if(lista == NULL)
    	  lista= PCB;
    	else{
    	  while(lista!= NULL){
    		if(lista->peso<PCB->peso)
    			  break;
    	    listaAnt=lista;
    	    lista=lista->pNext;}
    	  listaAnt->pNext = PCB;
    	  PCB->pNext=lista;
    	}
    }
    else{
      if (lista == NULL)
        lista= PCB;
      else{
        while(lista->pNext != NULL)
        {
            lista=lista->pNext;
        }
        lista->pNext = PCB;
      }
    }
    printf("Programa %d agregado a la cola de %s\n",PCB->id,tipoLista);
    return lista;
}
//Inserto una cpu en la lista
void agregarCPU(int id,int numSocket){

	nodoCPU NuevoNodo,lista;
    NuevoNodo =  malloc(sizeof(_CPU));//Creo un nodo
    NuevoNodo->id=id;
    NuevoNodo->numSocket=numSocket;
    NuevoNodo->disponible=1;
    NuevoNodo->programaEjecutando=NULL;
    NuevoNodo->pNext = NULL;
    lista=listaCPU;

    if (listaCPU == NULL)
        listaCPU= NuevoNodo;
    else{
        while(lista->pNext != NULL)
            lista=lista->pNext;
        lista->pNext = NuevoNodo;
    }
    printf("CPU %d agregada a la lista\n",id);
}
//Borro una cpu de la lista
void borrarCPU(int numSocket){

	nodoCPU lista,listaAnt;
    lista=listaAnt=listaCPU;
    if(listaCPU->numSocket==numSocket)
    	listaCPU=listaCPU->pNext;
    else{
      while(lista->numSocket!=numSocket){
    	listaAnt=lista;
    	lista=lista->pNext;}
      listaAnt->pNext=lista->pNext;}
    if(lista->programaEjecutando!=NULL){
      mensaje.id=PROGRAMA_ABORTADO;
      empaquetarMensaje(mensaje,string);
      //Le aviso al programa que la ejecucion fue abortada
      if (send(socketPrograma[lista->programaEjecutando->id], string, strlen(string), 0) >= 0)
    	  printf("Programa %d abortado\n",listaAuxiliarCPU->programaEjecutando->id);
      else
    	  perror("Error al enviar datos. Programa no encontrado.\n");
    }
    printf("CPU %d borrada de la lista\n",lista->id);
    while(lista != NULL){
    	lista->numSocket=lista->numSocket-1;
    	lista=lista->pNext;}
}

//Inserto un hilo de E/S en la lista
void agregarHiloIO(nodoHiloIO hIO){
	nodoHiloIO lista;
    lista=listaHilosIO;

    if (listaHilosIO == NULL)
        listaHilosIO= hIO;
    else{
        while(lista->pNext != NULL)
            lista=lista->pNext;
        lista->pNext = hIO;
    }
}
void inicializarListas(){
	programasNew =(nodoPCB)malloc(sizeof(_PCB));
	programasReady =(nodoPCB)malloc(sizeof(_PCB));
	programasExec =(nodoPCB)malloc(sizeof(_PCB));
	programasBlock =(nodoPCB)malloc(sizeof(_PCB));
	programasEnd =(nodoPCB)malloc(sizeof(_PCB));
	listaCPU =(nodoCPU)malloc(sizeof(_CPU));
	listaHilosIO =(nodoHiloIO)malloc(sizeof(_HILOIO));
	programasNew=NULL;
	programasReady=NULL;
	programasExec=NULL;
	programasBlock=NULL;
	programasEnd=NULL;
	listaCPU=NULL;
	listaHilosIO=NULL;
}
nodoPCB armarPCB(char programaRecibido[]){
    nodoPCB NuevoNodo;
    NuevoNodo =  malloc(sizeof(_PCB));//Creo un nodo
    NuevoNodo->id=idProg++;
    NuevoNodo->segmentoCodigo=(char*)malloc(sizeof(char)*strlen(programaRecibido));
    strcpy(NuevoNodo->segmentoCodigo,programaRecibido);
    datos=metadatada_desde_literal(programaRecibido);
    printf("La cantidad de instrucciones es %d\n",datos->instrucciones_size);
    printf("La cantidad de etiquetas es %d\n",datos->cantidad_de_etiquetas);
    printf("La cantidad de funciones es %d\n",datos->cantidad_de_funciones);
    printf("Tam del mapa serializado de etiquetas es %d\n",datos->etiquetas_size);
    printf("Serializacion de etiquetas es %s\n",datos->etiquetas);
    printf("El numero de la primera instruccion es %d\n",datos->instruccion_inicio);

    NuevoNodo->indiceCodigo=datos->instrucciones_serializado;
    NuevoNodo->tamIndiceCodigo=datos->instrucciones_size;
    NuevoNodo->indiceEtiquetas=datos->etiquetas;
    NuevoNodo->tamIndiceEtiquetas=datos->etiquetas_size;
    NuevoNodo->programCounter=datos->instruccion_inicio;
    NuevoNodo->tamContextoActual=0;
    NuevoNodo->segmentoStack=(char*)malloc(sizeof(char)*tamStack);
    NuevoNodo->cursorStack=(char*)malloc(sizeof(char)*tamStack);
    NuevoNodo->cursorStack=NuevoNodo->segmentoStack;
    NuevoNodo->peso=(5*datos->cantidad_de_etiquetas)+(3*datos->cantidad_de_funciones)+
    		datos->instrucciones_size;
    NuevoNodo->pNext = NULL;
    return NuevoNodo;
}
void *entradaSalida(void *id){
	printf("Hilo de %s creado\n",id);
	while(1){
	sem_wait(&semIO);
	pthread_mutex_lock (&semHiloIO);
	listaAuxHilosIO=listaHilosIO;
	while(strcmp(listaAuxHilosIO->id,id))
		listaAuxHilosIO=listaAuxHilosIO->pNext;
	listaAuxiliarProgramas=listaAntProgramas=programasBlock;
	pthread_mutex_lock (&semPrograma);
	if((programasBlock!=NULL)&&(strcmp(programasBlock->bloqueado,id)==0)){
		listaAuxHilosIO->programaEjecutando=programasBlock;
		programasBlock=programasBlock->pNext;}
	else{
		while(listaAuxiliarProgramas!=NULL){
			if(strcmp(listaAuxiliarProgramas->bloqueado,id)==0){
				listaAuxHilosIO->programaEjecutando=listaAuxiliarProgramas;
				listaAntProgramas->pNext=listaAuxiliarProgramas->pNext;
			    break;}
			listaAntProgramas=listaAuxiliarProgramas;
			listaAuxiliarProgramas=listaAuxiliarProgramas->pNext;}
	}
	pthread_mutex_unlock (&semPrograma);
	if(listaAuxHilosIO->programaEjecutando!=NULL){
	    printf("Programa %d atendido por %s\n",listaAuxHilosIO->programaEjecutando->id,id);
		while(listaAuxHilosIO->programaEjecutando->unidadBloqueo!=0){
		    printf("%s %d\n",id,listaAuxHilosIO->programaEjecutando->unidadBloqueo);
		    listaAuxHilosIO->programaEjecutando->unidadBloqueo--;
		    pthread_mutex_unlock (&semHiloIO);
		    usleep(listaAuxHilosIO->retardo);
		    pthread_mutex_lock (&semHiloIO);
		    listaAuxHilosIO=listaHilosIO;
		    while(strcmp(listaAuxHilosIO->id,id))
		    	listaAuxHilosIO=listaAuxHilosIO->pNext;}
		pthread_mutex_lock (&semPrograma);
		programasReady=enviarListaProgramas(listaAuxHilosIO->programaEjecutando,"Ready");
		pthread_mutex_unlock (&semPrograma);
		listaAuxHilosIO->programaEjecutando=NULL;}
	else
		sem_post(&semIO);
	pthread_mutex_unlock (&semHiloIO);
  }
}
int conectarUMV(){

	//Variables de comunicacion
	int nbytesRecibidos;
	struct sockaddr_in socketInfo;
    printf("Conectando al UMV...\n");

// Crear un socket:
// AF_INET: Socket de internet IPv4
// SOCK_STREAM: Orientado a la conexion, TCP
// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketUMV = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear socket");
		return EXIT_FAILURE;
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ipUMV);//Uso ip del kernel
	socketInfo.sin_port = htons(puertoUMV);//Uso puerto del kernel

// Conectar el socket con la direccion 'socketInfo'.
	if (connect(socketUMV, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {

		perror("Error al conectar socket");
		socketUMV=0;
		return EXIT_FAILURE;
	}
	printf("Conectado a la UMV!\n");
	return EXIT_SUCCESS;
}

int enviarPCB_UMV(nodoPCB pcb){
	char buffer[BUFF_SIZE];
	Sgm segmento;

	mensaje.id=RESERVAR_MEMORIA_PCB;
	segmento.idPrograma=pcb->id;
	strcpy(segmento.idSegmento,"segmentoCodigo");
    segmento.dirPuntero=pcb->segmentoCodigo;
    segmento.tam=strlen(pcb->segmentoCodigo);
    empaquetarSegmento(segmento,mensaje.payload);
    empaquetarMensaje(mensaje,buffer);

    if (send(socketUMV, buffer, strlen(buffer), 0) >= 0)
	  printf("Datos enviados a la UMV!\n");
    else
	  perror("Error al enviar datos. UMV no encontrada.\n");

    strcpy(segmento.idSegmento,"segmentoStack");
    segmento.dirPuntero=pcb->segmentoStack;
    segmento.tam=tamStack;
    empaquetarSegmento(segmento,mensaje.payload);
    empaquetarMensaje(mensaje,buffer);

    if (send(socketUMV, buffer, strlen(buffer), 0) >= 0)
      printf("Datos enviados a la UMV!\n");
    else
      perror("Error al enviar datos. UMV no encontrada.\n");

    strcpy(segmento.idSegmento,"indiceCodigo");
    segmento.dirPuntero=pcb->indiceCodigo;
    segmento.tam=pcb->tamIndiceCodigo;
    empaquetarSegmento(segmento,mensaje.payload);
    empaquetarMensaje(mensaje,buffer);

    if (send(socketUMV, buffer, strlen(buffer), 0) >= 0)
      printf("Datos enviados a la UMV!\n");
    else
      perror("Error al enviar datos. UMV no encontrada.\n");

    strcpy(segmento.idSegmento,"indiceEtiquetas");
    segmento.dirPuntero=pcb->indiceEtiquetas;
    segmento.tam=pcb->tamIndiceEtiquetas;
    empaquetarSegmento(segmento,mensaje.payload);
    empaquetarMensaje(mensaje,buffer);

    if (send(socketUMV, buffer, strlen(buffer), 0) >= 0)
       printf("Datos enviados a la UMV!\n");
    else
       perror("Error al enviar datos. UMV no encontrada.\n");

    return EXIT_SUCCESS;
}
