/*
	Convenzioni usate:
		b_nomevar / isNomeVar 	-> usata per variabili booleane
		n_nomevar 				-> usata (non spesso) per indicare un intero
		str_nomevar 			-> usata per stringhe ( array di char con '\0' finale )
		lenght_nomevar			-> usata in genere nelle stringhe per indicare la lunghezza della stringa prima del carattere '\0'
		size_nomevar			-> usata per indicare la dimensione totale di una lista o array ( nelle stringhe quindi indica la dimensione allocata all'array )
		l_nomevar 				-> usata per indicare una struttura di tipo lista
	Consiglio per chi dovrà esaminare il codice: Consiglio vivamente di compilarlo decommentando la definizione di 'DEBUG' in utility.h
*/
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "list.h"
#include <string.h>
#include <dirent.h>

#define FILE_INPUT_EXTENSION ".vm" // estensione personalizzabile
#ifndef FILE_INPUT_EXTENSION
#define FILE_INPUT_EXTENSION ".vm"
#endif
#define FILE_OUTPUT_EXTENSION ".asm" // estensione personalizzabile
#ifndef FILE_OUTPUT_EXTENSION
#define FILE_OUTPUT_EXTENSION ".asm"
#endif

#define INDEX_INSTRUCTION_NAME 0
#define INDEX_SEGMENT_NAME 1
#define INDEX_SEGMENT_VARIABLE 2

#define INDEX_FUNCTION_NAME 1
#define INDEX_FUNCTION_N_ARGUMENTS 2
#define INDEX_FUNCTION_N_LOCAL_VARIABLES INDEX_FUNCTION_N_ARGUMENTS

#define ASM_ACTIVATION_RECORD_PROCEDURE_CALL_NAME "PROCEDURE_CALLER"
#define ASM_ACTIVATION_RECORD_PROCEDURE_RETURN_NAME "PROCEDURE_RESTORER"

list_node *instructions_processed = NULL;
/**
 * @brief : Accumula ogni carattere fino al primo "\n" e lo converte in stringa; sostituisce '\t' con ' ',  sequenze  di ' ' lasciandone solo 1; ignora tutti i caratteri dopo '\r' o '\n', commenti "//"
 * PreCondition: La lista deve essere una lista di caratteri
 * @param input 
 * @return list_node* : il puntatore della lista con gli elementi rimossi, altrimenti NULL se è avvenuto un errore di sintassi per i commenti (con relativo messaggio stampato) o se la lista è vuota
 */
list_node *set_content_to_simple_vm_format( list_node *input ){
	list_node *tmp = input;
	list_node *output = NULL;
	int unsigned row = 1;
	char *str_buffer = NULL;
	int tmp_index = -1;
	bool b_error = false;
	char *ptr_value = NULL;

	while( !b_error && tmp != NULL ){
		tmp_index = -1;
		str_buffer = strDuplicate( tmp->value );
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
			else if( isSubstr( str_buffer, "\n", &tmp_index ) && tmp_index >= 0){ // ignoro il contenuto dopo "\n" ( se non era presente prima "\r") dato che sarebbe il delimitatore di riga
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
					printf( "RIGA %d normalizzata: \"%s\"\n", row, str_buffer);
					#endif
					output = push( output, str_buffer );
				}
				#ifdef DEBUG
				else{
					printf( "RIGA %d normalizzata e ignorata: \"", tmp->value );
				}
				#endif
			}
			#ifdef DEBUG
			else{
				printf( "RIGA %d ignorata : %s", row, tmp->value );
			}
			#endif
		}
		if( str_buffer[0] == '\0' ){
			free( str_buffer);
			str_buffer = NULL;
		}
		row +=1;
		tmp = tmp->next;
	}

	
	if( b_error ){
		delete_list( output, true ); // libera la memoria usata
		return NULL;
	}
	
	else{
		output = list_node_reverse( output );
	}
	return output;
}

/**
 * @brief Da una lista di stringhe ritorna una lista con  aggiunto "\r\n" a fine riga di ogni stringa
 * PreConduition: 	la lista deve essera una lista di stringhe
 * PostCondition: 	Alla fine di ogni stringa, aggiunge la sequenza di caratteri "\r\n";
 * 					se una stringa è vuota invece non viene aggiunta
 * 					N.B: no viene istanziata una nuova lista, ma i valori che erano presenti nella lista passata sono deallocati e sostituiti con le nuove stringhe estese
 * @param input 
 * @return list_node* 
 */
list_node *prepare_list_str_for_file( list_node *input){
	list_node *tmp = input;
	int length_str = 0;
	char *str_tmp = NULL;
	while( tmp != NULL ){
		length_str = strlen( tmp->value );
		if( length_str > 0 ){
			str_tmp = malloc( sizeof(char) * ( length_str + 2 + 1) );
			// aggiungo "\r\n" a fine stringa per delimitare il fine riga
			strcpy( str_tmp, tmp->value );
			strcat( str_tmp, "\r\n");
			free( tmp->value );
			tmp->value = str_tmp;
			str_tmp = NULL;
		}
		tmp = tmp->next;
	}
	return input;
}

