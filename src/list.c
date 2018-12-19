#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

/**
 * @brief
 * PreCondition: N/A
 * PostCondition: Dato un nodo, ritorna la sua lunghezza della lista a seguire (o precedere) da tale nodo
 * se b_reachLast != 0 conta fino all'ultimo elemento della lista ritornado lunghezza positiva
 * altrimenti conta fino al primo elemento ritornando lunghezza positiva
 * @param head 
 * @return La lunghezza della lista
 */
int size( list_node *head, bool b_reachLast){
	
	int c = 0;
	while( head != NULL ){
		c+=1;
		if( b_reachLast ){
			head = head->next;
		}
		else{
			head = head->prev;
		}
	}
	return c;
}

/**
 * @brief Data una lista ritorna il primo elemento della lista 
 * se la lista è vuota ritorna NULL
 * 
 * @param node 
 * @return list_node*  Il puntatore al primo nodo della lista
 */
list_node *first( list_node *node ){
	
	while( node != NULL && node->prev != NULL ){
		node = node->prev;
	}

	return node;
}

/**
 * @brief Data una lista ritorna l'ultimo elemento della lista 
 * se la lista è vuota ritorna NULL
 * 
 * @param head 
 * @return list_node*  Il puntatore all'ultimo nodo della lista
 */
list_node *last( list_node *head ){

	while( head != NULL && head->next != NULL ){
		head = head->next;
	}

	return head;
}

/**
 * @brief 			Inserisce nella testa dello stack il puntatore passato
 * PreCondition: 	il chiamante deve gestirsi autonomamente l'allocamento del valore, passandone solo l'indirizzo
 * PostCondition:	Inserisce nella testa dello stack il puntatore passato
 * @param stack 	lista considerata stack
 * @param value 	Puntatore del valore da aggiungere alla lista
 * @return list_node* puntatore alla nuova testa dello stack
 */
list_node *push( list_node *stack, void *value ){
	list_node *new_head = list_node_new( value );
	new_head->next = stack;
	stack->prev = new_head;
	return new_head;
}

/**
 * @brief Ritorna Il puntatore del valore presente in testa
 * PostCondition: se stack = NULL ritorna NULL
 * @param stack 
 * @return void* : Il puntatore del valore presente in testa
 */
void *top( list_node *stack){
	if( stack != NULL ){
		return stack->value;
	}
	else{
		return NULL;
	}
}

/**
 * @brief Toglie l'elemento presente in testa e lo ritorna
 * PreCondition:	l'argomento poassato deve essere l'indirizzo del puntatore allo stack
 * PostCondition:	La testa dello stack passato viene modificata: Lo stack punta al elemento successivo alla testa
 * @param stack 
 * @return list_node* : il nodo rimosso dalla testa
 */
list_node *pop( list_node **stack ){
	list_node *head = NULL;
	if( stack == NULL ){
		return NULL;
	}
	head = *stack;
	if( head != NULL ){
		*stack = head->next;
	}
	return head;
}

/**
 * @brief 
 * PreCondition: 	N/A
 * PostCondition:	Data una lista e un indice ritorna il nodo alla "index-esima"  posizione (anche negativa ) a partire dal nodo head
 * 					ritorna NULL se head == NULL oppure index < 0 oppure index è superiore (outofbound) alla lunghezza fino agli estremi
 *
 * @param head 
 * @param index 
 * @return list_node* Il nodo alla "index-esima" posizione
 */
list_node *get( list_node *head, int index ){

	if( head == NULL ){ // Considera anche caso index sia oltre la lunghezza di fino agli estremi ( prev o next ) poichè head diventerà NULL se l'ultimo head->next o head->prev == NULL
		return NULL;
	}
	else if( index == 0){
		return head;
	}
	else{
		if( index > 0){
			return get( head->next, index-1);
		}
		else{
			return get( head->prev, index+1);
		}
	}
}

/**
 * @brief 
 * PreCondition: index >= 0
 * PostCondition: Dati due nodi di lista e un indice, ritorna la prima lista con inserita la seconda a partire dalla posizione 'index-esima'
 * e collega l'ultimo elemento della seconda lista con la parte restante della prima, ovvero dalla posizione 'index'+1 della prima lista.
 * es: insert( c-i-a-o, c-i-a-o, 2 ) ->c-i-c-i-a-o-a-o
 * 
 * @param head 
 * @param node 
 * @param index 
 * @return list_node* nodo alla posizione index dopo inserimento
 */
list_node *insert( list_node *head, list_node *node, int index ){
	if( node == NULL ){
		return head;
	}
	else if( head == NULL  ){
		return NULL;
	}
	else if( index == 0){
		if( head != NULL ){
			list_node *node_last = last( node );
			head->prev = node_last;
			node_last->next = head;
			return node;
		}
		else{
			head = node;
		}
		return node;
	}
	else{
		if( index > 0 ){
			head->next = insert( head->next, node, index-1);
			if( head->next != NULL ){
				head->next->prev = head;
			}
		}
		return head;
	}
}

/**
 * @brief 
	PostCondition:
		Crea una nuovo nodo di una lista char assegnando il valore specificato ad esso.
 * 
 * @param value 
 * @param b_create_handler 
 * @param handler 
 * @return list_node* il puntatore del nuovo nodo
 */
