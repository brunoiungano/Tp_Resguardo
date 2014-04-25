#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h> //Biblioteca de la catedra
#include <commons/temporal.h> //Biblioteca de la catedra
#include <curses.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "utiles.h" //Biblioteca de funciones utiles
#include <signal.h>
#include "pthread.h" //Biblioteca de hilos
#include "semaphore.h"//Biblioteca de semaforos

#define BUFF_SIZE 1000 //Tamaño del buffer de datos que se transfieren entre procesos

//Definicion de funciones
int enviarScript();
void levantarArchivoPrograma();
void levantarPrograma();

//Variables globales
char *string, ipKernel[20], *direccion, script[100], programa[1000];
int puertoKernel;

//INICIO
int main(int argc, char* argv[]) {
	//Se lee la direccion del archivo y se lo asigna a la variable direccion
	direccion=(char*)malloc(sizeof(char)*100);
	strcpy(direccion,argv[2]);
	string=(char*)malloc(sizeof(char)*100);
	levantarArchivoPrograma();//Levanto el archivo programa
	levantarPrograma();
	enviarScript();
	return 0;
}

//Levanta el archivo de configuracion
void levantarArchivoPrograma(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"ip_kernel");
	strcpy(ipKernel,string);
	puertoKernel=config_get_int_value(archivoConf,"puerto_kernel");
	string=config_get_string_value(archivoConf,"script");
	strcpy(script,string);
	config_destroy(archivoConf);
}
//Envia el script al kernel
int enviarScript(){

	//Declaro las variables de comunicacion
	int unSocket,nbytesRecibidos;
	struct sockaddr_in socketInfo;
    char buffer[BUFF_SIZE];
	printf("Conectando...\n");

// Crear un socket:
// AF_INET: Socket de internet IPv4
// SOCK_STREAM: Orientado a la conexion, TCP
// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear socket");
		return EXIT_FAILURE;
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ipKernel);//Uso ip del kernel
	socketInfo.sin_port = htons(puertoKernel);//Uso puerto del kernel

// Conectar el socket con la direccion 'socketInfo'.
	if (connect(unSocket, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {

		perror("Error al conectar socket");
		return EXIT_FAILURE;
	}
	printf("Conectado al kernel!\n");

	strcpy(buffer,programa);
    //Le mando el programa al kernel
		if (send(unSocket, buffer, strlen(buffer), 0) >= 0)
			printf("Datos enviados al kernel!\nEsperando respuesta...\n");
		else
			perror("Error al enviar datos. Kernel no encontrado.\n");

		if ((nbytesRecibidos = recv(unSocket, buffer, BUFF_SIZE,0))== 0)
			perror("Error al recibir datos. Kernel no disponible");
		else{
			buffer[nbytesRecibidos]='\0';
			return EXIT_SUCCESS;}
}
void levantarPrograma(){
	char caracter;
	int i=0;
	FILE *Archivo;
	Archivo= fopen(script, "r"); /*Abrir archivo para lectura*/
    if(Archivo == NULL){
	   printf("Error al abrir el archivo \n");
	   exit (EXIT_FAILURE);}
    while((caracter=fgetc(Archivo))!=EOF)
		programa[i++]=caracter;
    programa[i]='\0';
    fclose(Archivo);
}
