#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
int BuscaFich(EXT_ENTRADA_DIR *directorio, char *nombre);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
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
     
     
    fent = fopen("particion.bin","r+b");
    
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    

    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS*SIZE_BLOQUE);
    
    for (;;){
		do {
		printf (">> ");
		fflush(stdin);
		fgets(comando, LONGITUD_COMANDO, stdin);
		} while (ComprobarComando(comando,orden,argumento1,argumento2) != 0);
		if (strcmp(orden, "info") == 0)
        	LeeSuperBloque(&ext_superblock);
        else if (strcmp(orden, "bytemaps") == 0)
        	Printbytemaps(&ext_bytemaps);
	    else if (strcmp(orden, "dir") == 0) 
            Directorio(directorio, &ext_blq_inodos);
        else if (strcmp(orden, "rename") == 0) {
            if(Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) == 1)
            	grabardatos = 0;
            else 
				grabardatos = 1;
        }
		else if (strcmp(orden, "print") == 0)
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        else if (strcmp(orden, "remove") == 0){
        	if(Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1) == 1)
        		grabardatos = 0;
            else 
				grabardatos = 1;
		}
		else if (strcmp(orden, "copy") == 0){
        	if(Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos,  argumento1, argumento2) == 1)
        		grabardatos = 0;
            else 
				grabardatos = 1;
		}
        else if (strcmp(orden, "exit") == 0){
            printf("Adios");
            fclose(fent);
            return 0;
        }
        else printf("No existe el comando %s\n", orden);
        if (grabardatos == 4){
        	GrabarSuperBloque(&ext_superblock, fent);
        	GrabarByteMaps(&ext_bytemaps, fent);
        	Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
        	GrabarDatos(memdatos, fent);
        	grabardatos = 0;
     	}
    } 
}


int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	if (strcmp(strcomando, "\n") == 0){
		return 1;
	}
	char comandoArray[strlen(strcomando) + 1];
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

