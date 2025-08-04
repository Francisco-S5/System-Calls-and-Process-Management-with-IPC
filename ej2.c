#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define TAM 500
#define TAM2 1000

//determinamos el estado del proceso usando el archivo /proc/[pid]/stat
void Estado(pid_t pid){
	char ruta[TAM];
	int archivo;
	char nombre[TAM2];
	bool bandera=false;
	
	//construye la ruta dinamica hacia /proc/[pid]/stat
	snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid); 
	//abrimos el archivo en modo lectura
	archivo=open(ruta, O_RDONLY);
	//verificamos que se haya abierto bien
	if(archivo==-1){
		perror("Error al abrir el archivo stat");
		bandera=true;
	}
	
	//leemos el archivo y lo vamos almacenando en nombre
	if(read(archivo, nombre, sizeof(nombre)) > 0 && !bandera){
		char estado;
		//analizamos el tercer campo %c que nos va a indicar el estado del proceso
		sscanf(nombre, "%*d %*s %c", &estado);
		printf("Estado del proceso: \n");
		if(estado=='R'){
			printf("Ejecutando\n");
		}
		else if(estado=='D'){
			printf("Ininterrumpible\n");
		}
		else if(estado=='S'){
			printf("Durmiendo\n");
		}
		else if(estado=='T'){
			printf("Detenido\n");
		}
		else if(estado=='Z'){
			printf("Zombi\n");
		}
		else{
			printf("Sin Estado\n");
		}
	}
	close(archivo);
}

//muestra el uso de memoria del proceso basado en el archivo /proc/[pid]/statm
void Memoria(pid_t pid) {
	char ruta[TAM];
	int archivo;
	char nombre[TAM2];
	bool bandera = false;
	
	//nuevamente construimos la ruta, similar a estado pero hacia /proc/[pid]/statm
	snprintf(ruta, sizeof(ruta), "/proc/%d/statm", pid);
	archivo = open(ruta, O_RDONLY);
	
	
	
	if(archivo==-1){
		perror("Error al abrir el archivo statm");
		bandera = true;
	}
	//leemos el archivo
	if(read(archivo, nombre, sizeof(nombre))>0 && !bandera){
		long memoria;
		//obtenemos el primer campo que representa las paginas de memorias usadas
		sscanf(nombre, "%ld", &memoria);
		//convertimos ese valor a kb multiplicando las paginas por 4
		printf("Uso de memoria: %ld KB\n", memoria * 4);
	}
	
	close(archivo);
}

//calculamos el uso de cpu del proceso en el intervalo de 1s
void UsoCPU(pid_t pid) {
	char ruta[TAM];
	int archivo;
	char nombre[TAM2];
	long tiempoUsuario, tiempoKernel, tiempoUsuario2, tiempoKernel2;
	long total1,total2;
	struct timespec ts;
	
	
	//como en las otras funciones contruimos la ruta hacia /proc/[pid]/stat
	snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);
	//abrimos el archivo en modo lectura
	archivo=open(ruta, O_RDONLY);
	
	if(archivo==-1){
		perror("Error al abrir el archivo stat para uso de CPU");
		return;
	}
	if(read(archivo, nombre, sizeof(nombre))>0){
		//obtenemos los tiempos que el proceso paso en modo kernel y usuario, aquellos que tengan * son omitidos
		sscanf(nombre, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld", &tiempoUsuario, &tiempoKernel);
	//el campo 13 y 14 de /proc/[pid]/stat tienen el tiempo que paso el proceso en usuario y kernel
	}
	close(archivo);
	
	
	//configura el periodo de tiempo total de espera (1 seg)
	ts.tv_sec = 1; //esperamos 1 segundo para calcular el porcentaje de uso
	ts.tv_nsec = 0;
	nanosleep(&ts, NULL); //suspende el procesoa ctual en 1 seg, dejando que los demas procesos continuen si ejecucion
	archivo=open(ruta, O_RDONLY);
	if(archivo==-1) {
		perror("Error al abrir el archivo stat para uso de CPU");
		return;
	}
	if(read(archivo, nombre, sizeof(nombre))>0){
		//volvemos a obtener los valores del tiempo de usuario y kernel luego de 1 segundo
		sscanf(nombre, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld", &tiempoUsuario2, &tiempoKernel2);
	}
	close(archivo);
	
	
	
	//obtenemos el tiempo total
	total1=tiempoUsuario+tiempoKernel;
	total2=tiempoUsuario2+tiempoKernel2;
	//calculamos el porcentaje de uso, sysconf convierte los tiempos en segundos dividiendo por el número de "ticks" del sistema
	double porcentaje=((double)(total2-total1)/sysconf(_SC_CLK_TCK))*100;
	
	printf("Uso de CPU: %.2f%%\n", porcentaje);
}


void Prioridad(pid_t pid){
	int prioridad;
	//obtenemos la prioridad del proceso utilizando getpriority
	prioridad=getpriority(PRIO_PROCESS, pid);
	
	
	if(prioridad==-1 && errno != 0){
		perror("Error al obtener la prioridad del proceso");
	}
	else{
		printf("Prioridad del proceso: %d\n", prioridad);
	}
}

bool infoProceso(pid_t pid) {
	char ruta[TAM];
	int archivo;
	char nombre[TAM2];
	bool bandera = true;
	
	snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid);
	archivo = open(ruta, O_RDONLY);
	
	
	if (archivo==-1) {
		perror("Error al abrir el archivo status");
		bandera=false;
		return bandera;
	}
	
	if (read(archivo, nombre, sizeof(nombre))>0 && bandera) {
		char *linea = strtok(nombre, "\n");//divimos el contenido de nombre en lineas
		
		while (linea != NULL) { //recorremos cada linea y buscamos la que comienze con "Name:"
			if (strncmp(linea, "Name:", 5) == 0) {//si encuentra la linea obtenemos el nombre del proceso que comienza en el 6to caracter
				printf("Nombre del proceso: %s\n", linea + 6);
				break;
			}
			linea = strtok(NULL, "\n");
		}
	}
	
	close(archivo);
	Estado(pid);
	Memoria(pid);
	UsoCPU(pid);
	Prioridad(pid);
	return bandera;
}
	

int main(int argc, char *argv[]){
	printf("Ejercicio 2, Grupo 18, 2024\n");
	printf("Integrantes:\n");
	printf("Antonio Bortoli - antoniobortoli2002@gmail.com\n");
	printf("Bautista Luetich - bauluetich@gmail.com\n");
	printf("Francisco Soltermann - frasoltermann@gmail.com\n");
	bool bandera=false;
	
	if(argc!=2){
		printf("Sintaxis: ejercicio2 PID\n");
		bandera=true;
	}
	
	pid_t pid;
	pid=atoi(argv[1]); //convertimos a entero el pid pasado como argumento
	
	if(pid<=0 && !bandera){
		printf("PID inválido\n");
		bandera=true;
	}
	
	if(!infoProceso(pid) && !bandera){
		printf("No se pudo obtener informacion del proceso %d\n", pid);
	}
	
	return 0;
}
