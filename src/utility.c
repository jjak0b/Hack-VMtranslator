#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <dirent.h>


bool strEndWith( char *str, char *str_suffix){

	int length_str = strlen( str ) - 1;
	int length_suffix = strlen( str_suffix ) - 1;
	int isRight = true;

	while( length_suffix >= 0 && length_str >= 0 && isRight){
		if( str[ length_str ] != str_suffix[ length_suffix ] ){
			isRight = false;
		}
		length_str -= 1;
		length_suffix -= 1;
	}
	return isRight;
}

/**
 * @brief
 * PreCondition: 	N/A
 * PostCondition: 	dato il nome di un file.estensione come e un list_node,
 * 					inserisce nella coda i caratteri letti dal file specificato
 * @param *filename 
 * @param *list_node
 * @return NULL se è impossbile aprire il file
 */
list_node *readFile( char *filename, list_node *l_handler ){
    FILE *fin;
    char c = '\0';
	char *ptr_char = NULL;
    fin = fopen( filename, "r");
    if( fin != NULL){
        while( !feof( fin ) ){
			if( fscanf( fin, "%c", &c) == 1 ){
				#ifdef DEBUG
				// printf("letto: '%c' -> ASCII: %d\n", c, (int)c);
				#endif				
				ptr_char = (char*)malloc( sizeof(char) );
				*ptr_char = c;
				l_handler = push( l_handler, ptr_char );
			}
        }
        fclose( fin );
		l_handler = list_node_reverse( l_handler );
    }
    else{
        return NULL;
    }

    return l_handler;
}

/**
 * @brief 
 * PreCondition: I 'value' puntati dai 'list_node', devono essere convertibili a char
 * PostCondition: 	Dato il nome di un file.estensione e un list_node,
 * 					scrive gli elementi della lista in un file con il nome indicato ( se non esite, viene creato )
 * @param filename 
 * @param l_handler 
 * @return false se è impossbile aprire/scrivere il file, true altrimenti
 */
bool writeFile( char *filename, list_node *head ){
	FILE *fout;
	char *ptr_char = NULL;
    fout = fopen( filename, "w");
    if( fout != NULL){
        while( !feof( fout ) && head != NULL){
			ptr_char = head->value;
			fprintf( fout, "%c", *ptr_char );
			head = head->next;
        }
        fclose( fout );
    }
    else{
        return false;
    }
    return true;
}

void *replaceFilenameExtension( char filename[], int size_filename, const char extension[] ){
	int length = strnlen( filename, size_filename );
	int i = length ;
	bool b_quit = false;
	while( i >= 0 && !b_quit){
		if( filename[ i ] == '.' ){
			filename[ i ] = '\0';
			b_quit = true;			
		}		
		i-= 1;
	}

	strncat( filename, extension, strlen(extension) );
}
/**
 * @brief indica se la stringa (o singolo carattere ) passata è un numero
 * PreCondition: str deve essere una stringa con già inserito '\0'
 * @param str 
 * @param isStr Indica se il puntatore punta a una stringa se true, oppure a un carattere se false
 * @return true se tutti i caratteri sono codificati in cifre da 0 a 9
 * @return false altrimenti
 */
bool isNumber( char *str, bool isStr){
	if( str == NULL ){
		return false;
	}
	else if( str[0] == '\0'){
		return false;
	}
	else{
		if( str[0] >= '0' && str[0] <= '9'){
			if( isStr && str[1] != '\0'){
				return true && isNumber( &str[1], isStr );
			}
			else{
				return true;
			}
		}
		else{
			return false;
		}
	}
}

/*To string*/

/**
 * @brief 
 * PreCondition: 	destination_size >= 2
 * 				 	se numero di caratteri di value > destination_size allora verranno inserite solo le prime 'destination_size-1' cifre meno significative nel buffer
 * 					poichè verrà inserito il carattere '\0' alla posizione destination_size-1
 *						es int_to_strbuffer( 1234, destination, 4 ) -> destination = "234".
 * PostCondition: 	Inserisce nell buffer destination le cifre del valore passato come caratteri
 * @param value 
 * @param destination 
 * @param destination_size 
 * @return true  se sono state inserite nel buffer tutte le cifre di value
 * @return false se numero di caratteri di value > destination_size 
 * 					
 */
