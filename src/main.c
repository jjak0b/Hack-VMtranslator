#include <stdio.h>
#include <stdlib.h>

/*
	Convenzioni usate:
		b_nomevar / isNomeVar 	-> usata per variabili booleane
		n_nomevar 				-> usata (non spesso) per indicare un intero
		str_nomevar 			-> usata per stringhe ( array di char con '\0' finale )
		lenght_nomevar			-> usata in genere nelle stringhe per indicare la lunghezza della stringa prima del carattere '\0'
		size_nomevar			-> usata per indicare la dimensione totale di una lista o array ( nelle stringhe quindi indica la dimensione allocata all'array )
		lh_nomevar 				-> usata per indicare una struttura di tipo list_handler
		l_nomevar 				-> usata per indicare una struttura di tipo lista
	Consiglio per chi dovrà esaminare il codice: Consiglio vivamente di compilarlo decommentando la definizione di 'DEBUG' in utility.h
	Inoltre per file aventi tantissime istruzioni, (quando superano le migliaia) potrebbe accadere che l'assembler ci metta qualche minuto per codificare le istruzioni
	Questo è dovuto al fatto che non è molto ottimizzato
*/
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "list.h"
#include <string.h>

#define FILE_INPUT_EXTENSION ".vm" // estensione personalizzabile
#ifndef FILE_INPUT_EXTENSION
#define FILE_INPUT_EXTENSION ".vm"
#endif
#define FILE_OUTPUT_EXTENSION ".asm" // estensione personalizzabile
#ifndef FILE_OUTPUT_EXTENSION
#define FILE_OUTPUT_EXTENSION ".asm"
#endif

typedef struct symbol{
	char *key; // identificatore del simbolo
	int value; // valore del simbolo
} symbol;

/**
 * @brief Istanzia un nuovo simbolo assegnando i valori specificati
 * PreCondition: key != NULL
 * @param key : identificatore del simbolo
 * @param value : valore del simbolo
 * @return symbol* puntatore al nuovo simbolo istanziato
 */
symbol *new_symbol( char *key, int value){
	symbol *s = malloc(sizeof( symbol ) );
	s->key = key;
	s->value = value;
	return s;
}

/**
 * @brief Stampa a schermo le informazioni relative ad ogni simbolo della lista a partire dal nodo dato
 * 
 * @param head 
 */
void print_symbols( list_node *head ){
	symbol *s = NULL;
	if( head != NULL ){
		s = (symbol*)head->value;
		if( s != NULL ){
			printf("key: '%s'\tvalue: '%d'\n", s->key, s->value);
		}
		print_symbols( head->next );
	}
	else{
		printf("\n");
	}
}

/**
 * @brief restituisce se presente il puntatore ad una struttura symbol presente nella lista data con la str_label specificata, altriementi NULL
 * PreCondition: lh_symbol_table deve contenere ua lista, di cui ogni nodo punta ad una struttura symbol
 * @param l_symbol_table Handler della lista di symbol in cui cercare
 * @param str_label chiave o identificatore del simbolo
 * @return symbol* puntatore della struttura symbol
 */
symbol *getFromSymbolTable( list_node *l_symbol_table, char *str_label ){
	if( l_symbol_table == NULL ){
		return NULL;
	}
	else if( str_label == NULL ){
		return NULL;
	}

	symbol *s = (symbol*)l_symbol_table->value;
	symbol *s_replace = NULL;

	bool b_found_known = false;
	while( l_symbol_table != NULL && !b_found_known ){
		s = (symbol*)l_symbol_table->value;
		if( !strcmp( s->key, str_label ) ){
			s_replace = s;
			b_found_known = true;
			#ifdef DEBUG
			printf("Trovato simbolo %s in lista: %d\n", s->key, s->value);
			#endif
		}
		l_symbol_table = l_symbol_table->next;
	}

	return s_replace;
}

/**
 * @brief : Accumula ogni carattere fino al primo "\n" e lo converte in stringa; sostituisce '\t' con ' ',  sequenze  di ' ' lasciandone solo 1; ignora tutti i caratteri dopo '\r', commenti "//"
 * PreCondition: La lista deve essere una lista di caratteri
 * @param input 
 * @return list_node* : il puntatore della lista con gli elementi rimossi, altrimenti NULL se è avvenuto un errore di sintassi per i commenti (con relativo messaggio stampato) o se la lista è vuota
 */
