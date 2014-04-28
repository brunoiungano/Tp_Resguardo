/*
 * umv.c
 *
 *  Created on: 24/04/2014
 *      Author: utnso
 */
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

#define BUFF_SIZE 1000

//Definimos variables globales
char *string, ipUmv[20], *direccion;
int puertoKernel,puertoCpu;
pthread_t hilorecibirProgramaCPU;
sem_t sem;


void* recibirProgramaCPU(void *);
void levantarArchivoUmv();

int main(int argc, char* argv[]){
	//Se lee la direccion del archivo y se lo asigna a la variable direccion
	direccion=(char*)malloc(sizeof(char)*100);
	strcpy(direccion,argv[2]);
	string=(char*)malloc(sizeof(char)*100);
	levantarArchivoUmv();//Levanto el archivo umv.txt
	sem_init(&sem,0,0);//Inicializo semaforo
	pthread_create(&hilorecibirProgramaCPU,NULL,recibirProgramaCPU,NULL);
	pthread_join(hilorecibirProgramaCPU,NULL);
	return 0;

}
//Levanta el archivo de configuracion
void levantarArchivoUmv(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"ip_umv");
	strcpy(ipUmv,string);
	puertoKernel=config_get_int_value(archivoConf,"puerto_kernel");
	puertoCpu=config_get_int_value(archivoConf,"puerto_cpu");
	config_destroy(archivoConf);

}

void* recibirProgramaCPU(void *var){

//Variables de comunicacion
	int socketEscuchaCpus,socketNuevaConexion,i,socketCPUs[100],nroSocketCPU,k=0;
    char buffer[BUFF_SIZE];
	struct sockaddr_in socketInfo;
	int nbytesRecibidos;
	int optval = 1;
	   //Variables de Select
    fd_set descriptoresLectura;
	struct timeval esperaSelect;


//Preparo el socket para recibir conexiones
	if ((socketEscuchaCpus = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}
	setsockopt(socketEscuchaCpus, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY;//Mi ip
	socketInfo.sin_port = htons(puertoCpu);//Mi puerto

	if (bind(socketEscuchaCpus, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
		perror("Error al bindear socket escucha");
		return EXIT_FAILURE;
	}

	//Preparado para escuchar conexiones
	if (listen(socketEscuchaCpus, 10) != 0) {
		perror("Error al poner a escuchar socket");
		return EXIT_FAILURE;

	}
	printf("Recibiendo programas\n");

while(1){
	    //Acepta una conexion
		if ((socketNuevaConexion = accept(socketEscuchaCpus, NULL, 0)) < 0) {
			perror("Error al aceptar conexion entrante");
			return EXIT_FAILURE;
	}

	    //Recibe el buffer enviado
		if ((nbytesRecibidos = recv(socketNuevaConexion, buffer, BUFF_SIZE,0))> 0) {
         printf("Se ha recibido un programa para almacenar\n");
         k++;
         printf("%d\n",k);
	        sem_post(&sem);
	            }
	}
FD_ZERO (&descriptoresLectura);
FD_SET (socketEscuchaCpus, &descriptoresLectura);
for (i=0; i<20; i++)
    FD_SET (socketCPUs[i], &descriptoresLectura);

esperaSelect.tv_sec=2; //Espera para el select en segundos
esperaSelect.tv_usec=0;//Espera para el select en milisegundos

select (100+1, &descriptoresLectura, NULL, NULL, &esperaSelect);

//Se trata los socket de las CPU
for (i=0; i<20; i++)
{
    if (FD_ISSET (socketCPUs[i], &descriptoresLectura))
    {
        if ((nbytesRecibidos = recv(socketCPUs[i], buffer, BUFF_SIZE,0))> 0)
        {
            /* Se ha leido un dato del cliente correctamente. Hacer aquí el tratamiento para ese mensaje. En el ejemplo, se lee y se escribe en pantalla. */
        }
        else
        {
         //Se ha cerrado la conexion con alguna cpu
          printf("La CPU en la conexion %d se desconecto del sistema\n",i);
        }
    }
}
//Se trata el socket que recibe las CPU
	if (FD_ISSET (socketEscuchaCpus, &descriptoresLectura)) {
	    //Acepta una conexion
		if ((socketCPUs[nroSocketCPU] = accept(socketEscuchaCpus, NULL, 0)) < 0) {
		   perror("Error al aceptar conexion entrante");
		   return EXIT_FAILURE;}

		//Recibe el buffer enviado
		if ((nbytesRecibidos = recv(socketCPUs[nroSocketCPU], buffer, BUFF_SIZE,0))> 0) {
			buffer[nbytesRecibidos]='\0';
			if(strcmp(buffer,"disponible")!=0)
			   printf("Mensaje recibido desconocido\n");
			else{
			   printf("Hay una CPU nueva!\n");

			}
		}
	}
}



