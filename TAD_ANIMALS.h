#ifndef TAD_ANIMALS_H
#define	TAD_ANIMALS_H

typedef struct {
    char type; //Tipo de animal ('V', 'G', 'C', 'P')
    unsigned char number; //Id
    char state; //Estado actual ('A' = Awake, 'S' = Sleep)
    unsigned long awake_timer; //Timestamp de cuándo se despertó, en segundos
} Animals;

void ANIMALS_Init(void);
//Pre: -
//Post: Inicializa los animales

void ANIMALS_PutTime(unsigned char idx, unsigned char t);
//Pre: 0 < id < 4
//Post: Inicializa el tiempo de creación 't' al animal 'idx'

void ANIMALS_Consume(char product);
//Pre: 0 < product < 4
//Post: Consume el producto que indique la variable 'producto'

void ANIMALS_Motor(void);
//Pre: -
//Post: Motor del sistema, se encarga de mirar el estado de sueño de los animales, la generación de animales y la producción

unsigned char ANIMALS_GetProd(unsigned char i);
//Pre: 0 < i < 4
//Post: Devuelve la cantidad de productos del índice 'i'

Animals* ANIMALS_GetAnimals(void);
//Pre: -
//Post: Devuelve la estructura de animales

unsigned char ANIMALS_GetTotalAnimals(void);
//Pre: -
//Post: Devuelve el total de animales de la estructura de animales

unsigned char* ANIMALS_GetAwakeEspecie();
//Pre: -
//Post: Devuelve la cantidad de animales despiertos de cada especie

unsigned char* ANIMALS_GetNumEspecie();
//Pre: -
//Post: Devuelve la cantidad de animales creados de cada especie

void ANIMALS_setTotalAnimals(unsigned char value);
//Pre: 0 < value < MAX_ANIMALS 
//Post: Introduce la cantidad de animales del sistema

void ANIMALS_Awake(unsigned char index);
//Pre: 0 < index < ANIMALS_GetTotalAnimals();
//Post: Despierta al animal correspondiente sengún la variable 'index'

void ANIMALS_Rebuild(void);
//Pre: -
//Post: Activa la inicialización de la estructura de animales

void ANIMALS_StopRebellion();
//Pre: -
//Post: Para la rebelión en la granja

void ANIMALS_StartRebellion();
//Pre: -
//Post: Inicia la rebelión en la granja

#endif	/* TAD_ANIMALS_H */