/**
 * @brief Inzializza lo Stack Pointer della VM con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_InitSP(list_node *output ){
	output = push( output, strDuplicate( "@256" ) );
	output = push( output, strDuplicate( "D=A" ) );
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "M=D" ) );
}

/**
 * @brief Incremenra lo Stack Pointer della VM con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_IncSP( list_node *output ){
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "AM=M+1" ) );
	return output;
}

/**
 * @brief Decrementa lo Stack Pointer della VM con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_DecSP( list_node *output ){
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "AM=M-1" ) );
	return output;
}

/**
 * @brief dichiara una label con istruzioni assembler hack data una stringa con il valore
 * PreCondition: output deve essere una lista di stringhe, str_label deve essere una stringa
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_declareLabel( list_node* output, char *str_label ){
	int length_label = strlen( str_label );
	char *str_tmp = malloc( sizeof(str_label) * ( length_label ) + 3 );
	strcpy( str_tmp, "(");
	strcat( str_tmp, str_label );
	strcat( str_tmp, ")");
	output = push( output, str_tmp );
	return output;
}

/**
 * @brief punta il registro A ASM ad una label con istruzioni assembler hack data una stringa con il valore
 * PreCondition: output deve essere una lista di stringhe, str_label deve essere una stringa
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_atLabel( list_node *output, char *str_label ){
	int length_label = strlen( str_label );
	char *str_tmp = malloc( sizeof(char) * (length_label + 2 ) );
	strcpy( str_tmp, "@");
	strcat( str_tmp, str_label );
	output = push( output, str_tmp );
	return output;
}

/**
 * @brief Aggiunge sulla cima dello stack della VM il valore specificato da 'index' nel segmento 'str_segment' con istruzioni assembler
 * PreCondition: output deve essere una lista di stringhe, filename, str_segment, index devono essere stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_push( list_node *output, char* filename, char *str_segment, char *index ){
	if( !isNumber( index, true ) ){
		return NULL;
	}
	int length_str_index = strlen( index );
	char *str_tmp = NULL;

	if( strcmp( str_segment, "pointer") ){
		// genero l'indice della variabile
		if( strcmp( str_segment, "static") ){ // @index
			// addr = START_SEGMENT + i
			output = ASM_atLabel(output, index);
		}
		else{ // @filename.index
			length_str_index = (strlen( filename ) + 1 + length_str_index );
			str_tmp = malloc( sizeof( char ) * (length_str_index + 1) );
			strcpy( str_tmp, filename );
			strcat( str_tmp, ".");
			strcat( str_tmp, index );
			output = ASM_atLabel( output, str_tmp );
			free( str_tmp );
		}
	}

	if( strcmp( str_segment, "constant") && strcmp( str_segment, "static" ) ){
		if( strcmp( str_segment, "pointer") ){
			output = push( output, strDuplicate( "D=A" ) );
		}

		if( !strcmp( str_segment, "local") ){
			output = push( output, strDuplicate( "@LCL" ) );
		}
		else if( !strcmp( str_segment, "argument") ){
			output = push( output, strDuplicate( "@ARG" ) );
		}
		else if( !strcmp( str_segment, "this") || ( !strcmp( str_segment, "pointer") && length_str_index == 1 && index[0] == '0' ) ){
			output = push( output, strDuplicate( "@THIS" ) );
		}
		else if( !strcmp( str_segment, "that") || ( !strcmp( str_segment, "pointer") && length_str_index == 1 && index[0] == '1' ) ){
			output = push( output, strDuplicate( "@THAT" ) );
		}
		else if( !strcmp( str_segment, "temp") ){
			output = push( output, strDuplicate( "@R5" ) );

			output = push( output, strDuplicate( "A=D+A" ) ); // addr = ADDRESS_START_SEGMENT + i
		}
		
		if( strcmp( str_segment, "temp") && strcmp( str_segment, "pointer") ){
			output = push( output, strDuplicate( "A=D+M" ) );  	// addr = START_SEGMENT + i
		}
	}

	if( !strcmp( str_segment, "constant" ) ){
		output = push( output, strDuplicate( "D=A" ) );
	}
	else{
		output = push( output, strDuplicate( "D=M" ) );		// *SP = *addr
	}

	output = ASM_IncSP( output );
	output = push( output, strDuplicate( "A=A-1" ) );
	output = push( output, strDuplicate( "M=D" ) );

	return output;
}

/**
 * @brief Rimuove dalla cima dello stack della VM il valore, e lo memoriza del segmento specificato da 'index' nel segmento 'str_segment' con istruzioni assembler
 * PreCondition: output deve essere una lista di stringhe, filename, str_segment, index devono essere stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_pop( list_node *output, char* filename, char *str_segment, char *index ){
	if( !isNumber( index, true ) ){
		return NULL;
	}
	int length_str_index = strlen( index );
	char *str_tmp = NULL;

	if( strcmp( str_segment, "pointer") ){
		// genero l'indice della variabile
		// pop constant x non è ammesso, pertanto si potranno avere risultati innaspettati
		if( strcmp( str_segment, "static") || !strcmp(str_segment, "constant") ){ // @index
			// addr = START_SEGMENT + i
			output = ASM_atLabel(output, index);
		}
		else{ // @filename.index
			length_str_index = (strlen( filename ) + 1 + length_str_index );
			str_tmp = malloc( sizeof( char ) * (length_str_index + 1) );
			strcpy( str_tmp, filename );
			strcat( str_tmp, ".");
			strcat( str_tmp, index );
			output = ASM_atLabel( output, str_tmp );
			free( str_tmp );
		}
		output = push( output, strDuplicate( "D=A" ) );
	}
	
	if( !strcmp( str_segment, "local" ) ){
		output = push( output, strDuplicate( "@LCL" ) );
	}
	else if( !strcmp( str_segment, "argument") ){
		output = push( output, strDuplicate( "@ARG" ) );
	}
	else if( !strcmp( str_segment, "this") || ( !strcmp( str_segment, "pointer") && length_str_index == 1 && index[0] == '0' ) ){
		output = push( output, strDuplicate( "@THIS" ) );
	}
	else if( !strcmp( str_segment, "that") || ( !strcmp( str_segment, "pointer") && length_str_index == 1 && index[0] == '1' ) ){
		output = push( output, strDuplicate( "@THAT" ) );
	}
	else if( !strcmp( str_segment, "temp") ){
		output = push( output, strDuplicate( "@R5" ) );

		output = push( output, strDuplicate( "D=D+A" ) ); 
	}

	if( !strcmp( str_segment, "pointer") ){
		output = push( output, strDuplicate( "D=A" ) );
	}
	else if( strcmp( str_segment, "static" ) && strcmp( str_segment, "temp" )){
		output = push( output, strDuplicate( "D=D+M" ) ); 
	}
	
	output = push( output, strDuplicate( "@R13" ) ); // uso R13 come variabile temporanea ( contiene l'indirizzo in cui salvare il valore dallo stack)
	output = push( output, strDuplicate( "M=D" ) );

	output = ASM_DecSP( output ); // SP--

	output = push( output, strDuplicate( "D=M" ) ); // *address = *SP
	output = push( output, strDuplicate( "@R13" ) ); 
	output = push( output, strDuplicate( "A=M" ) );
	output = push( output, strDuplicate( "M=D" ) );

	return output;
}

/**
 * @brief Dichiara una label con istruzioni assembler hack con la struttura "str_functionName.str_label"
 * PreCondition: output deve essere una lista di stringhe, str_functionName, str_label devono essere stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_label( list_node *output, char* str_functionName, char *str_label ){
	char *str_tmp = NULL;
	int length_functionName = strlen( str_functionName );
	int length_label = strlen( str_label );
	str_tmp = malloc( sizeof( char ) * ( length_functionName + length_label + 2 ) );
	strcpy( str_tmp, str_functionName);
	strcat( str_tmp, ".");
	strcat( str_tmp, str_label );
	output = ASM_declareLabel( output, str_tmp );
	free( str_tmp );
	return output;
}

/**
 * @brief Traduce l'istruzione if-goto o goto con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe, str_functionName, str_label devono essere stringhe;
 * 				 b_isConditional deve essere = true se si vuole l'istruzione di salto Condizionale, false se incondizionale
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_ifgoto( list_node *output, char* str_functionName, char *str_label, bool b_isConditional){
	int length_functionName = strlen( str_functionName );
	int length_label = strlen( str_label );
	char *str_tmp = malloc( sizeof(char) * ( length_functionName + length_label + 2 ) );
	strcpy( str_tmp, str_functionName );
	strcat( str_tmp, ".");
	strcat( str_tmp, str_label );

	if( b_isConditional ){
		output = ASM_DecSP( output );
		output = push( output, strDuplicate( "D=M" ) );
	}
	output = ASM_atLabel( output, str_tmp );
	free( str_tmp );
	if( b_isConditional ){
		output = push( output, strDuplicate("D;JNE"));
	}
	else{
		output = push( output, strDuplicate("0;JMP"));
	}

	return output;
}

/**
 * @brief Traduce l'istruzione goto con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe, str_functionName, str_label devono essere stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
#define ASM_goto( output, str_functionName, str_label) ( ASM_ifgoto( (output), (str_functionName), (str_label), (false) ) );

/**
 * @brief Traduce una chiamata di una subroutine primitiva in istruzioni assembler hack, mettendo nel registro D l'indirizzo di ritorno
 * PreCondition: output deve essere una lista di stringhe, per poter chiamare questa per l'istruzione str_instruction deve essere precedentemente dichiarata la subroutine con nome = strToUpperCase( str_instruction )
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_primitive_call( list_node *output, char *str_source_filename, char* str_instruction, unsigned int n_vr_row, list_node *implementation ){
	// chiamata subroutine
	char *str_instruction_Upper = strToUpperCase( str_instruction );
	int length_filename = strlen( str_source_filename );
	char *str_returnRow = int_to_string( n_vr_row + 1 );
	int length_returnRow = strlen( str_returnRow );
	int length_primitive = strlen( str_instruction );
	int length_returnLabel = length_filename + 8 + length_returnRow;
	char *str_returnLabel = malloc( sizeof( char ) * ( length_returnLabel + 1 ) );
	strcpy( str_returnLabel, str_source_filename );
	strcat( str_returnLabel, ".return." );
	strcat( str_returnLabel, str_returnRow );

	output = ASM_atLabel( output, str_returnLabel );
	output = push( output, strDuplicate( "D=A" ));
	if( implementation != NULL ){
		output = append( implementation, output );
	}
	else{
		output = ASM_atLabel( output, str_instruction_Upper );
		output = push( output, strDuplicate( "0;JMP" ));	
	}
	output = ASM_declareLabel( output, str_returnLabel );

	free( str_instruction_Upper );
	free( str_returnRow );
	free( str_returnLabel );

	return output;
}

/**
 * @brief Traduce l'istruzione function, dichiarandone le caratteristiche e l'inizializzazione delle variabili locali con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe, str_filename, str_functionName, str_n_local_vars devono essere stringhe;
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_function_declare( list_node *output, char *str_source_filename, unsigned int n_vr_row, char *str_functionName, char *str_n_local_vars ){
	output = ASM_declareLabel( output, str_functionName);
	// In R14 DEVE essere stato salvato il numero di variabili locali
	output = ASM_atLabel( output, str_n_local_vars );
	output = push( output, strDuplicate( "D=A" ) );
	output = push( output, strDuplicate( "@R14" ) );
	output = push( output, strDuplicate( "M=D" ) );
	output = ASM_primitive_call( output, str_source_filename, "FUNCTION_INIT", n_vr_row, NULL );
	return output;
}

/**
 * @brief Traduce l'istruzione call, includendo il salvataggio dell'activation record, e il passaggio del controllo alla funzione specificata con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe, str_filename, str_functionName, *str_n_args devono essere stringhe;
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_function_call( list_node *output, bool b_isAlreadyProcessed, char *str_filename, unsigned int n_vr_row, char *str_functionName, char *str_n_args ){
	char *str_tmp = NULL;
	int length_functionName = strlen( str_functionName );
	int n_args = atoi( str_n_args );
	int length_n_args = strlen( str_n_args );
	char *str_returnLabelRow = int_to_string( n_vr_row + 1 );
	int length_returnLabelRow = strlen( str_returnLabelRow );
	char *str_returnLabel = malloc( sizeof(char) * ( length_functionName + length_returnLabelRow + 9 ) );
	int length_unique_call_label = length_functionName + length_n_args + 6 ;
	char *str_unique_call_label = malloc( sizeof( char ) * ( length_unique_call_label + 1) );

	strcpy( str_unique_call_label, "CALL_" );
	strcat( str_unique_call_label, str_functionName );
	strcat( str_unique_call_label, "_");
	strcat( str_unique_call_label, str_n_args );

	strcpy( str_returnLabel, str_filename );
	strcat( str_returnLabel, ".return.");
	strcat( str_returnLabel, str_returnLabelRow );

	// In R13 DEVE essere stato salvato l'indirizzo di ritorno
	// In R14 DEVE essere stato salvato l'indirizzo della funzione chiamata
	// In R15 DEVE essere stato salvato il numero di argomenti passati
	output = ASM_atLabel( output, str_returnLabel ); // @filename.function.return.RETURNROW
	output = push( output, strDuplicate( "D=A" ) );

	if( b_isAlreadyProcessed ){
		output = ASM_atLabel(output, str_unique_call_label );
		output = push( output, strDuplicate( "0;JMP") );	
	}
	else{
		output = ASM_declareLabel( output, str_unique_call_label );
		// salvo Indirizzo ritorno
		output = push( output, strDuplicate( "@R13" ) );
		output = push( output, strDuplicate( "M=D" ) );
		// salvo indirizzo procedura chiamata
		output = ASM_atLabel(output, str_functionName );
		output = push( output, strDuplicate( "D=A" ) );
		output = push( output, strDuplicate( "@R14" ) );
		output = push( output, strDuplicate( "M=D" ) );
		// salvo numero argomenti passati
		output = ASM_atLabel( output, str_n_args );
		output = push( output, strDuplicate( "D=A" ) );
		output = push( output, strDuplicate( "@R15" ) );
		output = push( output, strDuplicate( "M=D" ) );
		// PASSO CONTROLLO
		output = ASM_atLabel(output, ASM_ACTIVATION_RECORD_PROCEDURE_CALL_NAME );
		output = push( output, strDuplicate( "0;JMP") );
	}
	// dichiaro indirizzo ritorno
	output = ASM_declareLabel( output, str_returnLabel );
	return output;
}

list_node *ASM_declare_ProcedureCaller( list_node *output ){
	// In R13 DEVE essere stato salvato l'indirizzo di ritorno
	// In R14 DEVE essere stato salvato l'indirizzo della funzione chiamata
	// In R15 DEVE essere stato salvato il numero di argomenti passati
	// salvo Indirizzo ritorno
	output = ASM_declareLabel(output, ASM_ACTIVATION_RECORD_PROCEDURE_CALL_NAME );
	output = push( output, strDuplicate( "@R13") );
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "A=M") );
	output = push( output, strDuplicate( "M=D") );
	output = ASM_IncSP( output );// //Incremento SP

	// Salvo LCL
	output = push( output, strDuplicate( "@LCL") );
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "A=M") );
	output = push( output, strDuplicate( "M=D") );

	//Incremento SP
	output = ASM_IncSP( output );// //Incremento SP

	//Salvo ARG
	output = push( output, strDuplicate( "@ARG") );
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "A=M") );
	output = push( output, strDuplicate( "M=D") );

	//Incremento SP
	output = ASM_IncSP( output );// //Incremento SP

	//Salvo THIS
	output = push( output, strDuplicate( "@THIS") );
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "A=M") );
	output = push( output, strDuplicate( "M=D") );

	//Incremento SP
	output = ASM_IncSP( output );// //Incremento SP

	//Salvo THAT
	output = push( output, strDuplicate( "@THAT") );
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "A=M") );
	output = push( output, strDuplicate( "M=D") );

	//Incremento SP
	output = ASM_IncSP( output );// //Incremento SP

	// setto ARG (D=SP-(n+5))
	output = push( output, strDuplicate( "@R15") ); // NUMERO ARGOMENTI ( ASSEGNATO DA CHIAMANTE)
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@5") );
	output = push( output, strDuplicate( "D=D+A") );
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "D=M-D") );
	output = push( output, strDuplicate( "@ARG") );
	output = push( output, strDuplicate( "M=D") );

	// setto LCL
	output = push( output, strDuplicate( "@SP") );
	output = push( output, strDuplicate( "D=M") );
	output = push( output, strDuplicate( "@LCL") );
	output = push( output, strDuplicate( "M=D") );

	// PASSO CONTROLLO
	output = push( output, strDuplicate( "@R14") );
	output = push( output, strDuplicate( "A=M") );
	output = push( output, strDuplicate( "0;JMP") );
	// Fine Chiamante di procedura --> è in esecuzione la funzione
}

/**
 * @brief Dichiara il set di istruzioni che occorrono per ripristinare l' activation record del chiamante, ponendo il valore di ritorno sullo stack e il passaggio del controllo al indirizzo di ritorno del chiamante con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_declare_ProcedureRestorer( list_node *output ){
	// R13 è memorizzato il frame
	// R14 è memorizzato l'indirizzo di ritorno
	output = ASM_declareLabel(output, ASM_ACTIVATION_RECORD_PROCEDURE_RETURN_NAME );
	// Inzializzo variabile temporanea FRAME
	output = push( output, strDuplicate( "@LCL"));
	output = push( output, strDuplicate( "D=M")); // output = push( output, strDuplicate( "D=A"));
	output = push( output, strDuplicate( "@R13"));
	output = push( output, strDuplicate( "M=D"));

	// Mi salvo l'indirizzo di ritorno
	output = push( output, strDuplicate( "@R13")); 	// FRAME-5
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@5"));
	output = push( output, strDuplicate( "D=D-A"));
	output = push( output, strDuplicate( "A=D"));	// RET = *(FRAME-5)
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@R14"));
	output = push( output, strDuplicate( "M=D"));

	// POP D
	output = push( output, strDuplicate( "@SP"));
	output = push( output, strDuplicate( "AM=M-1"));
	output = push( output, strDuplicate( "D=M"));

	// *ARG
	output = push( output, strDuplicate( "@ARG"));
	output = push( output, strDuplicate( "A=M"));
	output = push( output, strDuplicate( "M=D"));

	// ripristino SP caller
	output = push( output, strDuplicate( "@ARG"));
	output = push( output, strDuplicate( "D=M+1"));
	output = push( output, strDuplicate( "@SP"));
	output = push( output, strDuplicate( "M=D"));

	// ripristino THAT caller
	output = push( output, strDuplicate( "@R13")); 	// FRAME-1
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@1"));
	output = push( output, strDuplicate( "D=D-A"));
	output = push( output, strDuplicate( "A=D")); 	// THAT = *(FRAME-1)
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@THAT"));
	output = push( output, strDuplicate( "M=D"));

	// ripristino THIS caller
	output = push( output, strDuplicate( "@R13")); 	// FRAME-2
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@2"));
	output = push( output, strDuplicate( "D=D-A"));
	output = push( output, strDuplicate( "A=D")); 	// THIS = *(FRAME-2)
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@THIS"));
	output = push( output, strDuplicate( "M=D"));

	// ripristino ARG caller
	output = push( output, strDuplicate( "@R13")); 	// FRAME-3
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@3"));
	output = push( output, strDuplicate( "D=D-A"));
	output = push( output, strDuplicate( "A=D")); 	// ARG = *(FRAME-3)
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@ARG"));
	output = push( output, strDuplicate( "M=D"));

	// ripristino LCL caller
	output = push( output, strDuplicate( "@R13")); 	// FRAME-4
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@4"));
	output = push( output, strDuplicate( "D=D-A"));
	output = push( output, strDuplicate( "A=D")); 	// LCL = *(FRAME-4)
	output = push( output, strDuplicate( "D=M"));
	output = push( output, strDuplicate( "@LCL"));
	output = push( output, strDuplicate( "M=D"));

	// goto RET
	output = push( output, strDuplicate( "@R14"));
	output = push( output, strDuplicate( "A=M"));
	output = push( output, strDuplicate( "0;JMP"));

	return output;
}

/**
 * @brief Traduce l'istruzione return, ripristinando l' activation record del chiamante, ponendo il valore di ritorno sullo stack e il passaggio del controllo al indirizzo di ritorno del chiamante con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_function_return( list_node *output ){
	// ripristino l'activazion record
	output = ASM_atLabel( output, ASM_ACTIVATION_RECORD_PROCEDURE_RETURN_NAME);
	output = push( output, strDuplicate( "0;JMP"));
	
	return output;
}

/**
 * @brief Traduce l'istruzione neg con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_neg( list_node *output ){
	// decremento SP e lo punto ( consumo primo operando )
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "A=M-1" ) );

	output = push( output, strDuplicate( "M=-M") ); // neg
	// il valore in SP non è cambiato, quindi non c'è bisogno di incrementarlo
	return output;
}

/**
 * @brief Traduce l'istruzione not con istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe
 * PostCondition: in output sono allocate ed aggiunte (con push) le istruzioni necessarie
 * @param output 
 * @return list_node* 
 */
