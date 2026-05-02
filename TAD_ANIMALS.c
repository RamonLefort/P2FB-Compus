#include <xc.h>
#include "TAD_ANIMALS.h"
#include "TAD_TIMER.h"
#include "TAD_SIOTime.h"

#define MAX_ANIMALS 24
#define NUM_TYPES 4

typedef struct{
    char number;
    char type;
    char state;
    unsigned char last_sleep_day;
    unsigned char last_sleep_month;
    unsigned char last_sleep_hour;
    unsigned char last_sleep_minute;
    unsigned char last_sleep_second;
} Animals;

static Animals animals[MAX_ANIMALS];

static unsigned char total_animals = 0;
static unsigned char count_by_type[NUM_TYPES];
static unsigned char awake_vacas = 0;
static unsigned char awake_gallinas = 0;
static unsigned char awake_caballos = 0;
static unsigned char awake_cerdos = 0;
static unsigned char num_vacas = 1;
static unsigned char num_cerdos = 1;
static unsigned char num_caballos = 1;
static unsigned char num_gallinas = 1;
unsigned char TimerProdVacas, TimerProdCerdos, TimerProdGallinas, TimerProdCaballos;
static unsigned char num_milk = 0;
static unsigned char num_eggs = 0;
static unsigned char num_brush = 0;
static unsigned char num_meat = 0;
static unsigned char rebellion = 0;
static unsigned char last_gen_sec_V = 0;
static unsigned char last_gen_sec_G = 0;
static unsigned char last_gen_sec_C = 0;
static unsigned char last_gen_sec_P = 0;

void ANIMALS_Init(void){
    unsigned char i;

    total_animals = last_gen_sec_V = last_gen_sec_G = last_gen_sec_C = last_gen_sec_P = 0;

    for (i = 0; i < NUM_TYPES; i++) {
        count_by_type[i] = 0;
    }
    
    for (i = 0; i < MAX_ANIMALS; i++) {
        animals[i].number = 0;
        animals[i].type = 'V'; // V vaca, G per gallines, C per cavalls, P per porcs (EEPROM)
        animals[i].state = 'A'; // A for Awake, S for Sleep

        animals[i].last_sleep_day = 0;
        animals[i].last_sleep_month = 0;
        animals[i].last_sleep_hour = 0;
        animals[i].last_sleep_minute = 0;
        animals[i].last_sleep_second = 0;
    }
}

void start_rebellion(){
    rebellion = 1;
}

void Create_Production_Timer(unsigned char type){
    switch(type){
        case 'V':
            if(num_vacas == 1){
                TI_NewTimer(&TimerProdVacas);
            }
            break;
        case 'G':
            if(num_gallinas == 1){
                TI_NewTimer(&TimerProdGallinas);
            }
            break;
        case 'C':
            if(num_caballos == 1){
                TI_NewTimer(&TimerProdCaballos);
            }
            break;
        case 'P':
            if(num_cerdos == 1){
                TI_NewTimer(&TimerProdCerdos);
            }
            break;     
    }
}


// SOLO DEBE CONTAR EN PRODUCCION CUANDO ESTE DESPIERTO O SIMPLEMENTE SI EN EL MOMENTO DE LOS 2 MINS SI ESTA DORMIDO NO PRODUCE?
void Check_Prod_time(unsigned char type){
    unsigned char tics_meat = 255;
    unsigned char tics_milk = 255; // los tics se hardcodean
    unsigned char tic_eggs = 255;
    unsigned char tics_brush = 255;
    
    switch(type){
        case 'V':
            if(TI_GetTics(TimerProdVacas) == tics_milk && !rebellion){
                num_milk += awake_vacas;
            }
            break;
        case 'G':
            if(TI_GetTics(TimerProdGallinas) == tic_eggs && !rebellion){
                num_eggs += awake_gallinas;
            }
            break;
        case 'C':
            if(TI_GetTics(TimerProdCaballos) == tics_brush && !rebellion){
                num_brush += awake_caballos;
            }
            break;
        case 'P':
            if(TI_GetTics(TimerProdCerdos) == tics_meat && !rebellion){
                num_meat += awake_cerdos;
            }
            break;     
    }  
}

void ANIMALS_PutGenerationTimes(unsigned char timeVaca, unsigned char timeGallina, unsigned char timeCavallo, unsigned char timeCerdo){
    count_by_type[0] = timeVaca;
    count_by_type[1] = timeGallina;
    count_by_type[2] = timeCavallo;
    count_by_type[3] = timeCerdo;
}

