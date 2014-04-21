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

//Definicion de funciones
void *recibirProgramas(void *);
void *recibirCPU(void *);
void levantarArchivoKernel();

//Variables globales
char *string, ipKernel[20], *direccion, *programaRecibido;
int puertoCpu,puertoProg;
pthread_t hiloRecibeProg,hiloRecibeCpu;
sem_t sem;

//INICIO
int main(int argc, char* argv[]) {
	//Se lee la direccion del archivo y se lo asigna a la variable direccion
	direccion=(char*)malloc(sizeof(char)*100);
	strcpy(direccion,argv[2]);
	string=(char*)malloc(sizeof(char)*100);//Reservo memoria
	levantarArchivoKernel();//Levanto el archivo kernel
	sem_init(&sem,0,0);//Inicializo semaforo
	pthread_create(&hiloRecibeProg,NULL,recibirProgramas,NULL);//Creo el hilo para recibir programas
	pthread_create(&hiloRecibeCpu,NULL,recibirCPU,NULL);//Creo el hilo para recibir CPU
	pthread_join(hiloRecibeProg,NULL);
	pthread_join(hiloRecibeCpu,NULL);
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
	config_destroy(archivoConf);
}

void *recibirProgramas(void *var){
	//Variables de comunicacion
		int socketEscucha,socketNuevaConexion;
	    char buffer[BUFF_SIZE];
		struct sockaddr_in socketInfo;
		int nbytesRecibidos;
		int optval = 1;

	//Preparo el socket para recibir conexiones
		if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return EXIT_FAILURE;
		}

		setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		socketInfo.sin_family = AF_INET;
		socketInfo.sin_addr.s_addr = INADDR_ANY;//Mi ip
		socketInfo.sin_port = htons(puertoProg);//Mi puerto

		if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
			perror("Error al bindear socket escucha");
			return EXIT_FAILURE;
		}

		//Preparado para escuchar conexiones
		if (listen(socketEscucha, 10) != 0) {
			perror("Error al poner a escuchar socket");
			return EXIT_FAILURE;

		}
		printf("Recibiendo programas\n");

		while(1){
		    //Acepta una conexion
			if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0) {
				perror("Error al aceptar conexion entrante");
				return EXIT_FAILURE;
		}

		    //Recibe el buffer enviado
			if ((nbytesRecibidos = recv(socketNuevaConexion, buffer, BUFF_SIZE,0))> 0) {
		        buffer[nbytesRecibidos]='\0';
		        printf("Se ha recibido un programa para enviar a CPU\n");
		        programaRecibido=(char*)malloc(sizeof(char)*nbytesRecibidos);
		        strcpy(programaRecibido,buffer);
		        sem_post(&sem);
              }
		}
}
void *recibirCPU(void *var){
	//Variables de comunicacion
		int socketEscucha,socketNuevaConexion;
	    char buffer[BUFF_SIZE];
		struct sockaddr_in socketInfo;
		int nbytesRecibidos;
		int optval = 1;

	//Preparo el socket para recibir conexiones
		if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return EXIT_FAILURE;
		}

		setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		socketInfo.sin_family = AF_INET;
		socketInfo.sin_addr.s_addr = INADDR_ANY;//Mi ip
		socketInfo.sin_port = htons(puertoCpu);//Mi puerto

		if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
			perror("Error al bindear socket escucha");
			return EXIT_FAILURE;
		}

		//Preparado para escuchar conexiones
		if (listen(socketEscucha, 10) != 0) {
			perror("Error al poner a escuchar socket");
			return EXIT_FAILURE;

		}
		printf("Recibiendo CPU\n");

		while(1){
		    //Acepta una conexion
			if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0) {
				perror("Error al aceptar conexion entrante");
				return EXIT_FAILURE;
		}

		    //Recibe el buffer enviado
			if ((nbytesRecibidos = recv(socketNuevaConexion, buffer, BUFF_SIZE,0))> 0) {
		        buffer[nbytesRecibidos]='\0';
				if(strcmp(buffer,"disponible")!=0){
					printf("Mensaje recibido desconocido\n");}
				else{
		        printf("Hay una CPU disponible\n");
		        sem_wait(&sem);//Bloqueo el semaforo hasta que haya un programa
		        strcpy(buffer,programaRecibido);
		        if (send(socketNuevaConexion, buffer, strlen(buffer), 0) >= 0) {
		        			printf("Programa enviado a la CPU!\n");
		        		}
		                 //Si no se pudieron enviar los datos informo el error
		        		 else {
		        			perror("Error al enviar datos. CPU no encontrado.\n");
		        		}
		        sem_post(&sem);
              }
		}
    }
}