int BuscaFich(EXT_ENTRADA_DIR *directorio, char *nombre){
	int i;
	for(i=0; (directorio->dir_inodo != 0xFFFF || strcmp(directorio->dir_nfich, "" ) == 0); i++){
		if (strcmp(nombre, (directorio++)->dir_nfich) == 0){
			return i;
		}	
	}
	return 0;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
	printf("Bloque %i Bytes\n", psup->s_block_size);
    printf("Inodos particion = %i\n", psup->s_inodes_count);
    printf("Inodos libres = %i\n", psup->s_free_inodes_count);
    printf("Bloques particion = %i\n", psup->s_blocks_count);
    printf("Bloques libres = %i\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %i\n", psup->s_first_data_block);
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

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
	int i;
	while(directorio->dir_inodo != 0xFFFF || strcmp(directorio->dir_nfich, "" ) == 0){
		if (strcmp(directorio->dir_nfich, "." ) == 0 || strcmp(directorio->dir_nfich, "" ) == 0){
			directorio++;	
		}
		else{
			i = 0;
			printf("%s\ttamaño:%i\tinodo:%i bloques:", directorio->dir_nfich, inodos->blq_inodos[directorio->dir_inodo].size_fichero, directorio->dir_inodo);
			while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] != 0xFFFF){
				printf("%i ", inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i++]);
			}
			printf("\n");
			directorio++;
		}
	}
	printf("\n");
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
	int i;
	if (BuscaFich(directorio, nombrenuevo) != 0){
		printf("El nombre de fichero ya existe\n");
		return 0;
	}
	if (BuscaFich(directorio, nombreantiguo) != 0){
		directorio += BuscaFich(directorio, nombreantiguo);
		strcpy(directorio->dir_nfich, nombrenuevo);
		return 0;
	}
	printf("No se ha encontrado el fichero\n");
	return 1;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
	int i, j, nfichero;
	if (BuscaFich(directorio, nombre) != 0){
		nfichero = BuscaFich(directorio, nombre);
		directorio += nfichero;
		for(i = 0; inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] != 0xFFFF; i++){
			memdatos = memdatos + inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] - 4;
			for (j=0; j<= SIZE_BLOQUE && memdatos->dato[j] != 0; j++){
				printf("%c", memdatos->dato[j]);
			}
			memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] + 4;
		}
		printf("\n");
		return 0;
	}
	if (strcmp(nombre, "\n") == 0){
		printf("Nombre de fichero vacío, el comando se escribe como print 'nombre del fichero'\n");
		return 1;
	}
	printf("No se ha encontrado el fichero\n");
	return 1;	
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre){
	int i = 0, j, nbloques = 0;
	if (BuscaFich(directorio, nombre) != 0){	
		directorio += BuscaFich(directorio, nombre);
		while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i++] != 0xFFFF)
			nbloques++;
		ext_superblock->s_free_blocks_count += nbloques;							//Superbloque bloques libres
		ext_superblock->s_free_inodes_count += 1;									//Superbloque inodos
		strcpy(directorio->dir_nfich , "");											//dir nombre a ""
		ext_bytemaps->bmap_inodos[directorio->dir_inodo] = 0;						//bytemap inodos a 0
		inodos->blq_inodos[directorio->dir_inodo].size_fichero = 0;					//size a 0
		for(i = 0; i<nbloques; i++){
			ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i]] = 0;	// bytemap bloques a 0
		}
		for(i = 0; i <=MAX_NUMS_BLOQUE_INODO; i++){
			if(ext_superblock->s_first_data_block == inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i]){
				for(j = 4; j<=MAX_BLOQUES_DATOS; j++){
					if(ext_bytemaps->bmap_bloques[j] == 1){
						ext_superblock->s_first_data_block = j;			// superbloque firstdatablock
						break;
					}
				}
			}
			inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] = 0xFFFF;	//inodo a bloques todos a ffff
		}
		directorio->dir_inodo = 0xFFFF;											//inodo del directorio a ffff
		return 0;
	}
	if (strcmp(nombre, "\0") == 0){
		printf("Nombre de fichero vacío, el comando se escribe como remove 'nombre del fichero'\n");
		return 1;
	}
	else{
		printf("No se ha encontrado el fichero\n");
		return 1;
	}
	
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino){
	int i , j, k, nbloques = 0, tamanyorigen, posdirnuevo, posdiroriginal = BuscaFich(directorio, nombreorigen);
	char bloqueauxiliar[SIZE_BLOQUE];
	if (strcmp(nombreorigen, "\0") == 0 || strcmp(nombredestino, "\0") == 0){
		printf("Nombre de fichero vacío, el comando se escribe como print 'nombre del fichero'\n");
		return 1;
	}
	if (BuscaFich(directorio, nombreorigen) != 0){
		directorio += BuscaFich(directorio, nombreorigen);
		while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i++] != 0xFFFF)
			nbloques++;
		ext_superblock->s_free_blocks_count -= nbloques;				//Superbloque bloques libres
		ext_superblock->s_free_inodes_count -= 1;						//superbloque inodos libres
		if(ext_bytemaps->bmap_bloques[4] == 0){
			ext_superblock->s_first_data_block = 4;
		}
		tamanyorigen = inodos->blq_inodos[directorio->dir_inodo].size_fichero;   // tamaño origen
		directorio -= posdiroriginal;
		
		for(i = 0;(directorio->dir_inodo != 0xFFFF || strcmp(directorio->dir_nfich, "" ) == 0); i++){
			if (strcmp(directorio->dir_nfich, "" ) == 0){
				break;
			}
			else{
				directorio++;
			}
		}
		posdirnuevo = i;
		for (i = 3; i<=MAX_INODOS; i++){
			if(ext_bytemaps->bmap_inodos[i] == 0){
				break;
			}
		}
		directorio->dir_inodo = i;													//inodo directorio								
		ext_bytemaps->bmap_inodos[directorio->dir_inodo] = 1;						//bmap inodos a 1
		inodos->blq_inodos[directorio->dir_inodo].size_fichero = tamanyorigen;		//tamaño copiado
		strcpy(directorio->dir_nfich, nombredestino);									//nombre puesto					
		for(i = 4, j = 0; i<=MAX_BLOQUES_DATOS && j<nbloques; i++){
			if(ext_bytemaps->bmap_bloques[i] == 0){
				ext_bytemaps->bmap_bloques[i] = 1;
				inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j++] = i;			
			}
		}	
		for(j = 0; inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] != 0xFFFF; j++){
			directorio -= posdirnuevo;
			directorio += posdiroriginal;
			memdatos = memdatos +  inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] - 4;
			for(k = 0; k <= SIZE_BLOQUE && memdatos->dato[k] != 0; k++){
				bloqueauxiliar[k] = memdatos->dato[k];				
			}
			nbloques = k - 1;							// Para no crear una variable extra
			memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] + 4;	
			directorio -= posdiroriginal;
			directorio += posdirnuevo;
			memdatos = memdatos + inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] - 4;
			for(k = 0; k <= nbloques; k++){
				memdatos->dato[k] = bloqueauxiliar[k];
				printf("%c", memdatos->dato[k]); 
			}
			if (k<=SIZE_BLOQUE){
				memdatos->dato[k] = 0;		//EOF == 0
				
			}				 	
			memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j] + 4;
		}
		printf("\n");
		return 0;	
	}
	printf("No se ha encontrado el fichero\n");
	return 1;
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
