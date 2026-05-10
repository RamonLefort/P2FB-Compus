#include <xc.h>
#include "TAD_ANIMALS.h"
#include "TAD_SIOTime.h"
#include "TAD_SIOInt.h"
#include "TAD_LCD.h"
#include "TAD_EEPROM.h"

#define MAX_ANIMALS 12 // Reducido para ahorrar RAM (3 de cada tipo)

static Animals animals[MAX_ANIMALS];
static unsigned char total_animals = 0;
static unsigned char rebuild_idx = 0, eeprom_read = 0;
static unsigned char state = 0, i = 0;

// Arrays de estado: 0:Vaca, 1:Gallina, 2:Cavall, 3:Porc
static unsigned char num_especie[4];
static unsigned char awake_especie[4];
static unsigned char productos[4];

// --- GESTIÓN DEL TIEMPO LINEAL (Uptime) ---
static unsigned char time_generation[4];  // Frecuencia configurada
static unsigned long last_gen_timer[4];   // Marca de tiempo de la última generación
static unsigned long last_prod_timer[4];  // Marca de tiempo de la última producción

// Array constante en ROM: Tiempos de producción {Vaca, Gallina, Cavall, Porc}
const unsigned char prod_timeouts[4] = {47, 13, 23, 31}; 

static unsigned char rebellion = 0;
const char tipos_letra[] = {'V', 'G', 'C', 'P'};

void ANIMALS_Init(void) {
    total_animals = rebellion = 0;
    
    unsigned long current_time = TIME_GetTimestamp();
    for (unsigned char i = 0; i < 4; i++) {
        time_generation[i] = 0;
        num_especie[i] = 0;
        awake_especie[i] = 0;
        productos[i] = 0;
        
        // Inicializamos los contadores al tiempo actual
        last_gen_timer[i] = current_time;
        last_prod_timer[i] = current_time;
    }
}

// Getters 
unsigned char ANIMALS_GetProd(unsigned char i) { 
    return productos[i]; 
}

Animals* ANIMALS_GetAnimals(void) { 
    return animals; 
}

unsigned char ANIMALS_GetTotalAnimals(void){
    return total_animals; 
}

unsigned char* ANIMALS_GetNumEspecie(){
    return num_especie; 
}

unsigned char* ANIMALS_GetAwakeEspecie(){
    return awake_especie; 
}

void ANIMALS_setTotalAnimals(unsigned char value) { 
    total_animals = value; 
}

void ANIMALS_StartRebellion(){ 
    rebellion = 1;
}

void ANIMALS_StopRebellion(){ 
    rebellion = 0;
}

void ANIMALS_Consume(char product){
    //0 = Llet, 1 = Ous, 2 = Pinzell, 3 = Pernil
    switch(product){
        case 0:
            productos[1]--;
            break;
        case 1:
            productos[1]--;
            productos[3]--;
            break;
        case 2:
            productos[0] -= 2;
            break;
        case 3:
            productos[2] -= 2;
            break;
    }
}

void ANIMALS_Awake(unsigned char index) {
    animals[index].state = 'A';
    // Guardamos la "foto" exacta de cuándo se despertó
    animals[index].awake_timer = TIME_GetTimestamp();
    switch(animals[index].type){
        case 'V':
            awake_especie[0]++;
            break;
        case 'G':
            awake_especie[1]++;
            break;
        case 'C':
            awake_especie[2]++;
            break;
        case 'P':
            awake_especie[3]++;
            break;
    }
    EEMOTOR_TriggerWriteAll();
}

void Animal_Generation(unsigned char type_idx) {
    if (total_animals >= MAX_ANIMALS || num_especie[type_idx] > 2){
        return;
    }

    Animals *a = &animals[total_animals];
    a->type = tipos_letra[type_idx];
    a->number = num_especie[type_idx] + 1;
    LCD_PushMsg(2, type_idx, num_especie[type_idx] + 1, 0);
    EEMOTOR_TriggerWriteAll();

    num_especie[type_idx]++;
    ANIMALS_Awake(total_animals);
    total_animals++;
}