list_node *ASM_not( list_node *output ){
	// consumo l'operando e metto il risulato nella sua stessa locazione
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "A=M-1" ) );
	output = push( output, strDuplicate( "M=!M") ); // not
	// il valore in SP non è cambiato, quindi non c'è bisogno di incrementarlo
	return output;
}

/**
 * @brief dichiara la subroutine primitiva delle istruzioni lt, gt, eq in istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe;
 * 				 str_instruction deve essere una stringa e deve avere solamente uno tra i seguenti valori: ("lt", "gt", "eq") ( se nella VM sarebbero aggiunti anche "le", "ge", "ne" questa funzione li supporterà)
 * PostCondition: Aggiunge alla lista le istruzioni ASM come stringhe necessarie per l'istruzione
 * @param output 
 * @param str_instruction := tipo di istruzione di contronto valori in vm
 * @param n_vr_row := riga del file .vm in cui è presente tale istruzione
 * @return list_node* 
 */
list_node *ASM_declare_primitive_compare( list_node *output, char* str_instruction ){
	// x < y --> x-y < 0
	// inizializzazione delle stringhe usate come label
	char *str_instruction_Upper = strToUpperCase( str_instruction );
	int lenght_instruction_Upper = strlen( str_instruction );
	char *str_ASM_jump = malloc( sizeof(char) * ( 4 + lenght_instruction_Upper ) );
	strcpy( str_ASM_jump, "D;J");
	strcat( str_ASM_jump, str_instruction_Upper );

	int length_LABEL_CASE = lenght_instruction_Upper + 5;
	char *str_LABEL_CASE = malloc( sizeof(char) * (length_LABEL_CASE + 1 ) );
	strcpy( str_LABEL_CASE, str_instruction_Upper );
	strcat( str_LABEL_CASE, "_TRUE" );

	int length_LABEL_CASE_END = lenght_instruction_Upper + 4;
	char *str_LABEL_CASE_END = malloc( sizeof(char) * ( length_LABEL_CASE_END + 1) );
	strcpy( str_LABEL_CASE_END, str_instruction_Upper );
	strcat( str_LABEL_CASE_END, "_END");

	output = ASM_declareLabel( output, str_instruction_Upper );
	output = push( output, strDuplicate( "@R13" ) );
	output = push( output, strDuplicate( "M=D" ) );

	// sub
	// decremento SP e lo punto ( consumo primo operando )
	output = ASM_DecSP( output );
	output = push( output, strDuplicate( "D=-M" ) );
	// decremento SP e lo punto ( consumo secondo operando )
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "A=M-1" ) );
	// salvo risultato in registro D
	output = push( output, strDuplicate( "D=D+M" ) );

	// condizioni necessarie per verificare la validità dell'istruzione
	output = ASM_atLabel( output, str_LABEL_CASE );
	output = push( output, str_ASM_jump ); // output = push( output, strDuplicate( "D;JLT" ) );
	output = push( output, strDuplicate( "D=0" ) ); // ELSE
	output = ASM_atLabel( output, str_LABEL_CASE_END );
	output = push( output, strDuplicate( "0;JMP" ) );
	output = ASM_declareLabel( output, str_LABEL_CASE ); // IF
	output = push( output, strDuplicate( "D=-1" ) );
	output = ASM_declareLabel( output, str_LABEL_CASE_END );
	// push true (-1) o false (0) sullo stack
	output = push( output, strDuplicate( "@SP" ) ); // salvo true o false ( da D )
	output = push( output, strDuplicate( "A=M-1" ) );
	output = push( output, strDuplicate( "M=D" ) );
	output = push( output, strDuplicate( "@R13" ) ); // salto al'indirizzo di ritorno
	output = push( output, strDuplicate( "A=M" ) );
	output = push( output, strDuplicate( "0;JMP" ) );

	free( str_LABEL_CASE );
	free( str_LABEL_CASE_END );
	return output;
}
/**
 * @brief dichiara la subroutine primitiva delle istruzioni add, sub, and, or in istruzioni assembler hack
 * PreCondition: output deve essere una lista di stringhe;
 * 				 str_instruction deve essere una stringa e deve avere solamente uno tra i seguenti valori: (add, sub, and, or)
 * PostCondition: Aggiunge alla lista le istruzioni ASM come stringhe necessarie per l'istruzione
 * @param output 
 * @param str_instruction := tipo di istruzione di contronto valori in vm
 * @param n_vr_row := riga del file .vm in cui è presente tale istruzione
 * @return list_node* 
 */
