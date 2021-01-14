/* 
	PROYECTO 2 DE SISTEMAS OPERATIVOS 2INSO A
	
	RAÚL CARRASCAL Y DANIEL LEVA
	
	SE SIMULARÁ UN SISTEMA DE FICHEROS TIPO LINUX SIMPLIFICADO 
	
	NO SE USARÁ UNA PARTICIÓN DEL DISCO SINO EL FICHERO particion.bin QUE CONTENDRÁ EL SISTEMA COMPLETO
*/

/*
Hacemos los includes de las librerías necesarias y
del archivo con las cabeceras necesarias
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

/*
Define la longitud maxima que van a poder tener los comandos introducidos, pero el nombre de fichero máximo es 17
*/

#define LONGITUD_COMANDO 100

/*
Declaración de las funciones que comprueban comandos y comparan nombres de ficheros 
*/

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
int BuscaFich(EXT_ENTRADA_DIR *directorio, char *nombre);

/*
Declaración de las funciones que se van a usar para las acciones del sistema de ficheros
*/

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino);

/*
Declaración de las funciones utilizadas para guardar las acciones introducidas por el usuario
en el fichero binario particion.bin
*/

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(){
	
	/*
	Inicialización de las variables en las que se guardarán el comando introducida en consola
	y este mismo separado en 3 partes
	*/
	
	char comando[LONGITUD_COMANDO];
	char orden[LONGITUD_COMANDO];
	char argumento1[LONGITUD_COMANDO];
	char argumento2[LONGITUD_COMANDO];
	 
	int i,j;
	unsigned long int m;
	
	/*
	Inicialización de las variables de los structs definidos en cabeceras.h
	*/
	
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    
    int entradadir;
    int grabardatos = 0, nficheros = -3;
    
	/* 
	Se inicializa un puntero para el archivo
	particion.bin que contiene el sistema	
	*/
	
    FILE *fent;
    
    /*
	Se abre el archivo particion.bin     
	*/
	
    fent = fopen("particion.bin","r+b");
    
    /*
    Se leen los datos de particion.bin 
    */
    
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);  
	 
	/*
	Se guardan los datos de particion.bin en las structs del fichero cabeceras.h bloque a bloque
	*/
	
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS*SIZE_BLOQUE);
    
    /*
	Hace un bucle infinito de modo que cada vez que se ejecute un comando
    pida nuevamente una acción tras imprimir, si fuese necesaria,
    la información pertinente. La consola solo se cierra si se ejecuta el
    comando exit.
    */
    
    for (;;){
		do {
			
		/*
		Imprime el prompt
		*/
		
		printf (">> ");
		
		/*
		Se limpia el buffer del teclado
		*/	
			
		fflush(stdin);
		
		/*
		Se guarda en la variable comando el texto introducido por el usuario
		con un límite de 100 caracteres 
		*/
		
		fgets(comando, LONGITUD_COMANDO, stdin);
		
		/*
		Se separa el comando introducido en orden, argumento1 y argumento2 y 
		se revisa que no esté vacío (solo se haya pulsado intro)
		*/ 
		
		} while (ComprobarComando(comando, orden, argumento1, argumento2) == 1);
		
		/*
		Se elige la función a la que acudir dependiendo de 
		la orden introducida por el usuario
		*/
		
		if (strcmp(orden, "info") == 0)
        	LeeSuperBloque(&ext_superblock);
        else if (strcmp(orden, "bytemaps") == 0)
        	Printbytemaps(&ext_bytemaps);
	    else if (strcmp(orden, "dir") == 0) 
            Directorio(directorio, &ext_blq_inodos);
        else if (strcmp(orden, "rename") == 0) {
            if(Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) != 1);
            
				/*
				En caso de que Renombrar devuelva 1, es decir, se haya llevado a cabo con éxito,
				se graba el sistema de ficheros actualizado en el fichero binario que contiene el sistema.
				*/
				
				grabardatos = 1;           		
    	}
		else if (strcmp(orden, "print") == 0)
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        else if (strcmp(orden, "remove") == 0){
        	if(Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1) != 1);
        	
	        	/*
				En caso de que Borrar devuelva 1, es decir, se haya llevado a cabo con éxito,
				se graba el sistema de ficheros actualizado en el fichero binario que contiene el sistema.
				*/
				
				grabardatos = 1;  
		}
		else if (strcmp(orden, "copy") == 0){
        	if(Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos,  argumento1, argumento2) != 1);
        	
	        	/*
				En caso de que Copiar devuelva 1, es decir, se haya llevado a cabo con éxito,
				se graba el sistema de ficheros actualizado en el fichero binario que contiene el sistema.
				*/  
				
				grabardatos = 1; 
		}
        else if (strcmp(orden, "exit") == 0){
        	
        	/*
        	En caso de salir, se cierra el fichero particion.bin
        	*/
        	
            fclose(fent);
            return 0;
        }
        
        /*
        Si no existiese el comando introducido se le indicaría al usuario
        */
        
        else printf("No existe el comando %s\n", orden);
        
        /*
		En caso de necesitarlo, se graba el sistema de ficheros actualizado.
		Esto solo se da cuando se le ha asignado a la variable grabardatos
		el valor 1, es decir, en las ordenes de borrar, renombrar y copiar.
		Se devuelve grabardatos a 0 para que en la siguiente orden solo se graben
		los datos si vuelve a ser necesario
		*/
		
        if (grabardatos == 1){
        	GrabarSuperBloque(&ext_superblock, fent);
        	GrabarByteMaps(&ext_bytemaps, fent);
        	Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
        	GrabarDatos(memdatos, fent);
        	grabardatos = 0;
     	}
    } 
}

