#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>


#define TAM 5



void escribir(int suma, int producto);
void imprimir();

int main(int argc, char *argv[]) {
	printf("Ejercicio 3, Grupo 18, 2024\n");
	printf("Integrantes:\n");
	printf("Antonio Bortoli - antoniobortoli2002@gmail.com\n");
	printf("Bautista Luetich - bauluetich@gmail.com\n");
	printf("Francisco Soltermann - frasoltermann@gmail.com\n");

	
	
	int numeros[TAM]; //arreglo para numeros ingresados por argumento
	int i;
	i=0;
	int suma[2],producto[2]; //pipes para comunicar procesos
	
	if(argc!=2){
		printf("ERROR: ejercicio2 arreglo.\n");
		exit(EXIT_FAILURE);
	}
	
	char *num=argv[1];
	//validamos que la cantidad de digitos sea la cantidad de TAM
	if(strlen(num)!=TAM){
		printf("Error: Debes ingresar exactamente %d numeros.\n", TAM);
		exit(EXIT_FAILURE);
	}
	//verificamos que se haya ingresado numeros 
	for (i=0;i<TAM;i++) {
		if (num[i]<'0' || num[i]>'9'){
			printf("Error: Solo se permiten digitos del 0 al 9.\n");
			exit(EXIT_FAILURE);
		}
		numeros[i]=num[i]-'0';
	}
	//creamos los pipes
	if(pipe(suma)==-1){ //para enviar el resultado de la suma
		perror("Error al crear pipe de suma\n");
		exit(EXIT_FAILURE);
	}
	else if(pipe(producto)==-1){ //para enviar el resultado del producto
		perror("Error al crear pipe de producto\n");
		exit(EXIT_FAILURE);
	}
	//creamos el proceso hijo 1
	pid_t pid1=fork();
	if(pid1==-1){
		perror("Error al crear proceso hijo 1\n");
		exit(EXIT_FAILURE);
	}
	int aux1;
	//calculamos la suma en el hijo 1
	if(pid1==0){
		close(suma[0]); //cerramos el extremo de lectura de pipe de suma[0]
		aux1=0;
		i=0;
		while(i<TAM){ //calculamos la suma de los numeros del arreglo
			aux1+=numeros[i];
			i++;
		} 
		
		if(write(suma[1], &aux1, sizeof(aux1))==-1){ //escribimos el resultado en el extremo de escritura suma[1]
			perror("Error al escribir en el pipe de suma\n");
			exit(EXIT_FAILURE);
		}
		
		close(suma[1]); //cerramos el pipe y terminamos el proceso hijo cerrar los pipe sirve para evitar bloqueos entre los procesos
		exit(EXIT_SUCCESS);
	}
	
	pid_t pid2=fork(); //creamos el proceso hijo 2
	if(pid2==-1){
		perror("Error al crear proceso hijo 2\n");
		exit(EXIT_FAILURE);
	}
	int aux2;
	if(pid2==0){
		close(producto[0]); //cerramos el extremo de lectura de pipe de producto[0]
		aux2=1;
		i=0;
		while(i<TAM){ //calculamos el producto de los numeros del arreglo
			aux2*=numeros[i];
			i++;
		}
		if(write(producto[1], &aux2, sizeof(aux2))==-1){ //escribimos el resultado en el extremo de escritura de producto[1]
			perror("Error al escribir en el pipe de producto\n");
			exit(EXIT_FAILURE);
		}
		close(producto[1]);
		exit(EXIT_SUCCESS);
	}
	close(suma[1]); //cerramos los extremos de escritura de los pipe en el padre
	close(producto[1]);
	//si no cerramos los extremos podria haber un bloqueo indefinido ya que va a quedar esperando el envio de datos porq los extremos de escritura estan abiertos
	
	int aux3, aux4;
	//leemos los resultados del proceso hijo 1 y 2
	if(read(suma[0], &aux3, sizeof(aux3))==-1){
		perror("Error al leer del pipe de suma\n");
		exit(EXIT_FAILURE);
	}
	if(read(producto[0], &aux4, sizeof(aux4))==-1){
		perror("Error al leer del pipe de producto\n");
		exit(EXIT_FAILURE);
	}
	
	close(suma[0]); //ceerramos los extremos de lectura en el padre
	close(producto[0]);
	
	escribir(aux3, aux4);
	imprimir();
	
	//espera a que los procesos hijos se terminen antes de terminar
	//podria finalizar antes de procesar los resultados, generar problemas de sincronizacion o perdida de datos si no colocamos los wait
	wait(NULL);
	wait(NULL);
	
	return 0;
}
void escribir(int suma, int producto) {
	int archivo;
	char buffer[500];
	int tamanio;
	
	archivo=open("resultados.txt", O_CREAT | O_WRONLY | O_TRUNC); //creamos o abrimos el archivo resultados txt
	
	if(archivo==-1){
		perror("Error al abrir el archivo\n");
	}
	
	tamanio=snprintf(buffer, sizeof(buffer), "Suma: %d\nProducto: %d\n", suma, producto);
	if(write(archivo, buffer, tamanio)==-1){
		perror("Error al escribir en el archivo\n");
	}
	close(archivo);
}

void imprimir(){
	int archivo;
	char contenido[500];
	archivo=open("resultados.txt", O_RDONLY);
	
	if(archivo==-1){
		perror("Error al abrir el archivo para lectura\n");
	}
	
	
	ssize_t leido;
	leido=read(archivo, contenido, sizeof(contenido)-1);
	if(leido==-1){
		perror("Error al leer el archivo");
	}
	
	contenido[leido]='\0';
	printf("Contenido del archivo:\n%s", contenido);
	
	close(archivo);
}