list_node *ASM_declare_primitive_compute( list_node *output, char* str_instruction ){
	char *str_instruction_Upper = strToUpperCase( str_instruction );
	output = ASM_declareLabel( output, str_instruction_Upper );
	output = push( output, strDuplicate( "@R13" ) );
	output = push( output, strDuplicate( "M=D" ) );
	output = ASM_DecSP( output );
	if( strcmp(str_instruction_Upper, "SUB" ) ){
		output = push( output, strDuplicate( "D=M" ) );
	}
	else{
		output = push( output, strDuplicate( "D=-M" ) );
	}
	output = push( output, strDuplicate( "@SP" ) );
	output = push( output, strDuplicate( "A=M-1" ) );

	if( !strcmp(str_instruction_Upper, "SUB" ) || !strcmp(str_instruction_Upper, "ADD" ) ){
		output = push( output, strDuplicate( "M=D+M" ) );
	}
	else if( !strcmp(str_instruction_Upper, "AND" ) ){
		output = push( output, strDuplicate( "M=D&M" ) );
	}
	else if( !strcmp(str_instruction_Upper, "OR" ) ){
		output = push( output, strDuplicate( "M=D|M" ) );
	}
	output = push( output, strDuplicate( "@R13" ) );
	output = push( output, strDuplicate( "A=M" ) );
	output = push( output, strDuplicate( "0;JMP" ) );
	free( str_instruction_Upper );

	return output;
}
/**
 * @brief Dichiara la primitiva "FUNCTION_INIT" per inizializzare il segmento local in istruzioni assembler hack;
 * 		  premesso che in D deve essere memorizzato l'indirizzo di ritorno di questa primitiva;
 * 		  premesso che in R14 deve essere memorizzato il numero di locali della funzione
 * 
 * 
 * @param output 
 * @return list_node* 
 */