/*
Función que comprueba que el comando introducido no sea un intro y,
en caso de no serlo, lo separa en orden, argumento1 y argumento2
*/

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	
	/*
	Si se hace un intro sin haber escrito nada se vuelve a imprimir el prompt
	*/
	
	if (strcmp(strcomando, "\n") == 0){
		return 1;
	}
	
	/*
	Añade el caracter fin de cadena al final del array en el que se almacena 
	el comando introducido para que después la función strtok() funcione correctamente
	*/
	
	char comandoArray[strlen(strcomando) + 1];
	strcpy(comandoArray, strcomando);
	comandoArray[strlen(strcomando) - 1] = '\0';
	
	/*
	Se separa el comando introducido en orden, argumento 1 y argumento 2 mediante strtok()
	*/
	
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

/*
Función BuscaFich, sirve para buscar en que numero de directorio se encuentra
el fichero cuyo nombre coincide con el que se le pasa a la función y para comprobar
que la longitud del nombre sea la adecuada
*/

int BuscaFich(EXT_ENTRADA_DIR *directorio, char *nombre){
	int i;
	
	/*
	Devuelve 1 si el nombre que se le pasa es mayor de
	17 caracteres que es el limite de caracteres que puede 
	contener el array que almacena el nombre
	*/
	
	if (strlen(nombre) > LEN_NFICH ){
		return 1;
	}
	
	/*
	Devuelve el valor de i tras repetir el bucle tantas veces como sea necesario
	hasta que el nombre del fichero pasado a la función sea igual que el nombre 
	contenido en el directorio i
	El bucle finaliza devolviendo 0 en caso de que al fichero no le
	corresponda ningun inodo o no tenga nombre									
	*/
	
	for(i=0; (directorio->dir_inodo != NULL_INODO || strcmp(directorio->dir_nfich, "" ) == 0); i++){
		if (strcmp(nombre, (directorio++)->dir_nfich) == 0){
			return i;
		}	
	}
	return 0;
}

/*
Función para el comando info. Recurre a prints para mostrar en consola
la información del superbloque obteniéndola desde el struct
EXT_SIMPLE_SUPERBLOCK ubicado en el archivo cabeceras.h
*/

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
	printf("Bloque %i Bytes\n", psup->s_block_size);
    printf("Inodos particion = %i\n", psup->s_inodes_count);
    printf("Inodos libres = %i\n", psup->s_free_inodes_count);
    printf("Bloques particion = %i\n", psup->s_blocks_count);
    printf("Bloques libres = %i\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %i\n", psup->s_first_data_block);
}