void awake_animal(unsigned char index){
    animals[index].last_sleep_month = TIME_GetMonth();
    animals[index].last_sleep_day = TIME_GetDay();
    animals[index].last_sleep_hour = TIME_GetHour();
    animals[index].last_sleep_minute = TIME_GetMinute();
    animals[index].last_sleep_second = TIME_GetSecond();
    animals[index].state = 'A';
}

// Se entra cada vez que el timer llega a los tiempos de generacion
void Animal_Generation(unsigned char index, unsigned char type){ 
    // por default los animales estan despiertos y son vacas
    total_animals++;
    animals[index].type = type;
    switch(type){
        case 'V':
            if(num_vacas < 3){
                awake_vacas++;
                awake_animal(index);
                animals[index].number = num_vacas++;  
                animals[index].type = 'V';
            }
            break;
        
        case 'G':
            if(num_gallinas < 3){
                awake_gallinas++;
                awake_animal(index);
                animals[index].number = num_gallinas++;  
                animals[index].type = 'G';
            }
            break;
                
        case 'P':
            if(num_cerdos < 3){
                awake_cerdos++;
                awake_animal(index);
                animals[index].number = num_cerdos++;  
                animals[index].type = 'P';
            }
            break; 
            
        case 'C':
            if(num_caballos < 3){
                awake_caballos++;
                awake_animal(index);
                animals[index].number = num_caballos++;  
                animals[index].type = 'C';
            }
            break;     
    }
}

void Check_generation_time(unsigned char type, unsigned char interval_needed) {
    unsigned char current_sec = TIME_GetSecond();
    unsigned char start_sec;
    unsigned char elapsed;

    // 1. Seleccionar el tiempo de referencia según el tipo
    switch(type) {
        case 0: start_sec = last_gen_sec_V; break;
        case 1: start_sec = last_gen_sec_G; break;
        case 2: start_sec = last_gen_sec_C; break;
        case 3: start_sec = last_gen_sec_P; break;
        default: return;
    }

    // 2. Cálculo del tiempo transcurrido (Aritmética modular base 60)
    if (current_sec >= start_sec) {
        elapsed = current_sec - start_sec;
    } else {
        elapsed = (current_sec + 60) - start_sec;
    }

    // 3. Si ha pasado el tiempo, ejecutamos la función específica y actualizamos
    if (elapsed >= interval_needed) {
        
        switch(type) {
            case 0: 
                Animal_Generation(total_animals, 'V');
                last_gen_sec_V = current_sec; 
                break;
            case 1: 
                Animal_Generation(total_animals, 'G'); 
                last_gen_sec_G = current_sec; 
                break;
            case 2: 
                Animal_Generation(total_animals, 'C');
                last_gen_sec_C = current_sec;
                break;
            case 3: 
                Animal_Generation(total_animals, 'P');
                last_gen_sec_P = current_sec; 
                break;
        }
    }
}

void Check_if_put_sleep(unsigned char index){
    if(animals[index].last_sleep_month >= TIME_GetMonth() || animals[index].last_sleep_day >= TIME_GetDay() || animals[index].last_sleep_hour - TIME_GetHour() <= 2){
        if(TIME_GetMinute() - animals[index].last_sleep_minute <= 2 && TIME_GetSecond() < animals[index].last_sleep_second){
           return;
        } 
    }
    animals[index].state = 'S';
    switch(animals[index].type){
        case 'V':
            awake_vacas--;
            break;
        case 'C':
            awake_caballos--;
            break;
        case 'G':
            awake_gallinas--;
            break;
        case 'P':
            awake_cerdos--;
            break;
    }
}

void ANIMALS_Consume(unsigned char interface_option){
    switch(interface_option){
        case 0:
            if(num_eggs >= 1){
                num_eggs--;
                // mostrar por LCD
            }
            break;
        case 1:
            if(num_eggs >= 1 && num_meat >= 1){
                num_eggs--;
                num_meat--;
                 // mostrar por LCD
            }
            break;
        case 2:
            if(num_milk >= 2){
                num_milk--;
                num_milk--;
                // mostrar por LCD
            }
            break;
        case 3:
            if(num_brush >= 2){
                num_brush--;
                num_brush--;
                // mostrar por LCD
            }
            break;
        case 4:
            // ir atras en la interfaz
            break;     
    }
}

void ANIMALS_Motor(){
    static unsigned char state = 0;
    static unsigned char i = 0;
    
    switch(state){
        case 0:
            if(i < total_animals){
                Check_if_put_sleep(i); // hace falta añadir los valores de input de la SIO
                i++;
            }else{
                state = 1;
                i = 0;
            }
            break;
        case 1:
            if(i < NUM_TYPES){
                Check_generation_time(i, count_by_type[i]);
                Check_Prod_time(i);
            }
            else{
                state = 0;
                i = 0;
            }
            break;
    }
}