list_node *set_content_to_simple_vm_format( list_node *input ){
	list_node *tmp = input;
	list_node *output = NULL;
	list_node *buffer = NULL;
	int unsigned row = 1;
	char *str_buffer = NULL;
	int tmp_index = -1;
	bool b_error = false;
	char *ptr_value = NULL;
	while( !b_error && tmp != NULL ){
		ptr_value = tmp->value;
		if( *ptr_value == '\n' || tmp->next == NULL ){
			if( tmp->next == NULL ){ // caso in cui non ci sia un carattere di nuova riga
				buffer = push( buffer, tmp->value );
			}
			tmp_index = -1;
			buffer = list_node_reverse( buffer );
			str_buffer = list_to_string( buffer, NULL );
			int length_str = strlen( str_buffer );
			// printf( "Size:%d , str[0]=%d ,MYSTRING: %s\n", strlen( str_buffer ), str_buffer[0], str_buffer );
			if( str_buffer[0] != '\0' ){
				if( isSubstr( str_buffer, "//", &tmp_index ) && tmp_index >= 0 ){ // ignoro il contenuto dopo "//"
					str_buffer[ tmp_index ] = '\0';
					tmp_index = -1;
				}

				if( isSubstr( str_buffer, "\r", &tmp_index ) && tmp_index >= 0){ // ignoro il contenuto dopo "\r" dato che sarebbe il delimitatore di riga
					str_buffer[ tmp_index ] = '\0';
					tmp_index = -1;
				}

				if( str_buffer[0] != '\0' ){ // se il commento era ad inizio riga, ignora totalmente quest'ultima
					if( isSubstr( str_buffer, "/", &tmp_index ) && tmp_index >= 0 ){ // errore di sintassi per commento non valido dopo che ho ignorato i commenti validi
						printf( "ERRORE: Sintassi commento non valida a riga: %d, carattere: %d\n\"%s\"\n", row, tmp_index, str_buffer );
						b_error = true;
						tmp_index = -1;
					}
					if( !b_error ){
						int start_index = 0, end_index = 0;
						while( isSubstr( str_buffer, "\t", &tmp_index ) && tmp_index >= 0){ // Sostituisco tutte le tabulazioni con degli spazi
							str_buffer[ tmp_index ] = ' ';
							tmp_index = -1;
						}
						while( isSubstr( str_buffer, "  ", &tmp_index ) && tmp_index >= 0 ){ // Sostituisco le doppie spaziature con una singola
							strcpy( str_buffer + tmp_index + 1,  str_buffer + tmp_index + 2 );
							tmp_index = -1;
						}
						// ignora gli spazi prima del contenuto significativo, e ne memorizzo l'indice di partenza della stringa
						int length_str_tmp = getStrLimitIndexes( str_buffer, ' ', &start_index, &end_index );
						// printf( "start valid %d \t end valid: %d\n", start_index, end_index );
						if( length_str != length_str_tmp ){
							// Mi creo una stringa con i caratteri essenziali, senza la parte meno significativa
							char *str_tmp = malloc( sizeof( char ) * (length_str_tmp + 1 ) );
							strncpy( str_tmp, str_buffer + start_index, length_str_tmp );
							str_tmp[ length_str_tmp ] = '\0';
							free( str_buffer );
							str_buffer = str_tmp;
						}
					}
					if( str_buffer[0] != '\0' ){
						#ifdef DEBUG
						printf( "RIGA %d Elaborata: \"%s\"\n", row, str_buffer);
						#endif
						output = push( output, str_buffer );
					}
					#ifdef DEBUG
					else{
						printf( "RIGA %d elaborata e ignorata: \"", row);
						list_node_print( "%c", buffer );
						printf( "\"\n" );
					}
					#endif
				}
				#ifdef DEBUG
				else{
					printf( "RIGA %d ignorata: ", row);
					list_node_print( "%c", buffer );
					printf( "\n");
				}
				#endif
			}
			if( str_buffer[0] == '\0' ){
				free( str_buffer);
				str_buffer = NULL;
			}
			row +=1;
			delete_list( buffer, false );
			buffer = NULL;
		}
		else{
			buffer = push( buffer, tmp->value );
		}
		tmp = tmp->next;
	}

	if( b_error ){
		delete_list( output, true ); // libera la memoria usata
		return NULL;
	}
	return list_node_reverse( output );
}
/**
 * @brief Da una lista di caratteri ritorna una lista contenente stringhe costituite dalla COPIA dei caratteri della lista data
 * PreConduition: 	La lista deve essera una lista di caratteri; per delimitare il fine stringa, si utilizza il carattere '\n' e non "\r\n", il quale non sarà aggiunto alla stringa
 * PostCondition: 	I caratteri ' ' a inizio riga saranno ignorati
 * @param input 
 * @return list_node* 
 */
