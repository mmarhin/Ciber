#include <stdio.h>
#include <stdlib.h>

typedef struct Nodo {
    int dato;
    struct Nodo siguiente;
} Nodo;

/*@null@*/ Nodo* crearNodo(int dato) {
    Nodo* nuevoNodo = (Nodo*)malloc(sizeof(Nodo));
    nuevoNodo->dato = dato;  
    nuevoNodo->siguiente = NULL;
    return nuevoNodo;
}

void push(/*@null@*/ /*@shared@*/Nodo** cima, int dato) {
    Nodo* nuevoNodo = crearNodo(dato);
    nuevoNodo->siguiente = *cima;
    *cima = nuevoNodo;
}
int pop(/*@null@*/ /*@shared@*/Nodo** cima) {
    if (*cima == NULL) {
        printf("La pila está vacía\n");
        return -1;
    }
    
    Nodo* temp = *cima;
    int dato = temp->dato;  // Guardamos el dato antes de liberar
    *cima = (*cima)->siguiente;
    
    free(temp); 

    printf("%d", temp->dato);

    return dato;
}

void imprimirPila(/*@null@*/ /*@in@*/ Nodo* cima) {
    while (cima != NULL) {
        printf("%d -> ", cima->dato);
        cima = cima->siguiente;
    }
    printf("NULL\n");
}

void liberarPila(/*@null@*/ /*@only@*/Nodo** cima) {
    Nodo* actual = *cima;
    while (actual != NULL) {
        Nodo* temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    *cima = NULL;}  


int main() {
    Nodo* pila = NULL;
    int n;

    // Solicitar al usuario el número de elementos
    printf("Ingrese el número de elementos a insertar en la pila: ");
    scanf("%d", &n);

    // Insertar los elementos en la pila (multiplicados por cuatro)
    int i;
    for (i = 1; i <= n; i++) {
        push(&pila, i * 4);
    }

    // Imprimir la pila inicial
    printf("Pila inicial:\n");
    imprimirPila(pila);

    // Proceso de manipulación de la pila
    int contador = 1;

    while (pila != NULL) {
	int i;
        for (i = 0; i < 2; i++) {
            if (pila != NULL) {
                printf("Elemento removido: %d\n", pop(&pila));
            }
        }

        // Insertar un nuevo elemento
        if (pila != NULL) {
            push(&pila, contador * 4);
            printf("Elemento insertado: %d\n", contador * 4);
            contador++;
        }

        imprimirPila(pila);
    }

    liberarPila(&pila);
    liberarPila(&pila);  

    return 0;
}