bool int_to_strbuffer( int value, char *destination, int destination_size ){
	if( destination_size  < 1){
		return false;
	}
	else if( destination_size < 2){
		destination[0] = '\0';
		return false;
	}
	else{
		int state = true;
		if( value / 10 ==  0 ){ // value ha una cifra
			destination[0] = (char)(48+value);
			destination[1] = '\0';
		}
		else{
			state = int_to_strbuffer( value / 10 , destination, destination_size - 1 );
			 // concateno la/le cifra/e precedenti con quella appena estratta
			char tmp[2];
			int ultima_cifra = value % 10;
			tmp[ 0 ] = (char)(48+ultima_cifra);
			tmp[ 1 ] = '\0';
			strncat( destination, tmp, destination_size );
		}
		return state;
	}
}

int countDigits( int n ){
	if( n == 0 ){
		return 1;
	}
	else{
		int c = 0;
		while(n != 0 ){
			n = n / 10;
			c += 1;
		}
		return c;
	}
}

/**
 * @brief Converte un numero intero in una stringa, contenente come caratteri le cifre del numero dato
 * PreCondition: n > 0
 * @param n 
 * @return char* : il puntatore alla stringa (contenente \0)
 */
char *int_to_string( int n ){
	int n_digit = countDigits( n );
	char *str = (char*)malloc( sizeof(char) * ( n_digit + 1 ) );
	
	for( int i = n_digit-1; i >= 0; i -= 1 ){
		str[i] = (char)((int)'0' + (n % 10));
		n = n / 10;
	}
	str[ n_digit ] = '\0';
	return str;
}


/**
 * @brief coverte una lista a una stringa
 * PreCondition: list_node *head deve contenere nodi aventi dei char come valori puntati 
 *				 size_str deve essere = NULL se non si vuole ottenere la dimensione
 * PostCondition: Ritorna il puntatore alla stringa creata con i valori della lista passata
 *				  inoltre se in size_str viene passato un indirizzo, viene assegnato il valore della dimensione all'indirizzo puntato
 * @param head : il nodo da cui partire per ottere la stringa
 * @param size_str : dimensione dell'array contente la stringa
 * @return char* 
 */
char *list_to_string( list_node *head, int *size_str ){
	int size_s = size(head, true );
	if( size_str != NULL ){
		*size_str = size_s;
	}
	char *c = NULL;
	char *buffer = (char*)malloc( sizeof(char)*( size_s +1));
	// sicuramente non ho head = NULL nei primi size_str elementi
	for( int i = 0; i < size_s ; i+= 1){
		c = (char*)head->value;
		buffer[ i ] = *c;
		head = head->next;
	}
	buffer[size_s ] = '\0';
	return buffer;
}

/**
 * @brief Data una lista contenente booleani (bool definito da questa libreria), restituisce il puntatore alla stringa che ne contenente le cifre booleane '1'-'0'
 * PreCondition: 	head deve essere una lista dedicata a valori di tipo bool
 * 					size_str = NULL se non si vuole ottenere la dimensione allocata alla stringa
 * PostCondition: 	nel variabile puntata da size_str viene memorizzata la dimensione allocata della stringa restituita
 * @param head puntatore alla lista dedicata a valori di tipo bool
 * @param size_str indirizzo della variabile in cui memorizzare la dimensione della stringa
 * @return char* 
 */
char *list_binary_to_string( list_node *head, int *size_str ){
	list_node *tmp = head;
	int size_s = size(head, true ) + 1;
	if( size_str != NULL ){
		*size_str = size_s;
	}
	char *str = (char*)malloc( sizeof( char ) * ( size_s ) );
	bool *bit = NULL;
	for( int i = 0; i < size_s - 1; i += 1 ){
		bit = (bool*)tmp->value;
		if( !*bit ){
			str[ i ] = '0';
		}
		else{
			str[ i ] = '1';
		}
		tmp = tmp->next;
	}
	str[ size_s - 1 ] = '\0';
	return str;
}
/*To list*/

/**
 * @brief crea una lista con (una copia de)gli elementi della stringa (eccetto '\0')
 * 	PreCondition: str deve essere una stringa con '\0' già inserito
 * @param str 
 * @return list_node* 
 */
