#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define TAM 4096 //tamaño del buffer 4096 bytes porque la mayoria de los sistmas de archivos tienen un tamaño de bloque de 4kb por defecto

//muestra el porcentaje de progreso de la copia
//cantidad de bytes copiados hasta el momento sobre total de bytes a copiar
void progreso(off_t cantBytes, off_t tamTotal){
	double prog;
	
	prog=((cantBytes/(double)tamTotal)*100);
	
	printf("\rProgreso: %.2f%%\n", prog);
	
	
	fflush(stdout); //utilizamos esta funcion para actualizar la consola inmediatamente
}
int main(int argc, char *argv[]) {
	//argc es la cantidad de parametros que recibimos
	//argv contiene los parametros que recibimos
	printf("Ejercicio 1, Grupo 18, 2024\n");
	printf("Integrantes:\n");
	printf("Antonio Bortoli - antoniobortoli2002@gmail.com\n");
	printf("Bautista Luetich - bauluetich@gmail.com\n");
	printf("Francisco Soltermann - frasoltermann@gmail.com\n");
	
	bool bandera=false;
	//se utiliza para almacenar informacion sobre un archivo como tamaño archivo, permisos, fechas de modificacion
	//se rellena info con las funciones stat
	struct stat info; 
	
	char buffer[TAM];
	if(argc != 3){ //comprobamos que el usuario haya pasado por parametros las 2 rutas que debemos recibir (origen y destino)
		printf("ERROR: ejercicio1 ruta_archivo_origen ruta_archivo_destino.\n");
		bandera=true;
	}
	//abre el archivo de origen en modo lectura (O_RDONLY)
	int orig=open(argv[1], O_RDONLY);
	if(orig==-1 && bandera==false){
		perror("Error al abrir el archivo");
		bandera=true;
	}
	
	//utilizamos la funcion stat para llenar el struct info con la informacion del archivo
	if(stat(argv[1], &info)==-1 && bandera==false){
		perror("Error al obtener la informacion del archivo");
		close(orig);
		bandera=true;
	}
	off_t total=info.st_size; //para guardar tamaño de archivo en bytes
	
	//abrimos el archivo destino en modo escritura o_wrlony, o_trunc si ya esta creado para no sobreescribir (lo trunca), en el caso de que no sea asi o_creat
	int dest=open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if(dest==-1 && bandera==false){
		perror("Error al crear el archivo");
		close(orig);
		bandera=true;
	}
	
	ssize_t leido,escrito; //para guardar en bytes los datos leidos o escritos
	off_t copiado=0; //para guardar tamaño de archivo en bytes
	
	//leemos los datos del archivo origen y los vamos escribiendo en el archivo destino
	//vamos actualizando las variables a medida que vamos copiando el archivo para poder ir calculando el progreso y mostrarlo en pantalla
	while((leido=read(orig, buffer, TAM))>0 && bandera==false){
		escrito=write(dest, buffer, leido);
		if(escrito==-1){
			perror("Error al escribir en el archivo");
			close(orig);
			close(dest);
			
			
			bandera=true;
		}
		
		copiado+=escrito;
		progreso(copiado, total);
	}
	//verificamos que se haya podido leer el archivo
	if(leido==-1 && bandera==false){
	
		perror("Error al leer el archivo de origen");
		bandera = true;
	}

	
	
	
	close(orig);
	close(dest);
	printf("Finalizado.\n");	
	
	return 0;
}

