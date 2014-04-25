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

#define BUFF_SIZE 1000 //Tamaño del buffer de datos que se transfieren entre procesos

//PCB: Estructura que se usa para guardar la informacion de un programa
typedef struct _pcb
{
    int id;//identificador
    char segmentoCodigo[1000];
    char segmentoStack[10];
    char *cursorStack;
    int indiceCodigo[30][30];
    int indiceEtiquetas[30][30];
    int programCounter;
    int tamContextoActual;
    struct _PCB * pNext;
}_PCB;
typedef _PCB * nodoPCB;
nodoPCB programasNew;
nodoPCB programasReady;
nodoPCB programasExec;
nodoPCB programasBlock;

//Estructura que se usa para guardar la informacion de las cpu
typedef struct _cpu
{
    int id;//identificador
    int numSocket;//Numero de socket que usa
    int disponible;
    nodoPCB programaEjecutando;
    struct _CPU * pNext;
}_CPU;
typedef _CPU * nodoCPU;
nodoCPU listaCPU;

//Definicion de funciones
void *PLP(void *);
void *PCP(void *);
void levantarArchivoKernel();
void inicializarListas();
nodoPCB enviarListaProgramas(nodoPCB PCB,char tipoLista[]);
void agregarCPU(int id,int numSocket);
void borrarCPU(int numSocket);
nodoPCB armarPCB(char programaRecibido[]);

//Variables globales
char *string, ipKernel[20], *direccion;
int puertoCpu, puertoProg, multiProg, quantum, retardo, idProg, idCPU;
pthread_t hiloPLP,hiloPCP;
sem_t sem;
nodoPCB programa;

//INICIO
int main(int argc, char* argv[]) {
	//Se lee la direccion del archivo y se lo asigna a la variable direccion
	direccion=(char*)malloc(sizeof(char)*100);
	strcpy(direccion,argv[2]);
	string=(char*)malloc(sizeof(char)*100);//Reservo memoria
	levantarArchivoKernel();//Levanto el archivo kernel
	inicializarListas();
	sem_init(&sem,0,0);//Inicializo semaforo
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
	multiProg=config_get_int_value(archivoConf,"multiprogramacion");
	quantum=config_get_int_value(archivoConf,"quantum");
	retardo=config_get_int_value(archivoConf,"retardo");
	config_destroy(archivoConf);
}

void *PLP(void *var){
	//Variables de comunicacion
		int socketEscuchaProgramas,socketNuevaConexion;
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
			if ((socketNuevaConexion = accept(socketEscuchaProgramas, NULL, 0)) < 0) {
				perror("Error al aceptar conexion entrante");
				return EXIT_FAILURE;
		}

		    //Recibe el buffer enviado
			if ((nbytesRecibidos = recv(socketNuevaConexion, buffer, BUFF_SIZE,0))> 0) {
		        buffer[nbytesRecibidos]='\0';
		        printf("Se ha recibido un programa para enviar a CPU\n");
		        programa=armarPCB(buffer);
		        if(multiProg>0)
		        	programasReady=enviarListaProgramas(programa,"Ready");
		        else
		        	programasNew=enviarListaProgramas(programa,"New");
		        multiProg--;
		        sem_post(&sem);
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
       nodoCPU listaAuxiliarCPU;


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

		listaAuxiliarCPU=listaCPU;
		while((programasReady!=NULL)&&(listaAuxiliarCPU!=NULL)){
		  if(listaAuxiliarCPU->disponible==1){
			 listaAuxiliarCPU->disponible=0;
			 listaAuxiliarCPU->programaEjecutando=programasReady;
			 programasReady=programasReady->pNext;
			 strcpy(buffer,listaAuxiliarCPU->programaEjecutando->segmentoCodigo);
		     //Le mando el programa a la CPU
		  		if (send(socketCPU[listaAuxiliarCPU->numSocket], buffer, strlen(buffer), 0) >= 0)
		  		   printf("Programa %d enviado a la CPU %d\n",listaAuxiliarCPU->programaEjecutando->id,
		  				 listaAuxiliarCPU->id);
		  		else
		  		   perror("Error al enviar datos. CPU no encontrado.\n");}
		 listaAuxiliarCPU=listaAuxiliarCPU->pNext;}

		//Explicacion del select: http://www.chuidiang.com/clinux/sockets/socketselect.php
		FD_ZERO (&descriptoresLectura);
		FD_SET (socketEscuchaCPU, &descriptoresLectura);
		for (i=0; i<idCPU; i++)
		    FD_SET (socketCPU[i], &descriptoresLectura);

		esperaSelect.tv_sec=2; //Espera para el select en segundos
		esperaSelect.tv_usec=0;//Espera para el select en milisegundos

		select (100+1, &descriptoresLectura, NULL, NULL, &esperaSelect);

		//Se trata los socket de las CPU
		for (i=0; i<idCPU; i++)
		{
		    if (FD_ISSET (socketCPU[i], &descriptoresLectura))
		    {
		        if ((nbytesRecibidos = recv(socketCPU[i], buffer, BUFF_SIZE,0))> 0)
		        {
		            /* Se ha leido un dato del cliente correctamente. Hacer aquí el tratamiento para ese mensaje. En el ejemplo, se lee y se escribe en pantalla. */
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
				   agregarCPU(idCPU,nroSocketCPU);
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

	nodoPCB lista;

    if(strcmp(tipoLista,"New")==0)
        lista=programasNew;

    if(strcmp(tipoLista,"Ready")==0)
        lista=programasReady;

    if (lista == NULL)
        lista= PCB;
    else{
        while(lista->pNext != NULL)
        {
            lista=lista->pNext;
        }
        lista->pNext = PCB;
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
    printf("CPU %d borrada de la lista\n",lista->id);
    while(lista != NULL){
    	lista->numSocket=lista->numSocket-1;
    	lista=lista->pNext;}
}
void inicializarListas(){
	programasNew =(nodoPCB)malloc(sizeof(_PCB));
	programasReady =(nodoPCB)malloc(sizeof(_PCB));
	programasExec =(nodoPCB)malloc(sizeof(_PCB));
	programasBlock =(nodoPCB)malloc(sizeof(_PCB));
	listaCPU =(nodoCPU)malloc(sizeof(_PCB));
	programasNew=NULL;
	programasReady=NULL;
	programasExec=NULL;
	programasBlock=NULL;
	listaCPU=NULL;
}
nodoPCB armarPCB(char programaRecibido[]){
    nodoPCB NuevoNodo;
    NuevoNodo =  malloc(sizeof(_PCB));//Creo un nodo
    NuevoNodo->id=idProg++;
    strcpy(NuevoNodo->segmentoCodigo,programaRecibido);
    NuevoNodo->pNext = NULL;
    return NuevoNodo;
}

