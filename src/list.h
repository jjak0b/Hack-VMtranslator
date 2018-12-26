#ifndef LIST_H
#define LIST_H

struct list_node; // typedef struct list_node list_node; /*Bi-lista di caratteri*/
typedef struct list_node {
	void *value;						/*	puntatore a un indirizzo generico non specificato: Spiegazione...
										 *	Poiché Malloc restituisce un puntatore a void di uno spazio allocato di una dimensione X
										 *	non consente di accedere direttamente al valore puntato, poichè appunto il dato sarebbe "void", quindi sarà il chiamante
										 *	a occuparsi di utilizzare il puntatore del tipo di dato corretto.
										 *	inoltre, grazie a questo è possibile utilizzare la stessa struttura dati per inserire anche tipi di dato diversi tra loro
										 * 	senza riscrivere stesse porzioni di codice con il tipo cambiato inutilmente
										 */ 
	struct list_node *next;				/*puntatore al nodo successivo*/
	struct list_node *prev;				/*puntatore al nodo precedente*/
} list_node;

#include "utility.h"

/**
 * @brief Indica se la lista indicata a partire da tale nodo è vuota
 * 
 * @param head 
 * @return true se è vuota
 * @return false altrimenti
 */
bool isEmpty( list_node *head );

/**
 * @brief
 * PreCondition: N/A
 * PostCondition: Dato un nodo, ritorna la sua lunghezza della lista a seguire (o precedere) da tale nodo
 * se b_reachLast != 0 conta fino all'ultimo elemento della lista ritornado lunghezza positiva
 * altrimenti conta fino al primo elemento ritornando lunghezza positiva
 * @param head 
 * @return La lunghezza della lista
 */
int size( list_node *head, bool b_reachLast);

/**
 * @brief Data una lista ritorna il primo elemento della lista 
 * se la lista è vuota ritorna NULL
 * 
 * @param node 
 * @return list_node*  Il puntatore al primo nodo della lista
 */
list_node *first( list_node *node );

/**
 * @brief Datp una lista ritorna l'ultimo elemento della lista 
 * se la lista è vuota ritorna NULL
 * 
 * @param head 
 * @return list_node*  Il puntatore all'ultimo nodo della lista
 */
list_node *last( list_node *head );

/**
 * @brief 			Inserisce nella testa dello stack il puntatore passato
 * PreCondition: 	il chiamante deve gestirsi autonomamente l'allocamento del valore, passandone solo l'indirizzo
 * PostCondition:	Inserisce nella testa dello stack il puntatore passato
 * @param stack 	lista considerata stack
 * @param value 	Puntatore del valore da aggiungere alla lista
 * @return list_node* puntatore alla nuova testa dello stack
 */
list_node *push( list_node *stack, void *value );

/**
 * @brief Ritorna Il puntatore del valore presente in testa
 * PostCondition: se stack = NULL ritorna NULL
 * @param stack 
 * @return void* : Il puntatore del valore presente in testa
 */
void *top( list_node *stack);

/**
 * @brief Toglie l'elemento presente in testa e lo ritorna
 * PreCondition:	l'argomento poassato deve essere l'indirizzo del puntatore allo stack
 * PostCondition:	La testa dello stack passato viene modificata: Lo stack punta al elemento successivo alla testa
 * @param stack 
 * @return list_node* : il nodo rimosso dalla testa
 */
list_node *pop( list_node **stack );

/** // NON TESTATA E NON USATA
 * @brief Il nodo alla "index-esima" posizione
 * PreCondition: 	N/A
 * PostCondition:	Data una lista e un indice ritorna il nodo alla "index-esima"  posizione (anche negativa ) a partire dal nodo head
 * 					ritorna NULL se head == NULL oppure index < 0 oppure index è superiore (outofbound) alla lunghezza fino agli estremi
 *
 * @param head 
 * @param index 
 * @return list_node* Il nodo alla "index-esima" posizione
 */
list_node *get( list_node *head, int index );

