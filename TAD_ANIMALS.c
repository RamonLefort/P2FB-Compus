#include <xc.h>
#include "TAD_ANIMALS.h"
#include "TAD_SIOTime.h"
#include "TAD_SIOInt.h"
#include "TAD_LCD.h"
#include "TAD_EEPROM.h"

//Debido a que nos pedían un mínimo de 3 animales por especie y un máximo de 24 animales
//y como no nos caben más animales por un tema de memoria RAM hemos establecido un límite de 12 animales
#define MAX_ANIMALS 12

static Animals animals[MAX_ANIMALS];
static unsigned char total_animals = 0;
static unsigned char rebuild_index = 0, eeprom_read = 0;
static unsigned char state = 0, i = 0;

// Arrays de estado: 0:Vaca, 1:Gallina, 2:Cavall, 3:Porc
static unsigned char num_especie[4];
static unsigned char awake_especie[4];
static unsigned char productos[4];
//Arrays de tiempo: 0:Vaca, 1:Gallina, 2:Cavall, 3:Porc
static unsigned char time_generation[4];
static unsigned long last_gen_timer[4];
static unsigned long last_prod_timer[4];

//Array de tiempos de producción: Vaca, Gallina, Cavall, Porc
const unsigned char prod_timeouts[4] = {47, 13, 23, 31}; 

static unsigned char rebellion = 0;
const char tipos_letra[] = {'V', 'G', 'C', 'P'};

void ANIMALS_Init(void) {
    //Inicializamos el sistema sin animales y sin rebeliones
    total_animals = rebellion = 0;
    unsigned long current_time = TIME_GetTimestamp();
    //Inicializamos las variables necesarias para la producción y generación de los animales
    for (unsigned char i = 0; i < 4; i++) {
        time_generation[i] = 0;
        num_especie[i] = 0;
        awake_especie[i] = 0;
        productos[i] = 0;
        last_gen_timer[i] = current_time;
        last_prod_timer[i] = current_time;
    }
}

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
            //Huevo frito = 1 huevo
            productos[1]--;
            break;
        case 1:
            //Tortilla con jamón = 1 huevo y 1 jamón
            productos[1]--;
            productos[3]--;
            break;
        case 2:
            //Cacaolat = 2 leches
            productos[0] -= 2;
            break;
        case 3:
            //Pintura = 2 pieles
            productos[2] -= 2;
            break;
    }
}

void ANIMALS_Awake(unsigned char index) {
    //Establecemos el estado de despertado del animal, guardamos la hora actual y actualizamos la variable animales despiertos
    animals[index].state = 'A';
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
    //Activamos el guardado de los animales en la EEPROM
    EE_WriteAll();
}

void Animal_Generation(unsigned char animal_type) {
    //Revisamos si ya hemos alcanzado el máximo de animales totales o de esa especie en concreto
    if (total_animals >= MAX_ANIMALS || num_especie[animal_type] > 2){
        return;
    }

    //Aádimos el animal la estructura de animales
    Animals *a = &animals[total_animals];
    a->type = tipos_letra[animal_type];
    a->number = num_especie[animal_type] + 1;
    //Enviamos la notificación al LCD
    LCD_PushMsg(2, animal_type, num_especie[animal_type] + 1, 0);
    //Activamos el guardado de los animales en la EEPROM
    EE_WriteAll();
    //Actualizamos las variables correspondientes
    num_especie[animal_type]++;
    ANIMALS_Awake(total_animals);
    total_animals++;
}

void Check_and_Generate(unsigned char animal_type) {
    //Revisamos si se ha inicializado el tiempo de generación
    if (time_generation[animal_type] == 0){
        return;
    }

    //Revisamos si se ha llegado al tiempo de generación
    if (TIME_HasElapsed(last_gen_timer[animal_type], time_generation[animal_type])) {
        //Generamos el animal
        Animal_Generation(animal_type);
        last_gen_timer[animal_type] = TIME_GetTimestamp(); 
    }
}

void Check_if_put_sleep(unsigned char index_animal) {
    //Revisamos si el animal ya está dormido
    if (animals[index_animal].state == 'S'){ 
        return;
    }
    
    //Miramos si han pasado 2 minutos desde la última vez que durmió
    if (TIME_HasElapsed(animals[index_animal].awake_timer, 120)) {
        //Ponemos a dormir al animal
        animals[index_animal].state = 'S';
        switch(animals[index_animal].type){
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
        case 0: //Revisamos los temporizadores de sueño de los animales
            if (i < total_animals){
                Check_if_put_sleep(i++);
            } else {
                i = 0;
                state++;
            }
            break;
            
        case 1: //Revisamos la generación y producción de los animales
            //Revisamos la generación de animales
            Check_and_Generate(i);
            
            //Revisamos la producción de los animales
            if (TIME_HasElapsed(last_prod_timer[i], prod_timeouts[i])) {
                last_prod_timer[i] = TIME_GetTimestamp();
                //Si no hay rebelión y hay algún animal despierto generamos productos
                if (!rebellion && awake_especie[i] > 0) {
                    productos[i] += awake_especie[i];
                    //Enviamos el mensaje de la generación de productos por el LCD
                    LCD_PushMsg(1, i, productos[i], 0);
                }
            }
            if (++i >= 4) { 
                i = 0;
                //Si hay que leer la EEPROM avanzamos al estado 3, sino volvemos al estado 0
                if (eeprom_read == 1) {
                    state++;
                }else{
                    state = 0;
                }
            }
            break;
        case 2: //Revisamos el array de animales para volver a montar el sistema correctamente
            if (rebuild_index == 0 && eeprom_read == 1) {
                //Reseteamos el sistema antes de empezar
                num_especie[0] = num_especie[1] = num_especie[2] = num_especie[3] = awake_especie[0] = awake_especie[1] = awake_especie[2] = awake_especie[3] = 0;
            }

            if (rebuild_index < total_animals) {
                //Revisamos su estado y modificamos las variables
                unsigned char t_idx = 0;
                switch(animals[rebuild_index].type) {
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
                if (animals[rebuild_index].state == 'A'){
                    awake_especie[t_idx]++;
                }
                rebuild_index++;
            } else {
                //Devolvemos el sistema a su funcionamiento normal
                rebuild_index = 0;
                eeprom_read = 0;
                state = 0;
            }
            break;
    }
}

void ANIMALS_Rebuild(void) {
    //Reiniciamos las variables para la lectura de la EEPROM
    rebuild_index = 0;
    eeprom_read = 1;
    //Devolvemos el motor al estado 0 para que revise el estado actual de los animales antes de reconstruir el sistema
    i = 0;
    state = 0;
}

void ANIMALS_PutTime(unsigned char idx, unsigned char t) {
    time_generation[idx] = t;
    //Actualizamos el último segundo de generación del animal
    last_gen_timer[idx] = TIME_GetTimestamp();
}