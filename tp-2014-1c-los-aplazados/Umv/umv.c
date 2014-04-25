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
char *string, ipUmv[20], *direccion;
int puertoKernel,puertoProg;

void levantarArchivoUmv();

int main(int argc, char* argv[]){
	//Se lee la direccion del archivo y se lo asigna a la variable direccion
	direccion=(char*)malloc(sizeof(char)*100);
	strcpy(direccion,argv[2]);
	string=(char*)malloc(sizeof(char)*100);
	levantarArchivoUmv();//Levanto el archivo umv.txt
	printf("%s %d %d",string,puertoKernel,puertoProg);
	return 0;

}
//Levanta el archivo de configuracion
void levantarArchivoUmv(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"ip_umv");
	strcpy(ipUmv,string);
	puertoKernel=config_get_int_value(archivoConf,"puerto_kernel");
	puertoProg=config_get_int_value(archivoConf,"puerto_programa");
	config_destroy(archivoConf);

}