list_node *list_node_new( void *value ){
	list_node *head;
	head = ( list_node* ) malloc( sizeof( list_node ) );

	head->next = NULL;
	head->prev = NULL;
	head->value = value;

	return head;
}

/**
 * @brief Dealloca il valore ed il nodo stesso, collegando prima tra loro il nodo successivo e precedente
 * 
 * @param node 
 */
void delete_node( list_node *node){
	if( node != NULL ){
		if( node->prev != NULL ){
			node->prev->next = node->next;
		}
		if( node->next != NULL){
			node->next->prev = node->prev;
		}
		free( node->value );
		node->value = NULL;
		free( node );
		node = NULL;
	}
}

/**
 * @brief data una lista restituisce la lista invertita
 * 
 * @param head 
 * @return list_node* : puntatore alla nuova testa
 */
list_node *list_node_reverse( list_node *head ){
	list_node  *head_new, *head_old;
	head_new = NULL ;
	while (head != NULL){
		head_old = head;
		head = head->next;
		head_old->next = head_new;
		head_new = head_old;
	}
	return head_new;
}

/**
 * @brief Stampa i valori degli elementi di una
 * Precondition: valore puntato da head->value deve essere un tipo supportato da printf (es char*, char, int, float, ecc..)
 * 
 * @param format formato di stampa del valore: la stringa viene passata a printf, quindi usare come se fosse tale funzione
 * @param head testa della lista da cui verranno stampati gli elementi
 */
void list_node_print( const char *format, list_node *head){
	char *ptr_char =NULL;
	while( head != NULL ){
		ptr_char = head->value;
		printf( format, *ptr_char);
		head = head->next;
	}
}

/**
 * @brief istanzia un nuovo nodo e vi assegna il puntatore dato, restituisce la testa della lista aggiornata
 * PostCondition: se handler = NULL viene istanziata e restituita un nuovo handler ( a cui assegna comunque il puntatore dato alla lista )
 * @param head
 * @param value 
 * @return list_node* 
 */
list_node *enqueue( list_node *head, void *value ){
	list_node *new_tail = list_node_new( value );
	list_node *old_tail = last( head );
	
	new_tail->prev = old_tail;
	new_tail->next = NULL; // dovrebbe essere già a NULL dato che era in coda...
	if( old_tail != NULL ){
		old_tail->next = new_tail;
	}
	else{
		head = new_tail;
	}

	return head;
}


/**
 * @brief rimuove dalla coda il primo nodo
 * PostCondition: 	se head = NULL restiuisce NULL
 * 					se b_free_item  = true allora viene deallocato lo spazio allocato da head->value, altrimenti dealloca solamete il nodo
 * @param handler 
 * @param b_free_item
 * @return list_node* della nuova testa della lista
 */
list_node *dequeue( list_node *head, bool b_free_item ){
	if( head != NULL ){
		list_node *new_head = head->next;
		if( b_free_item ){
			free( head->value );
			head->value = NULL;
		}
		// se Succede un un altro nodo lo collego il precedente con il successivo
		if( head->prev != NULL ){
			head->prev->next = head->next;
			
		}
		// stessa cosa se precede
		if( head->next != NULL ){
			head->next->prev = head->prev;
		}

		free( head );
		return new_head;
	}
	else{
		return NULL;
	}
}

/** // NON TESTATA E NON USATA
 * @brief Indica se i valori puntati da ogni nodo delle due liste sono uguali
 * 
 * @param head1 
 * @param head2 
 * @return true se tutti i valori puntati da ogni nodo delle due liste sono uguali
 * @return false altrimenti
 */
bool isEqual( list_node *head1, list_node *head2 ){
	char *val1 = (char*)head1->value;
	char *val2 = (char*)head2->value;
	if( ( head1 == NULL && head2 != NULL ) || ( head2 == NULL && head1 != NULL ) ){
		return false;
	}
	else if( *val1 == *val2 ){
		return true && isEqual( head1->next, head2->next );
	}
	else{
		return false;
	}
}

/**
 * @brief Dealloca i nodi della lista se b_delete_values = true allora dealloca i valori puntati dai nodi
 * 
 * @param head testa della lista da cui eliminare i nodi successivi
 * @param b_delete_values  (true) indica se deallocare i valori puntati dai nodi .(false) altrimenti
 */
void delete_list( list_node *head, bool b_delete_values ){
	if( head != NULL ){
		list_node *node = head;
		list_node *next = NULL;
		while( node != NULL ){
			next = node->next;
			if( b_delete_values ){
				free( node->value );
				node->value = NULL;
			}
			free( node );
			node = next;
		}
	}
}

/**
 * @brief Collega la lista  l_source dal puntatore dato a l_destination dalla testa assoluta
 * es: *d-e-s-t -> s-o-u-r-c-e
 * PostCondition: in ogni nodo di l_source viene assegnato l'handler di l_destination
 * @param l_destination 
 * @param l_source 
 * @return list_node* : puntatore alla testa dell'intera lista collegata
 */
list_node *append( list_node *l_destination, list_node *l_source){

	if( l_destination == NULL ){
		return l_source;
	}

	list_node *head_tmp = l_source;
	list_node *tail_old = last( l_destination );

	// collegamento liste
	l_source->prev = tail_old;
	tail_old->next = l_source;
	return l_destination;
}