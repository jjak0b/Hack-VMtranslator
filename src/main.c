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


list_node *translator( list_node *input ){

	list_node *output = NULL;
	// traduce
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
						printf("caratteri letti: %d\n", size(  lh_input->head, true ) );
						list_node_print( "%c", lh_input->head );
						printf("\n");
						#endif
						
						output = translator( input ); // elabora il contenuto del file, restituendo il contenuto da scrivere su file
						delete_list( input, true );
						input = NULL;
						#ifdef DEBUG
						printf("caratteri elaborati: %d\n", size(  lh_output->head, true ) );
						list_node_print( "%c", lh_output->head );
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