list_node *string_to_list( char str[] ){
	if( str == NULL ){
		return NULL;
	}
	else{
		int n = strlen( str );
		if( n > 0 ){
			char *c = NULL;
			list_node *head = NULL;
			for(int i = 0; i < n; i += 1 ){
				c = (char*)malloc(sizeof(char)); // istanzio un nuovo carattere e copio il valore da str per evitare problemi ( vedere sotto)
				*c = str[i];
				head = enqueue( head, c ); // se fosse handler = enqueue( handler, &str[i] ); probabilemnte mi causera dei pending pointers nelle liste che puntano alla lista se faccio un suo free dal chiamante
			}
			return head;
		}
	}

	return NULL;
}

/**
 * @brief Dato un numero restituisce il puntatore alla testa della lista contenente la lista delle cifre intere (allocate) del valore intero dato
 * PreCondition: n >= 0
 * @param n 
 * @return list_node* 
 */
list_node *int_to_list( int n ){
	list_node *head =  NULL;
	int *n_tmp = (int*)malloc( sizeof( int ) );
	if( n == 0 ){
		*n_tmp = 0;
		list_node *head = list_node_new( n_tmp );
	}
	else{		
		*n_tmp = n % 10;
		n = n / 10;
		list_node *head = list_node_new( n_tmp );
		while( n != 0 ){
			n_tmp = (int*)malloc( sizeof( int ) );
			*n_tmp = n % 10;
			list_node *head_new = list_node_new( n_tmp );
			head = insert( head, head_new, 0 );
			n = n / 10;		
		}
	}
	return head;
}

/**
 * @brief dato un valore e un numero massimo di bit, resitituisce la lista delle cifre booleane (allocate) codificate dal valore intero dato in sequenza binaria
 * PreCondition: n > 0
 * PostCondition: Sono inseriti nella lista solo i primi max_bit_count bit (ovvero di tipo bool) più significativi
 * @param n : l'intero da convertire in binario
 * @param max_bit_count Il numero di bit massimi da ottenere
 * @return list_node*  puntatore alla testa della lista contenente le cifre booleane (allocate) codificate dal valore intero dato
 */
list_node *int_to_binary_list( int n, int unsigned max_bit_count ){
	list_node *binary = NULL;
	bool *bit = NULL;
	int c = 0;
	while( c < max_bit_count /*n!=0*/ ){
		bit = (bool*)malloc( sizeof( bool ) );
		*bit = n % 2;
		n = n / 2;
		binary = enqueue( binary, bit );
		c += 1;
	}
	binary = list_node_reverse( binary );
	return binary;
}

/**
 * @brief indica se subStr è una sottostringa in str
 * Precondition:  str e subStr devono essere stringhe con '\0'
 * 				  se start_index != NULL verrà memorizzato l'indice dove inizia subStr in str
 * @param str 
 * @param subStr 
 * @return true se subStr è una sotto stringa di str
 * @return false altrimenti
 */
bool isSubstr(const char *str, const char *subStr, int *start_index) {
	int length_str = strlen( str );
	int length_subStr = strlen( subStr );
    int n = length_str - length_subStr;
	bool isSub = false;
	// confronta ogni gruppo a length_subSt a length_subSt
	// se la differenza è 0 allora subStr è sotto stringa di str
	// altrimenti ricomincia il procedimento cominciando dal carattere successivo finchè i < n + 1
	int i = 0;
    while( i < n+1 && !isSub ) {
        if ( !strncmp(str + i, subStr, length_subStr ) ) {
			if( start_index != NULL ){
				*start_index = i;
			}
            isSub = true;
        }
		i += 1;
    }

    return isSub;
}

/**
 * @brief Ignora i caratteri specificati consecutivi prima e dopo il contenuto significativo; memorizza in start_index l'indice di partenza e in end_index quello di fine della stringa
 * PreCondition: str deve essere una stringa
 * PostCondition: memorizza in start_index l'indice di partenza e in end_index quello di fine della stringa se loro sono != NULL
 * @param str 
 * @param ignore_char 
 * @return int : la lunghezza effettiva della stringa significativa
 */