/*
Función para el comando bytemaps. Imprime el bytemap de 
inodos y las primeras 25 posiciones del bytemap de bloques.
*/

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
	int i;
	printf("Inodos :");
	
	/*
	Recorre el array del bytemap de inodos imprimiendo uno a uno sus valores
	*/
	
	for (i = 0; i<MAX_INODOS; i++)
		printf("%i ", ext_bytemaps->bmap_inodos[i]);
	printf("\nBloques [0-25] :");
	
	/*
	Recorre las primeras 25 posiciones del array del bytemap de bloques imprimiendo uno a uno sus valores
	*/
	
	for (i = 0; i<25; i++)
		printf("%i ", ext_bytemaps->bmap_bloques[i]);
	printf("\n");
}

/*
Función para el comando dir. 
*/

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
	int i;
	
	/*
	Mientras que el inodo del directorio actual no es FFFF, o en el caso de que lo sea,
	que no haya sido previamente borrado (la unica diferencia entre un directorio todavia no escrito
	y uno borrado es el nombre), continua haciendo directorio++
	*/
	
	while(directorio->dir_inodo != NULL_INODO || strcmp(directorio->dir_nfich, "" ) == 0){
		if (strcmp(directorio->dir_nfich, "." ) == 0 || strcmp(directorio->dir_nfich, "" ) == 0){
			
			/*
			Si es el directorio raíz o uno previamente borrado, lo saltamos
			*/
			
			directorio++;	
		}
		else{
			
			/*
			En el caso contrario, vamos imprimiendo cada uno de los datos
			*/
			
			i = 0;
			printf("%s\ttamaño:%i\tinodo:%i bloques: ", directorio->dir_nfich, inodos->blq_inodos[directorio->dir_inodo].size_fichero, directorio->dir_inodo); 
			while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] != NULL_BLOQUE){
				printf("%i ", inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i++]);
			}
			printf("\n");
			directorio++;
		}
	}
}

/*
Función para el comando rename. 
*/

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){

	/*
	Si el nombre que se intenta asignar contiene más de 17 caracteres
	entonces no se renombra el fichero y no se graban los datos
	*/
	
	if (BuscaFich(directorio, nombrenuevo) == 1){
		printf("Nombre de fichero demasiado largo, el máximo es %i\n", LEN_NFICH);
		return 1; 
	}
	
	/*
	Si el nombre que se intenta asignar ya pertenece a otro fichero
	entonces no se renombra el fichero y no se graban los datos
	*/
	
	if (BuscaFich(directorio, nombrenuevo) != 0){
		printf("El nombre de fichero ya existe\n");
		return 1;
	}
	
	/*
	Si encontramos el fichero a renombrar, se actualiza el nombre del fichero y se graban los datos
	*/
	
	if (BuscaFich(directorio, nombreantiguo) != 0){
		directorio += BuscaFich(directorio, nombreantiguo);
		strcpy(directorio->dir_nfich, nombrenuevo);
		return 0;
	}
	
	/*
	Si no se da ninguno de los casos anteriores significa 
	que se ha introducido mal el argumento1 (el correspondiente
	al nombre del fichero) y por tanto se avisa del error
	y no se graban los datos
	*/
	
	printf("No se ha encontrado el fichero\n");
	return 1;
}