void Check_and_Generate(unsigned char i) {
    // Si la generación está apagada, salimos O(1)
    if (time_generation[i] == 0){
        return;
    }

    // Lógica Uptime: Completamente segura contra desbordamientos
    if (TIME_HasElapsed(last_gen_timer[i], time_generation[i])) {
        Animal_Generation(i);
        // Actualizamos la marca de tiempo para el próximo ciclo
        last_gen_timer[i] = TIME_GetTimestamp(); 
    }
}

void Check_if_put_sleep(unsigned char index) {
    if (animals[index].state == 'S'){ 
        return;
    }
    
    // 120 segundos = 2 minutos exactos de máquina, sin importar el calendario
    if (TIME_HasElapsed(animals[index].awake_timer, 120)) {
        animals[index].state = 'S';
        
        // Cacheamos el tipo para no iterar el struct repetidamente
        switch(animals[index].type){
            case 'V':
                awake_especie[0]--;
                break;
            case 'G':
                awake_especie[1]--;
                break;
            case 'C':
                awake_especie[2]--;
                break;
            case 'P':
                awake_especie[3]--;
                break;
        }
    }
}

void ANIMALS_Motor(void) {
    
    switch(state) {
        case 0: // Chequear sueño (1 animal por vuelta cooperativa)
            if (i < total_animals){
                Check_if_put_sleep(i++);
            } else {
                i = 0;
                state++; // Pasamos a la fase de granja
            }
            break;
            
        case 1: // Generación y Producción (1 especie por vuelta cooperativa)
            // 1. Fase de Nacimientos
            Check_and_Generate(i);
            
            // 2. Fase de Producción (Corregida con Uptime)
            // Usamos la tabla de ROM 'prod_timeouts' para no usar IFs
            if (TIME_HasElapsed(last_prod_timer[i], prod_timeouts[i])) {
                last_prod_timer[i] = TIME_GetTimestamp(); // Reiniciamos el cronómetro de esta especie
                // Solo producen si no hay rebelión y si hay animales despiertos
                if (!rebellion && awake_especie[i] > 0) {
                    productos[i] += awake_especie[i];
                    LCD_PushMsg(1, i, productos[i], 0);
                }
            }
            
            // Avanzamos al siguiente tipo de animal de forma cíclica
            if (++i >= 4) { 
                i = 0;
                if (eeprom_read == 1) {
                    state++;
                }else{
                    state = 0;
                }
            }
            break;
        case 2: // --- ESTADO 2: REBUILD COOPERATIVO (NUEVO) ---
            // Este estado solo se activa externamente tras cargar la EEPROM
            if (rebuild_idx == 0 && eeprom_read == 1) {
                // Reset de contadores solo al empezar el barrido
                num_especie[0] = num_especie[1] = num_especie[2] = num_especie[3] = awake_especie[0] = awake_especie[1] = awake_especie[2] = awake_especie[3] = 0;
            }

            if (rebuild_idx < total_animals) {
                // Procesamos UN animal en esta llamada
                unsigned char t_idx = 0;
                switch(animals[rebuild_idx].type) {
                    case 'V':
                        t_idx = 0; 
                        break;
                    case 'G':
                        t_idx = 1;
                        break;
                    case 'C':
                        t_idx = 2;
                        break;
                    case 'P':
                        t_idx = 3;
                        break;
                }
                num_especie[t_idx]++;
                if (animals[rebuild_idx].state == 'A'){
                    awake_especie[t_idx]++;
                }
                
                rebuild_idx++; // En la siguiente vuelta del main() procesaremos el siguiente
            } else {
                rebuild_idx = 0;
                eeprom_read = 0;
                state = 0;
            }
            break;
    }
}

void ANIMALS_Rebuild(void) {
    rebuild_idx = 0;
    eeprom_read = 1;
    i = 0;
    state = 0;
}

void ANIMALS_PutTime(unsigned char idx, unsigned char t) {
    time_generation[idx] = t;
    // Al configurar un nuevo tiempo, reiniciamos el contador para que empiece desde cero
    last_gen_timer[idx] = TIME_GetTimestamp();
}