list_node *ASM_declare_primitive_FunctionInit( list_node *output ){
	output = ASM_declareLabel( output, "FUNCTION_INIT" );	
	output = push( output, strDuplicate( "@R13" ) ); // memorizzo l'indirizzo di ritorno
	output = push( output, strDuplicate( "M=D" ) );
	output = push( output, strDuplicate( "@R14" ) ); // recupero il numero variabili locali
	output = push( output, strDuplicate( "D=M" ) );
	// INIZIALIZZO LOCALI
	output = ASM_declareLabel( output, "INIT_LOCALS" );
	output = ASM_atLabel( output, "INIT_LOCALS_END" );
	output = push( output, strDuplicate( "D;JLE" ) );
	// push 0
	output = ASM_IncSP( output );
	output = push( output, strDuplicate( "A=A-1" ) );
	output = push( output, strDuplicate( "M=0" ) );
	output = push( output, strDuplicate( "D=D-1" ) );

	output = ASM_atLabel( output, "INIT_LOCALS" );
	output = push( output, strDuplicate( "0;JMP" ) );
	output = ASM_declareLabel( output, "INIT_LOCALS_END" );
	output = push( output, strDuplicate( "@R13" ) );
	output = push( output, strDuplicate( "A=M" ) );
	output = push( output, strDuplicate( "0;JMP" ) );
}

