#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, int nficheros);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo, int nficheros);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre, int nficheros);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, int nficheros);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(){
	char comando[LONGITUD_COMANDO];
	char orden[LONGITUD_COMANDO];
	char argumento1[LONGITUD_COMANDO];
	char argumento2[LONGITUD_COMANDO];
	 
	int i,j;
	unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos = 0, nficheros = -3;
    FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     
    fent = fopen("particion.bin","r+b");
    
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    

    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS*SIZE_BLOQUE);
    for(i = 0; i <=MAX_INODOS; i++){
    	if(ext_bytemaps.bmap_inodos[i] == 1){
    		nficheros++;
		}
	}
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
        }
        else if (strcmp(orden, "bytemaps") == 0)
        	Printbytemaps(&ext_bytemaps);
	    else if (strcmp(orden, "dir") == 0) 
            Directorio(directorio, &ext_blq_inodos, nficheros);
        else if (strcmp(orden, "rename") == 0) {
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2, nficheros);
            grabardatos = 1;
        }
		else if (strcmp(orden, "print") == 0)
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1, nficheros);
        else if (strcmp(orden, "remove") == 0){
        	Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, nficheros);
		}
        else if (grabardatos == 1){
        	GrabarSuperBloque(&ext_superblock, fent);
        	GrabarByteMaps(&ext_bytemaps, fent);
        	Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
        	GrabarDatos(memdatos, fent);
        	grabardatos = 0;
     	}
        else if (strcmp(orden, "exit") == 0){
            printf("Adios");
            fclose(fent);
            return 0;
        }
        else printf("No existe el comando %s\n", orden);
    } 
}


int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	/*if (strcmp(strcomando, "")){
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

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, int nficheros){
int i, j;
directorio++;
	for(i = 0; i<=(nficheros-1); i++){
		if (strcmp(directorio->dir_nfich, "" ) == 0){
			directorio++;	
		}
		else{
			j = 0;
			printf("\n%s\ttamaño:%i\tinodo:%i bloques:", directorio->dir_nfich, inodos->blq_inodos[directorio->dir_inodo].size_fichero, directorio->dir_inodo);
			while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] != 0xFFFF){
				printf("%i ", inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j++]);
			}
			directorio++;
		}
	}
printf("\n");
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo, int nficheros){
	int i;
	for(i=0; i<=nficheros; i++){
		if (strcmp(nombrenuevo, directorio->dir_nfich) == 0){
			printf("El nombre de fichero ya existe\n");
			return 0;
		}
		directorio++;
	}
	directorio = directorio - (nficheros - 1);
	for(i=0; i<=nficheros; i++){
		if (strcmp(nombreantiguo, directorio->dir_nfich) == 0){
			strcpy(directorio->dir_nfich, nombrenuevo);
			return 0;
		}
		directorio++;
	}
	printf("No se ha encontrado el fichero\n");
	return 0;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
	
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
	
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre, int nficheros){
	int i, j, k, existe;
	for(i=0; i<=nficheros; i++){
		if (strcmp(nombre, directorio->dir_nfich) == 0){
			j = 0;
			while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] != 0xFFFF){
				k = 0;
				memdatos = memdatos + inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] - 4;
				while (k <= SIZE_BLOQUE && memdatos->dato[k] != 0){
					printf("%c", memdatos->dato[k]);
					k++;
				}
				memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] + 4;
				j++;
			}
			printf("\n");
			return 0;
		}
		directorio++;
	}
	printf("No se ha encontrado el fichero\n");			
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, int nficheros){
	int i, j=0, nbloques = 0;	
	for(i=0; i<=nficheros; i++){
		if (strcmp(nombre, directorio->dir_nfich) == 0){
			while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j++] != 0xFFFF)
				nbloques++;
			strcpy(directorio->dir_nfich , "");
			ext_bytemaps->bmap_inodos[directorio->dir_inodo] = 0;
			inodos->blq_inodos[directorio->dir_inodo].size_fichero = 0;
			for(j = 0; j<nbloques; j++){
				ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j]] = 0;
			}
			for(j = 0; j <=MAX_NUMS_BLOQUE_INODO; j++)
				inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] = 0xFFFF;
			directorio->dir_inodo = 0xFFFF;
			return 0;
		}
		directorio++;
	}
	/*if (strcmp(nombre, "")){
		printf("Nombre de fichero vacío, el comando se escribe como remove 'nombre del fichero'\n");
	}*/
	printf("No se ha encontrado el fichero\n");
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino){
	
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
	rewind(fich);
	fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
	fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
	fseek(fich, SIZE_BLOQUE*2, SEEK_SET);
	fwrite(inodos, SIZE_BLOQUE, 1, fich);
	fseek(fich, SIZE_BLOQUE*3, SEEK_SET);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
	fseek(fich, SIZE_BLOQUE*4, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
    rewind(fich);
}
