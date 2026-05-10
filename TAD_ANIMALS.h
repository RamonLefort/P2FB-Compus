#ifndef TAD_ANIMALS_H
#define	TAD_ANIMALS_H

typedef struct {
    char type;                  // 1 byte: Tipo de animal ('V', 'G', 'C', 'P')
    unsigned char number;       // 1 byte: Identificador numérico (1, 2, 3...)
    char state;                 // 1 byte: Estado actual ('A' = Awake, 'S' = Sleep)
    unsigned long awake_timer;  // 4 bytes: Timestamp de máquina de cuándo se despertó
} Animals;

void ANIMALS_Init(void);

void ANIMALS_PutTime(unsigned char idx, unsigned char t);

void ANIMALS_Consume(char product);

void ANIMALS_Motor(void);

unsigned char ANIMALS_GetProd(unsigned char i);

Animals* ANIMALS_GetAnimals(void);

unsigned char ANIMALS_GetTotalAnimals(void);

unsigned char* ANIMALS_GetAwakeEspecie();

unsigned char* ANIMALS_GetNumEspecie();

void ANIMALS_setTotalAnimals(unsigned char value);

void ANIMALS_Awake(unsigned char index);

void ANIMALS_StopRebellion();

void ANIMALS_StartRebellion();

#endif	/* TAD_ANIMALS_H */