int getStrLimitIndexes( const char *str, char ignore_char, int *start_index, int *end_index){
	int length_str = strlen( str );
	bool b_found_valid = false;
	int _start_index = 0, _end_index = length_str - 1;
	// ignora i caratteri specificati prima del contenuto significativo, e ne memorizzo l'indice di partenza della stringa
	for( int i = 0; i < length_str && !b_found_valid; i+=1 ){
		if( str[i] != ignore_char){
			b_found_valid = true;
		}
		else{
			_start_index += 1;
		}
	}
	// ignora i caratteri specificati a partire dalla fine, e ne memorizzo l'indice della fine significativa della stringa
	b_found_valid = false;
	for( int i = length_str - 1; i >= _start_index && !b_found_valid; i-=1 ){
		if( str[i] != ignore_char){
			b_found_valid = true;
		}
		else{
			_end_index -= 1;
		}
	}

	if( start_index != NULL){
		*start_index = _start_index;
	}

	if( end_index != NULL){
		*end_index = _end_index;
	}

	length_str -= ( (length_str - 1 ) - _end_index);
	length_str -= _start_index;

	return length_str;
}

/**
 * @brief restituisce una lista di sotto stringhecontenute tra un delimitatoree specificato
 * 			esempio: strWords( "c_ia_o", "_" ) := "c" -> "ia" -> "o"
 * PreCondition: str e str_delimitator devono essere stringhe != NULL
 * PostCondition: I valori della lista sono allocati dinamicamente in questa funzione.
 * 
 * @param str 
 * @param str_delimitator 
 * @return list_node* la lista delle parole tra il delimitatore
 */
list_node *strWords( const char *str, const char *str_delimitator ){

	list_node *words = NULL;
	char *str_tmp = NULL;
	int length_str = strlen( str ), length_delim = strlen( str_delimitator );
	int start_index = 0, tmp_index = 0, length_str_tmp = 0;
	while( start_index < length_str && isSubstr( str + start_index, str_delimitator, &tmp_index) ){// ogni ciclo la stringa inizia dal carattere successivo del delimitatore
		
		// memorizzo i primi tmp_index caratteri a partire dallo start_index-esimo caratere
		length_str_tmp = tmp_index;
		if( length_str_tmp > 0 ){
			str_tmp = malloc( sizeof(char) * length_str_tmp + 1);

			strncpy( str_tmp, str + start_index, length_str_tmp );
			str_tmp[ length_str_tmp ] = '\0';

			words = push( words, str_tmp );
		}
		start_index += tmp_index + length_delim; // inizio la stringa dopo i caratteri memorizzati e dopo il delimitatore
	}

	// prendo il contenuto restante della stringa e lo considero una parola
	length_str_tmp = length_str - start_index;
	if( length_str_tmp > 0 ){
		str_tmp = malloc( sizeof(char) * length_str_tmp + 1);

		strncpy( str_tmp, str + start_index, length_str_tmp );
		str_tmp[ length_str_tmp ] = '\0';

		words = push( words, str_tmp );
	}

	return list_node_reverse( words );
}

/**
 * @brief 	Restituisce gli elementi della lista come array di puntatori,
 * PreCondition: 	Input deve essere una lista di stringhe; se non si vuoloe ottenere la dimensione dell'array size_array deve essre = NULL
 * PostCondition: 	Se b_copy = true i puntatori dell'array puntano alle COPIE degli elementi puntati dai nodi;
 * 					altrimenti puntano durettamente ai valori puntati dai nodi
 *					Se size_array != NULL allora *size_array = size( input, true )
 * @param input : lista di stringhe
 * @param size_array : puntatore alla dimensione dell'array
 * @param b_copy : indica se effettuare o meno la copia dei valori ei nodi della lista
 * @return char** 
 */
char **list_toArrayStr( list_node *input, unsigned int *size_array, bool b_copy ){
	list_node *tmp = input;
	int size_list = size( tmp, true ), length_str = 0;
	char **rows = malloc( sizeof( char* ) * size_list ); // Array di puntatori
	for( int i = 0; i < size_list; i += 1 ){
		length_str = strlen( tmp->value );
		if( b_copy ){
			rows[i] = malloc( sizeof( char ) * (length_str + 1 ) );
			strncpy( rows[i], tmp->value, length_str );
			rows[length_str] = '\0';
		}
		else{
			rows[ i ] = tmp->value;
		}
		tmp = tmp->next;
	}

	if( size_array != NULL ){
		*size_array = size_list;
	}

	return rows;
}