/**
 * @brief 
 * PreCondition: N/A
 * PostCondition: Dati due nodi di lista e un indice, ritorna la prima lista con inserita la seconda a partire dalla posizione 'index-esima'
 * e collega l'ultimo elemento della seconda lista con la parte restante della prima, ovvero dalla posizione 'index'+1 della prima lista.
 * (Funziona anche con index negativo)
 * es: insert( c-i-a-o, c-i-a-o, 2 ) ->c-i-c-i-a-o-a-o
 * 
 * @param head 
 * @param node 
 * @param index 
 * @return list_node* nodo alla posizione index dopo inserimento
 */
list_node *insert( list_node *head, list_node *node, int index );

/**
 * @brief 
 * 	PreCondition:
		si suppone che b_create_handler sia = 1 solo se si vuole creare una nuova lista completa ( e non un singolo nodo )
		mentre se è già presente una lista ma si vuole aggiungere un nodo, il chiamante deve passare una struttura "handler" di una lista precedentemente creata
	PostCondition:
		Crea una nuovo nodo di una lista char assegnando il valore specificato ad esso.
		se "b_create_handler" = 0 assegna il parametro "handler" come struttura "handler" della nuova lista che verrà creata
		se "b_create_handler" != 0 alloca nuova memoria al puntatore "handler" quindi crea una nuova struttura condivisa con tutti nodi della lista:
			la struttura "handler" contiene sempre il puntatore alla testa (->head) ed alla coda (->tail)
 * 
 * @param value 
 * @param b_create_handler 
 * @param handler 
 * @return list_node* 
 */
list_node *list_node_new( void *value );

/**
 * @brief Dealloca il valore ed il nodo specificato, ritornando il nodo successivo
 *
 * PostCondition: Collega prima tra loro il nodo successivo e precedente
 * 
 * @param node 
 * @return list_node* Il puntatore al nodo successivo
 */
list_node *delete_node( list_node *node);

/**
 * @brief data una lista restituisce la lista invertita
 * 
 * @param head 
 * @return list_node* : puntatore alla nuova testa
 */
list_node *list_node_reverse( list_node *head );

/**
 * @brief Stampa i valori degli elementi di una
 * Precondition: valore puntato da head->value deve essere un tipo supportato da printf (es char*, char, int, float, ecc..)
 * 
 * @param format formato di stampa del valore: la stringa viene passata a printf, quindi usare come se fosse tale funzione
 * @param head testa della lista da cui verranno stampati gli elementi
 */
void list_node_print( const char *format, list_node *head);

/**
 * @brief istanzia un nuovo nodo e vi assegna il puntatore dato, restituisce la testa della lista aggiornata
 * PostCondition: se handler = NULL viene istanziata e restituita un nuovo handler ( a cui assegna comunque il puntatore dato alla lista )
 * @param head
 * @param value 
 * @return list_node* 
 */
list_node *enqueue( list_node *head, void *value );

/**
 * @brief rimuove dalla coda il primo nodo
 * PostCondition: 	se head = NULL restiuisce NULL
 * 					se b_free_item  = true allora viene deallocato lo spazio allocato da head->value, altrimenti dealloca solamete il nodo
 * @param handler 
 * @param b_free_item
 * @return list_node* della nuova testa della lista
 */
list_node *dequeue( list_node *head, bool b_free_item );

/** // NON TESTATA E NON USATA
 * @brief Indica se i valori puntati da ogni nodo delle due liste sono uguali
 * 
 * @param head1 
 * @param head2 
 * @return true se tutti i valori puntati da ogni nodo delle due liste sono uguali
 * @return false altrimenti
 */
bool isEqual( list_node *head1, list_node *head2 );

/**
 * @brief Dealloca i nodi della lista se b_delete_values = true allora dealloca i valori puntati dai nodi
 * 
 * @param head testa della lista da cui eliminare i nodi successivi
 * @param b_delete_values  (true) indica se deallocare i valori puntati dai nodi .(false) altrimenti
 */
void delete_list( list_node *head, bool b_delete_values );

/**
 * @brief Collega la lista  l_source dal puntatore dato a l_destination dalla testa assoluta
 * es: *d-e-s-t -> s-o-u-r-c-e
 * PostCondition: in ogni nodo di l_source viene assegnato l'handler di l_destination
 * @param l_destination 
 * @param l_source 
 * @return list_node* : puntatore alla testa dell'intera lista collegata
 */
list_node *append( list_node *l_destination, list_node *l_source);

#endif