list_node *setup_list_char_to_list_str( list_node *input){
	list_node *tmp = input;
	list_node *buffer = NULL;
	list_node *output = NULL;
	while( tmp != NULL ){
		if(*(char*)tmp->value == '\n' || tmp->next == NULL ){
			if( tmp->next == NULL ){
				buffer = push( buffer, tmp->value);
			}
			buffer = list_node_reverse( buffer );
			output = push( output, list_to_string( buffer, NULL ) );
		}
		else{
			buffer = push( buffer, tmp->value);
		}
		tmp = tmp->next;
	}
	return list_node_reverse( output );
}

/**
 * @brief Da una lista di stringhe ritorna una lista contenente una COPIA dei suoi caratteri
 * PreConduition: 	la lista deve essera una lista di stringhe
 * PostCondition: 	Alla fine di ogni stringa, aggiunge la sequenza di caratteri "\r\n";
 * 					se una stringa è vuota invece non viene aggiunta
 * @param input 
 * @return list_node* 
 */
list_node *setup_list_str_to_list_char( list_node *input){
	list_node *tmp = input;
	list_node *output = NULL;
	int length_str = 0;
	list_node *list_string = NULL;
	char *str_tmp = NULL;
	while( tmp != NULL ){
		length_str = strlen( tmp->value );
		if( length_str > 0 ){
			str_tmp = malloc( sizeof(char) * ( length_str + 2 + 1) );
			// aggiungo "\r\n" a fine stringa per delimitare il fine riga
			strcpy( str_tmp, tmp->value );
			strcat( str_tmp, "\r\n");
			list_string = string_to_list( str_tmp );
			// dealloco lo spazio che ho utilizzato
			free( str_tmp );
			str_tmp = NULL;
			// collego la lista di caratteri alla lista di output
			output = append( output, list_string );
		}
		tmp = tmp->next;
	}
	return output;
}

list_node *ASM_IncSP( list_node *output ){
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "M=M+1" ) );
	return output;
}

list_node *ASM_DecSP( list_node *output ){
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "M=M-1" ) );
	return output;
}

list_node *ASM_push( list_node *output, char *str_segment, char *index ){
	if( !isNumber( index, true ) ){
		return NULL;
	}
	int length_str_index = strlen( index );
	char *str_tmp = NULL;

	// genero l'indice della variabile
	if( strcmp( str_segment, "static") ){ // @index
		str_tmp = malloc( sizeof( char ) *  length_str_index + 2 );
		strcpy( str_tmp, "@"); 
		strncpy( str_tmp + 1, index, length_str_index );
		str_tmp[ length_str_index] = '\0';
		output = push( output, str_tmp ); // addr = START_SEGMENT + i
	}
	else{ // @static.index
		length_str_index = (strlen( "static." ) + length_str_index );
		str_tmp = malloc( sizeof( char ) * (length_str_index + 2) );
		strcpy( str_tmp, "@static."); 
		strcat( str_tmp, index );
		output = push( output, str_tmp );
	}

	if( strcmp( str_segment, "static" ) ){
		output = push( output, strDuplicate( "D=A" ) );
		if( !strcmp( str_segment, "constant") ){
			output = push( output, strDuplicate( "@0" ) );
		}
		else if( !strcmp( str_segment, "local") ){
			output = push( output, strDuplicate( "@LCL" ) );
		}
		else if( !strcmp( str_segment, "argument") ){
			output = push( output, strDuplicate( "@ARG" ) );
		}
		output = push( output, strDuplicate( "A=A+D" ) );  	// addr = START_SEGMENT + i
	}

	output = push( output, strDuplicate( "D=M" ) );		// *SP = *addr
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "A=M" ) );
	output = push( output, strDuplicate( "M=D" ) );

	return ASM_IncSP( output ); // SP++
}