/**
 * @brief Data una liste di stringhe contenente stringhe di istruzioni in formato "semplice" ( ottenuta come output di set_content_to_simple_vm_format(...) ) e il nome del file letto,
 * 		  traduce le istruzioni VM Hack del file in Assembler hack;
 * 		  Restituisce NULL se è avvenuto un errore durante l'elaborazione di qualche istruzione
 * PreCondition: input deve essere una lista di stringhe ottenuta in output dala funzione set_content_to_simple_vm_format(...) ;
 * 				 impostare b_init = true se si vuole inizializzare la VR hack nel file indicato con la chiamata a Sys.init
 * PostCondition: in output sono allocate ed aggiunte le istruzioni necessarie pronte per l'esecuzione (ordinate)
 * @param input 
 * @param filename 
 * @param b_init 
 * @return list_node* 
 */
list_node *translate( list_node *input, char *str_source_filename, char* str_dest_filename, bool b_init, bool b_callInit ){
	unsigned int vr_row = 1; // indica la riga corrispondente all'istruzione che si sta elaborando
	list_node *tmp = NULL, // nodo della lista a cui viene assegnatao la riga dell'istruzione da elaborare
	*output = NULL, // lista contenente il contenuto elaborato
	*instruction_words = NULL, // lista che contiene le parole di ogni riga, utilizzando come separatore " "
	*implementation = NULL; // lista contenente l'implementazione di una funzionalità primitiva da associare a ASM_primitive_call(...)

	int n_words = 0; // numero di parole in instruction_words ( non utilizzato )
	bool b_error = false; // indica se è avvenuto un errore, appena ha valore true, la funziona ritorna NULL
	char *str = NULL; // puntatore relativo a tmp->value
	bool b_processed = false; // indica se un'istruzione ( per ora associato a chiamate a funzioni con medesimi argomenti, funzionalità primitive ) è stata precedentemente elaborata
	char **instruction = NULL; // matrice contenente le parole della lista instruction_words, per un accesso rapido
	char *str_last_function_name = malloc( sizeof( char ) ); // contiene in nome dell'ultima funzione elaborata ( utilizzata per le label )
	*str_last_function_name = '\0';

	if( b_init ){
		output = ASM_atLabel( output, "BOOTSTRAP" );
		output = push( output, strDuplicate("0;JMP" ) );
		#ifdef DEBUG
		output = push(output, strDuplicate("// Definizione Subroutine") );
		#endif

		output = ASM_declare_ProcedureCaller( output );
		output = ASM_declare_ProcedureRestorer( output );
		output = ASM_declare_primitive_FunctionInit( output );

		#ifdef DEBUG
		output = push(output, strDuplicate("// bootstrap") );
		#endif

		output = ASM_declareLabel( output, "BOOTSTRAP" );
		output = ASM_InitSP( output );

		if( b_callInit ){
			tmp = push( tmp, strDuplicate("call Sys.init 0") );
		}
	}
	tmp = append( tmp, input );
	while( !b_error && tmp != NULL ){
		str = tmp->value;
		#ifdef DEBUG
		// aggiungo la relativa istruzione VMHack come commento
		char *str_debug = malloc( sizeof(char) * ( strlen( str ) + 4 ) );
		strcpy( str_debug, "// ");
		strcat( str_debug, str );
		output = push( output, str_debug );
		#endif
		instruction_words = strWords( str, " ");

		// con isStrInList(...) controllo se questa istruzione è stata già elaborata in precedenza
		// la imposto come già elabora, e alcune funzioni (quello che lo supportano) la elaboreranno con una chiamata diretta

		b_processed = false;

		instruction = list_toArrayStr( instruction_words, &n_words, false );
		if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "push" ) ){ // istruzioni per accedere a memoria
			output = ASM_push( output, str_source_filename, instruction[ INDEX_SEGMENT_NAME ], instruction[INDEX_SEGMENT_VARIABLE] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "pop" ) ){
			output = ASM_pop( output, str_source_filename, instruction[ INDEX_SEGMENT_NAME ], instruction[INDEX_SEGMENT_VARIABLE] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "label" ) ){ // istruzioni di salto / etichette
			output = ASM_label( output, str_last_function_name, instruction[ INDEX_SEGMENT_NAME ] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "goto" ) ){ // istruzioni di salto / etichette
			output = ASM_goto( output, str_last_function_name, instruction[ INDEX_SEGMENT_NAME ] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "if-goto" ) ){ // istruzioni di salto / etichette
			output = ASM_ifgoto( output, str_last_function_name, instruction[ INDEX_SEGMENT_NAME ], true);
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "function" ) ){ // Istruzioni per funzioni
			free( str_last_function_name );
			str_last_function_name = strDuplicate( instruction[INDEX_FUNCTION_NAME] );
			output = ASM_function_declare( output, str_source_filename, vr_row, instruction[INDEX_FUNCTION_NAME], instruction[ INDEX_FUNCTION_N_LOCAL_VARIABLES] );
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "call" ) ){
			b_processed = isStrInList( str, instructions_processed );
			if( !b_processed ){
				instructions_processed = push( instructions_processed, strDuplicate(str) );
			}
			output = ASM_function_call( output, b_processed, str_source_filename, vr_row, instruction[INDEX_FUNCTION_NAME], instruction[ INDEX_FUNCTION_N_ARGUMENTS ]);
		}
		else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "return" ) ){
			output = ASM_function_return( output );
		}
		else{ // è un'operazione "primitiva"
			if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "add" ) || // chiamata a subroutine primitiva
				!strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "sub" ) || 
				!strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "and" ) ||
				!strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "or" )  ){

				b_processed = isStrInList( instruction[ INDEX_INSTRUCTION_NAME ], instructions_processed );
				implementation = NULL;
				if( !b_processed ){	
					instructions_processed = push( instructions_processed, strDuplicate( instruction[ INDEX_INSTRUCTION_NAME ] ) );
					implementation = ASM_declare_primitive_compute( implementation, instruction[ INDEX_INSTRUCTION_NAME ] );
				}
				output = ASM_primitive_call( output, str_source_filename, instruction[ INDEX_INSTRUCTION_NAME ], vr_row , implementation );
				implementation = NULL;
			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "eq" ) ||
					 !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "gt" ) || 
					 !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "lt" ) ){

					b_processed = isStrInList( instruction[ INDEX_INSTRUCTION_NAME ],  instructions_processed );
					implementation = NULL;
					if( !b_processed ){
						instructions_processed = push( instructions_processed, strDuplicate( instruction[ INDEX_INSTRUCTION_NAME ] ) );
						implementation = ASM_declare_primitive_compare( implementation, instruction[ INDEX_INSTRUCTION_NAME ] );
					}
					output = ASM_primitive_call( output, str_source_filename, instruction[ INDEX_INSTRUCTION_NAME ], vr_row , implementation );
					implementation = NULL;
			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "neg" ) ){ // la chiamata alla primitiva di quest operatori unari non sarebbe ottimizzata, quindi genera direttamente il codice
				output = ASM_neg( output );
			}
			else if( !strcmp( instruction[ INDEX_INSTRUCTION_NAME ], "not" ) ){
				output = ASM_not( output );
			}
			else{
				printf( "ERRORE: istruzione \"%s\" non riconosciuta o supportata \n", instruction[ INDEX_INSTRUCTION_NAME ] );
				b_error = true;
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
	free( str_last_function_name );

	return list_node_reverse( output );
}