/**
 * @brief Copia e parte significativa di str in una nuova allocazione di memoria
 * PreCondition: str != NULL
 * @param str 
 * @return char* 
 */
char *strDuplicate( const char *str ){
	if( str == NULL ){
		return NULL;
	}
	int length = strlen( str );
	char *new_str = malloc( sizeof( char ) * length + 1 );
	strcpy( new_str, str );
	new_str[ length ] = '\0';
	return new_str;
}

/**
 * @brief Istanzia una nuova stringa con il nome del file a partire dal percorso dato
 * PostCondition: il valore restituito è una nuova stringa istanziata
 * @param filePath percorso file da cui recuperare il nome del file
 * @return char* puntatore alla nuova stringa istanziata
 */
char* getFileNameFromPath( char *filePath, bool b_getExtension){
	if( filePath == NULL ){
		return NULL;
	}
	int length = strlen( filePath );
	bool b_found = false;
	char *str_fileName = NULL;
	for( int i = length - 1; i >= 0 && !b_found; i-=1 ){
		if( filePath[i] == FILE_PATH_SEPARATOR ){
			str_fileName = strDuplicate( filePath + i + 1 );
			b_found = true;
		}
	}
	if( !b_found ){
		str_fileName = strDuplicate( filePath );
	}

	if( !b_getExtension ){
		b_found = false;
		length = strlen( str_fileName );
		for( int i = length- 1; i >= 0 && !b_found; i-=1 ){
			if( str_fileName[i] == '.' ){
				str_fileName[i] = '\0';
				b_found = true;
			}
		}	
	}
	return str_fileName;
}

/**
 * @brief restituisce il puntatore ad una striga istanziata, contenente le lettere maiuscole di tutte le lettere contenute nella stringa data
 * PreCondition: str deve essere una stringa
 * PostCondition: alloca in memoria una nuova stringa
 * @param str 
 * @return char* 
 */
char *strToUpperCase( char *str ){
	int length = strlen( str );
	if( length > 0 ){
		char *str_upperCase = malloc( sizeof( char) * ( length + 1 ) );
		for( int i = 0; i < length; i+=1 ){
			if( str[i] >= 'a' || str[i] <= 'z' ){
				str_upperCase[i] = str[i] - 32; // 32 è valore ashii di differenza tra lettere aschii, ad esempio tra'A' e 'a'
			}
			else{
				str_upperCase[i] = str[i];
			}
		}
		str_upperCase[ length ] = '\0';
		return str_upperCase;
	}
	else{
		return NULL;
	}
}

/**
 * @brief restituisce il puntatore ad una striga istanziata, contenente le lettere minuscole di tutte le lettere contenute nella stringa data
 * PreCondition: str deve essere una stringa
 * PostCondition: alloca in memoria una nuova stringa
 * @param str 
 * @return char* 
 */
char *strToLowerCase( char *str  ){
	int length = strlen( str );
	if( length > 0 ){
		char *str_lowerCase = malloc( sizeof( char) * ( length + 1 ) );
		for( int i = 0; i < length; i+=1 ){
			if( str[i] >= 'A' || str[i] <= 'X' ){
				str_lowerCase[i] = str[i] + 32; // 32 è valore ashii di differenza tra lettere aschii, ad esempio tra'A' e 'a'
			}
			else{
				str_lowerCase[i] = str[i];
			}
		}
		str_lowerCase[ length ] = '\0';
		return str_lowerCase;
	}
	else{
		return NULL;
	}
}

/**
 * @brief verifica se il percorso dato è di un file (altrimenti cartella)
 * PreCondition: la funzione accede alla cartella e la chiude utilizzando funzioni di libreria dirent.h
 * @param name 
 * @return true 
 * @return false 
 */
bool isFile(const char* str_path){
	DIR* directory = opendir(str_path);

	if(directory != NULL)
	{
		closedir(directory);
		return false;
	}
	return true;
}