/*
Función del comando print. Imprime por consola el contenido del fichero elegido
*/

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
	int i, j;
	if (BuscaFich(directorio, nombre) != 0){
		directorio += BuscaFich(directorio, nombre);
		
		/*
		Después de actualizar el directorio, por cada uno de los bloques de datos hasta que sean FFFF,
		va a mover el puntero de datos a el bloque actual - 4 ya que los primeros 4 están reservados,
		imprime el contenido del bloque hasta que acabe o hasta que llegue a 0 (EOF),
		y resetea el puntero
		*/
		
		for(i = 0; inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] != NULL_BLOQUE; i++){
			memdatos = memdatos + inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] - 4;
			for (j=0; j<= SIZE_BLOQUE && memdatos->dato[j] != 0; j++){
				printf("%c", memdatos->dato[j]);
			}
			memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] + 4;
		}
		printf("\n");
		return 0;
	}
	
	/*
	Si no se introduce ningun argumento te avisa del error y no graba los datos.
	*/
	
	if (strcmp(nombre, "\n") == 0){
		printf("Nombre de fichero vacío, el comando se escribe como print 'nombre del fichero'\n");
		return 1;
	}
	
	/*
	Si no se da ninguno de los casos anteriores significa 
	que se ha introducido mal el argumento1 (el correspondiente
	al nombre del fichero) y por tanto se avisa del error
	y no se graban los datos
	*/
	
	printf("No se ha encontrado el fichero\n");
	return 1;	
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre){									//???????????????????
	int i = 0, j;
	if (BuscaFich(directorio, nombre) != 0){	
		directorio += BuscaFich(directorio, nombre);
		
		while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i++] != NULL_BLOQUE);
		
		/*
		El programa va borrando los apartados en orden de menor a mayor importancia, primero los apartados del superbloque,
		luego el nombre, el tamaño y los bytemaps y finalmente el contenido del inodo del directorio junto al propio inodo
		*/
		
		ext_superblock->s_free_blocks_count += (i - 1);
		ext_superblock->s_free_inodes_count += 1;
		strcpy(directorio->dir_nfich , "");
		ext_bytemaps->bmap_inodos[directorio->dir_inodo] = 0;
		inodos->blq_inodos[directorio->dir_inodo].size_fichero = 0;
		for(i = 0; inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] != NULL_BLOQUE; i++){
			ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i]] = 0;
		}
		for(i = 0; i <=MAX_NUMS_BLOQUE_INODO; i++){
			if(ext_superblock->s_first_data_block == inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i]){
				for(j = 4; j<=MAX_BLOQUES_DATOS; j++){
					if(ext_bytemaps->bmap_bloques[j] == 1){
						ext_superblock->s_first_data_block = j;
						break;
					}
				}
			}
			inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] = NULL_BLOQUE;
		}
		directorio->dir_inodo = NULL_INODO;
		return 0;
	}
	
	/*
	Si no se especifica el fichero que se quiere borrar da error, avisa al usuario 
	y no graba los datos
	*/
	
	if (strcmp(nombre, "\0") == 0){
		printf("Nombre de fichero vacío, el comando se escribe como remove 'nombre del fichero'\n");
		return 1;
	}
	
	/*
	Si no se da ninguno de los casos anteriores significa 
	que se ha introducido mal el argumento1 (el correspondiente
	al nombre del fichero) y por tanto se avisa del error
	y no se graban los datos
	*/
	
	else{
		printf("No se ha encontrado el fichero\n");
		return 1;
	}
	
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino){
	int i , j, nbloques = 0, tamanyorigen, posdirnuevo, posdiroriginal = BuscaFich(directorio, nombreorigen);
	char bloqueauxiliar[SIZE_BLOQUE];
	
	/*
	Si se intenta copiar asignando un nombre de destino de más de 17 caracteres da error,
	se avisa al usuario y no se graban los datos.
	*/
	
	if (BuscaFich(directorio, nombredestino) == 1){
		printf("Nombre de fichero demasiado largo, el máximo es %i\n", LEN_NFICH);
		return 1;
	}
	
	/*
	Si no se especifica el nombre del fichero que se quieres copiar o el nombre de destino
	se avisa del error al usuario y no se graban los datos.
	En caso de que el nombre sea una s tampoco se admitirá, es un error de strtok() que pasa una s cuando en realidad esta vacío																?????????????????????????
	*/
	
	if (strcmp(nombreorigen, "\0") == 0 || strcmp(nombredestino, "\0") == 0 || strcmp(nombredestino, "s") == 0){   // Si encuentras por que coño aparece una s y tengo que filtrarla me harias muy feliz(no es para ti dani)
		printf("Nombre de fichero vacío, el comando se escribe como print 'nombre del fichero'\n");
		return 1;
	}
	if (BuscaFich(directorio, nombreorigen) != 0){
		directorio += BuscaFich(directorio, nombreorigen);
		while(inodos->blq_inodos[directorio->dir_inodo].i_nbloque[nbloques++] != NULL_BLOQUE);
		
		/*
		Copiar es parecido a borrar, pero en este caso se usan variables auxiliares para no tener que estar
		moviendose constantemente entre los 2 directorios, pero igualmente empieza por editar el superbloque,
		luego copia el tamaño original y el nombre del fichero, y finalmente copia los datos
		*/
		
		ext_superblock->s_free_blocks_count -= (nbloques--) - 1;		//Superbloque bloques libres											?????????????????????????
		ext_superblock->s_free_inodes_count -= 1;						//superbloque inodos libres
		if(ext_bytemaps->bmap_bloques[4] == 0){
			ext_superblock->s_first_data_block = 4;
		}
		tamanyorigen = inodos->blq_inodos[directorio->dir_inodo].size_fichero;   // tamaño origen
		directorio -= posdiroriginal;
		
		for(i = 0;(directorio->dir_inodo != NULL_INODO || strcmp(directorio->dir_nfich, "" ) == 0); i++)
			directorio++;
		posdirnuevo = i;
		
		/*
		Guarda aqui la posición del nuevo directorio, este bucle prioriza los directorios previamente borrados para
		que no queden espacios vacios entre medias
		*/
		
		for (i = 3; i<=MAX_INODOS; i++){
			if(ext_bytemaps->bmap_inodos[i] == 0){
				break;
			}
		}
		
		/*
		Busca el primer inodo libre y se lo asigna
		*/
		
		directorio->dir_inodo = i;							
		ext_bytemaps->bmap_inodos[directorio->dir_inodo] = 1;
		inodos->blq_inodos[directorio->dir_inodo].size_fichero = tamanyorigen;
		strcpy(directorio->dir_nfich, nombredestino);
		
		/*
		Edita el bytemap de bloques
		*/
						
		for(i = 4, j = 0; i<=MAX_BLOQUES_DATOS && j<nbloques; i++){
			if(ext_bytemaps->bmap_bloques[i] == 0){
				ext_bytemaps->bmap_bloques[i] = 1;
				inodos->blq_inodos[directorio->dir_inodo].i_nbloque[j++] = i;			
			}
		}
		
		/*
		En el último bucle copia los datos, primero resetea el puntero y lo mueve al directorio original, copia el primer
		bloque de datos a un array auxiliar, resetea de nuevo ambos punteros(directorio y datos), mueve el puntero de directorio
		a el nuevo directorio y guarda los datos de el array auxiliar en este, esto se repite por cada uno de los bloques que ocupe
		el fichero.
		*/
		
		for(i = 0; inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] != NULL_BLOQUE; i++){
			directorio -= posdirnuevo;
			directorio += posdiroriginal;
			memdatos = memdatos +  inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] - 4;
			for(j = 0; j < SIZE_BLOQUE && memdatos->dato[j] != 0; j++)
				bloqueauxiliar[j] = memdatos->dato[j];	 				
			nbloques = j;							// Para no crear una variable extra
			memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] + 4;
			directorio -= posdiroriginal;
			directorio += posdirnuevo;
			memdatos = memdatos + inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] - 4;
			for(j = 0; j < nbloques; j++){
				memdatos->dato[j] = bloqueauxiliar[j];
			}
			
			/*
			Al final del último bloque, guarda un 0 para indicar que es el EOF
			*/
			
			if (j<SIZE_BLOQUE){
				memdatos->dato[j] = 0;
			}				 	
			memdatos = memdatos - inodos->blq_inodos[directorio->dir_inodo].i_nbloque[i] + 4;
		}
		return 0;	
	}
	
	/*
	Si no se da ninguno de los casos anteriores significa 
	que se ha introducido mal el argumento1 (el correspondiente
	al nombre del fichero) y por tanto se avisa del error
	y no se graban los datos
	*/
	
	printf("No se ha encontrado el fichero\n");
	return 1;
}

/*
Hace un rewind y escribe en el fichero particion.bin los datos del superbloque con fwrite
*/

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
	rewind(fich);
	fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

/*
Graba en particion.bin los datos de los bytemaps 
*/

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

/*
Graba en particion.bin los inodos y el directorio
*/

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
	fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

/*
Graba los datos del fichero en particion.bin
*/

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}