/**
 * @brief Data una lista di stringhe contenente istruzioni in formato "non semplice" ( non ottenuto come output di set_content_to_simple_vm_format(...) ) e il percorso del file di output,
 * 		 traduce le istruzioni VM Hack del file in Assembler hack;
 * 		 Restituisce NULL se è avvenuto un errore durante l'elaborazione di qualche istruzione
 * PreCondition: input deve essere una lista di stringhe contenente istruzioni VM hack da elaborare;
 * 				 filePathname deve essere una stringa
 * 				 impostare b_init = true se si vuole inizializzare la VR hack nel file indicato
 * PostCondition: nella lista di STRINGHE restituita sono allocate le istruzioni necessarie pronte per l'esecuzione e scrittura su file (ordinate)
 * 				 N.B: la lista input != lista restituita
 * @param input 
 * @param filePathname 
 * @param b_init
 * @return list_node* 
 */
list_node *translator( list_node *input, char *str_source_filename, char *str_dest_fileName, bool b_init, bool b_callInit ){
	list_node *output = NULL, *tmp = NULL, *instructions = NULL;
	char *source_filename = getFileNameFromPath( str_source_filename, false );
	char *dest_filename = getFileNameFromPath( str_dest_fileName, false );
	
	printf("Rimozioni commenti e normalizzazione del contenuto in corso...\n");
	instructions = set_content_to_simple_vm_format( input ); // normalizza il contenuto in stringhe

	#ifdef DEBUG
	printf( "FILENAME: '%s'\n", str_source_filename );
	#endif

	if( instructions == NULL ){
		return NULL;
	}
	
	printf("Traduzione delle istruzioni VM in ASM in corso...\n");
	// traduce
	tmp = translate( instructions, source_filename, dest_filename, b_init, b_callInit);
	if( tmp == NULL ){
		return NULL;
	}
	delete_list( instructions, true );
	instructions = NULL;

	printf("Preparazione elaborato per la scrittura su file...\n");
	output = prepare_list_str_for_file( tmp );
	printf("Traduzione completata\n");

	free( source_filename );
	free( dest_filename );
	return output;
}