list_node *ASM_pop( list_node *output, char *str_segment, char *index ){
	if( !isNumber( index, true ) ){
		return NULL;
	}
	int length_str_index = strlen( index );
	char *str_tmp = NULL;
	// genero l'indice della variabile
	
	if( strcmp( str_segment, "static") ){ // @index
		str_tmp = malloc( sizeof( char ) *  length_str_index + 2 );
		strcpy( str_tmp, "@"); 
		strncpy( str_tmp + 1, index, length_str_index );
		str_tmp[ length_str_index] = '\0';
		output = push( output, str_tmp ); // addr = START_SEGMENT + i
	}
	else{ // @static.index
		length_str_index = (strlen( "static." ) + length_str_index );
		str_tmp = malloc( sizeof( char ) * (length_str_index + 2) );
		strcpy( str_tmp, "@static."); 
		strcat( str_tmp, index );
		output = push( output, str_tmp );
	}

	output = push( output, strDuplicate( "D=A" ) );

	if( !strcmp( str_segment, "local" ) ){
		output = push( output, strDuplicate( "@LCL" ) );
	}
	else if( !strcmp( str_segment, "argument") ){
		output = push( output, strDuplicate( "@ARG" ) );
	}

	if( strcmp( str_segment, "static" ) ){
		output = push( output, strDuplicate( "D=A+D" ) ); 
	}
	
	
	output = push( output, strDuplicate( "@R13" ) ); // uso R13 come variabile temporanea
	output = push( output, strDuplicate( "M=D" ) );

	output = ASM_DecSP( output ); // SP--

	output = push( output, strDuplicate( "D=M" ) );// *address = *SP
	output = push( output, strDuplicate( "@R13" ) ); 
	output = push( output, strDuplicate( "A=M" ) );
	output = push( output, strDuplicate( "M=D" ) );

	return output;
}

#define INDEX_INSTRUCTION_NAME 0
#define INDEX_SEGMENT_NAME 1
#define INDEX_SEGMENT_VARIABLE 2

#define INDEX_FUNCTION_NAME 1
#define INDEX_FUNCTION_N_ARGUMENTS 2
#define INDEX_FUNCTION_N_LOCAL_VARIABLES INDEX_FUNCTION_N_ARGUMENTS

list_node *translate( list_node *input ){

	unsigned int vr_row = 1;
	list_node *tmp = input, *output = NULL, *instruction_words = NULL;
	int n_words = 0;
	bool b_error = false;
	char *str = NULL;
	char **instruction = NULL;
	while( !b_error && tmp != NULL ){
		str = tmp->value;
		instruction_words = strWords( tmp->value, " ");
		instruction = list_toArrayStr( instruction_words, &n_words, false );
		if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "push" ) ){ // istruzioni per accedere a memoria
			output = ASM_push( output, instruction[ INDEX_SEGMENT_NAME ], instruction[INDEX_SEGMENT_VARIABLE] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "pop" ) ){
			output = ASM_pop( output, instruction[ INDEX_SEGMENT_NAME ], instruction[INDEX_SEGMENT_VARIABLE] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "label" ) ){ // istruzioni di salto / etichette

		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "goto" ) ){ // istruzioni di salto / etichette

		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "if-goto" ) ){ // istruzioni di salto / etichette

		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "function" ) ){ // Istruzioni per funzioni

		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "call" ) ){

		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "return" ) ){

		}
		else{ // è un'operazione "primitiva"
			if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "add" ) ){

			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "sub" ) ){

			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "neg" ) ){

			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "not" ) ){

			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "and" ) ){

			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "or" ) ){

			}
			else{ // operatori matematici =<>
				if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "eq" ) ){

				}
				else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "gt" ) ){

				}
				else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "lt" ) ){

				}
				else{
					printf( "ERRORE: istruzione non riconosciuta \n" );
					b_error = true;
				}
			}
		}

		if( output == NULL ){
			b_error = true;
		}

		if( b_error ){
			printf( "Errore avvenuto durante l'elaborazione della riga %d : \"%s\"\n", vr_row, tmp->value );
			delete_list( output, true );
			output = NULL;
		}
		free( instruction );
		instruction = NULL;

		delete_list( instruction_words, true );
		instruction_words = NULL;

		tmp = tmp->next;
		vr_row += 1;
	}

	return list_node_reverse( output );
}

