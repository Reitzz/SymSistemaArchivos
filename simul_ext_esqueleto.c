#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(){
	char *comando[LONGITUD_COMANDO];
	char *orden[LONGITUD_COMANDO];
	char *argumento1[LONGITUD_COMANDO];
	char *argumento2[LONGITUD_COMANDO];
	 
	int i,j;
	unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     
    fent = fopen("particion.bin","r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos,(EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
    for (;;){
		do {
		printf (">> ");
		fflush(stdin);
		fgets(comando, LONGITUD_COMANDO, stdin);
		} while (ComprobarComando(comando,orden,argumento1,argumento2) != 0);
		if (strcmp(orden, "info") == 0){
        	printf("Bloque %i Bytes\n", SIZE_BLOQUE);
        	printf("inodos particion = %i\n", MAX_INODOS);
        	printf("inodos libres = %i\n", ext_superblock.s_free_inodes_count);
        	printf("Bloques particion = %i\n", MAX_BLOQUES_PARTICION);
        	printf("Bloques libres = %i\n", ext_superblock.s_free_blocks_count);
        	printf("Primer bloque de datos = %i\n", ext_superblock.s_first_data_block);
            continue;
        }
        if (strcmp(orden, "bytemaps") == 0){
        	Printbytemaps(&ext_bytemaps);
    	}
	    if (strcmp(orden, "dir") == 0) {
            Directorio(&directorio, &ext_blq_inodos);
            continue;
        }
        
         // Escritura de metadatos en comandos rename, remove, copy     
         /*Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);*/
        /*if (grabardatos){
        	GrabarDatos(&memdatos,fent);
        	grabardatos = 0;
     	}*/	
         //Si el comando es salir se habrÃ¡n escrito todos los metadatos
         //faltan los datos y cerrar
        if (strcmp(orden, "salir") == 0){
        	printf("Adios");
            //GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
        }
        
        
    }
    
    
    
    
}



void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
	int i;
	printf("Inodos :");
	for (i = 0; i<MAX_INODOS; i++)
		printf("%i ", ext_bytemaps->bmap_inodos[i]);
	printf("\nBloques [0-25] :");
	for (i = 0; i<25; i++)
		printf("%i ", ext_bytemaps->bmap_bloques[i]);
	printf("\n");
}
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	/*if (strcmp(strcomando, "\0")){
		return 1;
	}*/
	char comandoArray[strlen(strcomando)];
	strcpy(comandoArray, strcomando);
	comandoArray[strlen(strcomando) - 1] = '\0';
	char *ph = strtok(comandoArray, " ");
	strcpy(orden, ph);
	ph = strtok(NULL, " ");
	if (ph == NULL){
		return 0;
	}
	strcpy(argumento1, ph);
	ph = strtok(NULL, " ");
	if (ph == NULL){
		return 0;
	}
	strcpy(argumento2, ph);
	return 0;
}
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
	
}
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
	
}
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
int i = 0, j = 0;
directorio++;
	for(i=0; i<=2; i++){
		j = 0;
		printf("\n%s\ttamaño:%i\tinodo:%i bloques:", directorio->dir_nfich, inodos->blq_inodos[directorio->dir_inodo].size_fichero, directorio->dir_inodo);
		while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] != 65535){
			printf("%i ", inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j++]);
		}
		directorio++;
	}
printf("\n");
}
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
	
}
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
	
}
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
	
}
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){
	
}
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
	
}
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
	
}
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
	
}
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
	
}