int main( int nArgs, char **args ){

	if( nArgs > 1 && nArgs < 3){
		char *str_path = strDuplicate( args[1] );
		int lenght_path = strlen( str_path );

		char *str_filepath = NULL;
		int length_filepath = -1;

		char *str_filename = NULL;
		int lenght_filename = -1;

		char *str_filepath_out = NULL;
		int length_filepath_out = -1;

		char *str_tmp = NULL;
		int length_tmp = -1;

		char *str_path_separator = malloc( sizeof(char) * 2 ); // lo copio in questo modo poichè FILE_PATH_SEPARATOR può essere cambiato
		str_path_separator[0] = FILE_PATH_SEPARATOR;
		str_path_separator[1] = '\0';

		list_node *input = NULL, *output = NULL, *tmp = NULL, *tmp2 = NULL;
		bool b_isFile = false;
		bool b_error = false;

		// Indico quanti caratteri dedicare al separatore di percorso nel caso non ci sia
		int length_path_separator = 1;
		int length_estension = strlen( FILE_OUTPUT_EXTENSION );

		if( str_path != NULL ){
			printf("Percorso specificato: '%s'\n", str_path );
			b_isFile = isFile( str_path );
			list_node *list_filenames = NULL;
			if( b_isFile ){
				if( strEndWith( str_path, FILE_INPUT_EXTENSION ) ){
					// separo nomefile e percorso radice dove risiede il file
					str_filename = getFileNameFromPath( str_path, true );
					list_filenames = push( list_filenames, str_filename );
					int index_start_filename = -1;
					if( isSubstr( str_path, str_filename, &index_start_filename ) && index >= 0 ){// sarà sempre true
						str_path[ index_start_filename ] = '\0';
						lenght_path = strlen( str_path );
					}
					
					if( lenght_path == 0){
						free( str_path );
						lenght_path = 2;
						str_path = malloc( sizeof(char) * ( lenght_path + 1 ) );
						str_path[ 0 ] = '.';
						str_path[ 1 ] = FILE_PATH_SEPARATOR;
						str_path[ 2 ] = '\0';
					}

					// costruisco il percorso finale per l'output
					str_filename = getFileNameFromPath( str_filename, false ); // non faccio il free perchè la stringa è salvata in list_filenames
					lenght_filename = strlen( str_filename );
					length_filepath_out = lenght_path + lenght_filename + length_estension;
					str_filepath_out = malloc( sizeof( char ) * length_filepath_out + 1 );
					strcpy( str_filepath_out, str_path );
					strcat( str_filepath_out, str_filename );
					strcat( str_filepath_out, FILE_OUTPUT_EXTENSION );
				}
				else{
					printf("ERRORE: Estensione file non supportata\n");
					b_error = true;
				}
			}
			else{
				DIR *dir;
				struct dirent *ent;
				dir = opendir ( str_path );
				if ( dir != NULL ) {
					// Cerco i file validi e li aggiungo in lista
					ent = readdir (dir);
					while ( ent != NULL ) {
						if( strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") && strEndWith(ent->d_name, FILE_INPUT_EXTENSION) ){
							list_filenames = push( list_filenames, strDuplicate( ent->d_name ) );
							printf( "File individuato: '%s'\n", ent->d_name );
						}
						ent = readdir (dir);
					}
					closedir(dir);
				}
				else {
					printf("ERRORE: Impossibile accedere alla directory '%s'\n", str_path );
					b_error = true;
				}

				if( strEndWith( str_path, str_path_separator ) ){ // il path completo include uno "/" finale che non è da considerare
					lenght_path -= 1 ;
					str_path[ lenght_path ] = '\0';
				}

				// costruisco il percorso finale per l'output
				length_filepath_out = lenght_path + length_estension;
				str_filepath_out = malloc( sizeof( char ) * length_filepath_out + 1 );
				strcpy( str_filepath_out, str_path );
				strcat( str_filepath_out, FILE_OUTPUT_EXTENSION );

				// (ri)aggiungo il separatore ( se è stato rimosso) per ottenere la radice del percorso ( vedere ciclo seguente )
				str_tmp = malloc( sizeof(char) * ( lenght_path + 2 ) );
				strcpy( str_tmp, str_path );
				strcat( str_tmp, str_path_separator );
				free( str_path );
				str_path = str_tmp;
				str_tmp = NULL;
			}

			#ifndef DEBUG
			printf( "Si consiglia di ricompilare decommentando prima la definizione di 'DEBUG' in utility.h se si vuole ottenere un feedback grafico delle operazioni che il traduttore sta elaborando\n" );
			#endif

			// Inizio a leggere i(l) file(s)
			// Elabora i file trovati
			list_node *node_filename = list_filenames;
			while( node_filename != NULL && !b_error ){
				// Ricavo le informazioni riguardo al nome del file
				str_filename = node_filename->value;
				lenght_filename = strlen( str_filename );

				// Ricavo le informazioni riguardo al percorso dove risided il file
				length_filepath = lenght_path + lenght_filename ;
				// costruisco il percorso del file con str_path  + str_filename
				str_filepath = malloc( sizeof( char ) * ( length_filepath + 1 ) );
				strcpy( str_filepath, str_path );
				strcat( str_filepath, str_filename );

				printf("Lettura file '%s' in corso...\n", str_filepath);
				input = readFile( str_filepath, input );
				if( input != NULL ){
					printf("File '%s' letto con successo\n", str_filepath);
					printf("Righe lette: %d\n", size( input, true ) );
					#ifdef DEBUG
					list_node_print( "%s", input );
					printf("\n");
					#endif
					
					tmp = translator( input, str_filename, str_filepath_out, output == NULL, !b_isFile ); // elabora il contenuto del file, restituendo il contenuto da scrivere su file
					b_error = tmp == NULL;
					delete_list( input, true );
					input = NULL;
					if( b_error ){
						printf("ERRORE: Impossibile completare la traduzione a causa di un errore nel file '%s' \n", str_filename );
					}
					else{
						output = append( output, tmp ); // Unisco il contenuto di tutti i file
						tmp = NULL;
					}
				}
				else{
					printf("ERRORE: Impossbile aprire il file '%s'\n", str_filepath );
				}
				node_filename = node_filename->next;
			}

			delete_list( node_filename, true );
			node_filename = NULL;

			if( !b_error ){
				printf("Righe totali elaborate: %d\n", size( output, true ) );
				#ifdef DEBUG
				list_node_print( "%s", output );
				printf( "\n" );
				#endif

				printf("Scrittura dell'elaborazione su file '%s' in corso...\n", str_filepath_out);
				if( writeFile( str_filepath_out, output, "%s") ){
					printf("Scrittura sul file '%s' avvenuta con successo\n", str_filepath_out );
				}
				else{
					printf("ERRORE: Impossibile aprire o scrivere sul file '%s'\n", str_filepath_out );
				}
			}
			else{
				printf("ERRORE: Impossbile completare l'operazione a causa di un errore durante l'elaborazione\n");
			}
			free( str_filepath_out );
			delete_list( output, true );
		}
		else{
			printf("ERRORE: File input non speficato\n");
		}
	}
	else{
		printf("ERRORE: File input mancante, numero parametri trovati: %d\n", nArgs);
	}

	return 0;
}