list_node *translator( list_node *input ){
	list_node *output = NULL, *tmp = NULL, *instructions = NULL;

	printf("Rimozioni commenti e normalizzazione del contenuto in corso...\n");
	instructions = set_content_to_simple_vm_format( input ); // normalizza il contenuto in stringhe

	#ifdef DEBUG
	printf("\nOUTPUT:\n");
	writeFile( "tmp.asm", setup_list_str_to_list_char( instructions ));
	#endif

	if( instructions == NULL ){
		return NULL;
	}
	
	printf("Traduzione delle istruzioni VM in ASM in corso...\n");
	// traduce
	tmp = translate( instructions );
	delete_list( instructions, true );
	instructions = NULL;
	
	#ifdef DEBUG
	printf("Conversione lista stringhe a lista caratteri in corso...\n");
	printf("\n");
	#endif

	output = setup_list_str_to_list_char( tmp );
	delete_list( tmp, true );
	tmp = NULL;

	return output;
}

int main( int nArgs, char **args ){

	if( nArgs > 1 && nArgs < 3){
		char *ptr_char = NULL; // usato temporaneamente
		char *filename = args[1];		
		list_node *input = NULL, *output = NULL;
		
		if( filename == NULL ){
			printf("ERRORE: File input non speficato\n");
		}
		else{
			if( !strEndWith( filename, FILE_INPUT_EXTENSION ) ){
				printf("ERRORE: Estensione file non supportata\n");
			}
			else{
				input = readFile( filename, input );
				if( input == NULL ){
					printf("ERRORE: Impossbile aprire il file '%s'\n", filename );
				}
				else{
					ptr_char = (char*)input->value;
					if( *ptr_char == '\0' ){
						printf("ERRORE: Il file '%s' risulta vuoto\n", filename );
					}
					else{
						printf("file '%s' letto con successo\n", filename);
						
						#ifndef DEBUG
						printf( "Si consiglia di ricompilare decommentando prima la definizione di 'DEBUG' in utility.h se si vuole ottenere un feedback grafico delle operazioni che l'assembler sta elaborando\n" );
						#endif

						#ifdef DEBUG
						printf("caratteri letti: %d\n", size( input, true ) );
						list_node_print( "%c", input );
						printf("\n");
						#endif
						
						output = translator( input ); // elabora il contenuto del file, restituendo il contenuto da scrivere su file
						delete_list( input, true );
						input = NULL;
						#ifdef DEBUG
						printf("caratteri elaborati: %d\n", size(  output, true ) );
						list_node_print( "%c", output );
						printf( "\n" );
						#endif

						if( output != NULL ){
							int length_estension = strlen( FILE_OUTPUT_EXTENSION );
							int length_filename = strlen( filename );
							int length_FilenameOut = length_filename + length_estension; // la dimensione non è proprio ottimizzata ma sicuramente non andrà outofbound
							char *filename_out = ( char* ) malloc(sizeof(char) * ( length_FilenameOut + 1 ) );
							strncpy( filename_out, filename, length_FilenameOut );
							replaceFilenameExtension( filename_out, length_filename, FILE_OUTPUT_EXTENSION );
							printf("Scrittura dell'elaborazione su file '%s' in corso...\n", filename_out);
							if( writeFile( filename_out, output) ){
								printf("Scrittura sul file '%s' avvenuta con successo\n", filename_out );
							}
							else{
								printf("ERRORE: Impossibile aprire o scrivere sul file '%s'\n", filename_out );
							}
						}
						else{
							printf("ERRORE: Impossbile completare l'operazione a causa di un errore durante l'elaborazione\n");
						}
					}
					delete_list( output, true);
				}
			}
		}
	}
	else{
		printf("ERRORE: File input mancante, numero parametri trovati: %d\n", nArgs);
	}

	return 0;
}