#include "TAD_ANIMALS.h"
#include "TIMER.h"

#define MAX_ANIMALS 24
#define NUM_TYPES 4

struct Animals{
    char number = 0;
    char type = 0;
    char state = 0;
    unsigned char last_sleep_day = 0;
    unsigned char last_sleep_month = 0;
    unsigned char last_sleep_hour = 0;
    unsigned char last_sleep_minute = 0;
    unsigned char last_sleep_second = 0;
};

typedef Animals animals[MAX_ANIMALS];

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
unsigned char TimerGenVacas, TimerGenCerdos, TimerGenGallinas, TimerGenCaballos;
unsigned char TimerProdVacas, TimerProdCerdos, TimerProdGallinas, TimerProdCaballos;
static unsigned char num_milk = 0;
static unsigned char num_eggs = 0;
static unsigned char num_brush = 0;
static unsigned char num_meat = 0;
static unsigned char rebellion = 0;

void ANIMALS_Init(void){
    unsigned char i;

    total_animals = 0;

    for (i = 0; i < NUM_TYPES; i++) {
        count_by_type[i] = 0;
    }
    
    for (i = 0; i < MAX_ANIMALS; i++) {
        animals[i].number = 0;
        animals[i].type = "V"; // V vaca, G per gallines, C per cavalls, P per porcs (EEPROM)
        
        animals[i].state = "A"; // A for Awake, S for Sleep

        animals[i].last_sleep_day = 0; //EEPROM
        animals[i].last_sleep_month = 0; //EEPROM
        animals[i].last_sleep_hour = 0; //EEPROM
        animals[i].last_sleep_minute = 0; //EEPROM
        animals[i].last_sleep_second = 0; //EEPROM
    }
}

void start_rebellion(){
    rebellion = 1;
}

void Create_Production_Timer(unsigned char type){
    switch(type){
        case "V":
            if(num_vacas == 1){
                TI_NewTimer(&TimerProdVacas);
            }
            break;
        case "G":
            if(num_gallinas == 1){
                TI_NewTimer(&TimerProdGallinas);
            }
            break;
        case "C":
            if(num_caballos == 1){
                TI_NewTimer(&TimerProdCaballos);
            }
            break;
        case "P":
            if(num_cerdos == 1){
                TI_NewTimer(&TimerProdCerdos);
            }
            break;     
    }
}


// SOLO DEBE CONTAR EN PRODUCCION CUANDO ESTE DESPIERTO O SIMPLEMENTE SI EN EL MOMENTO DE LOS 2 MINS SI ESTA DORMIDO NO PRODUCE?
void Check_Prod_time(unsigned char type){
    unsigned char tics_meat = 66666;
    unsigned char tics_milk = 66666; // los tics se hardcodean
    unsigned char tic_eggs = 66666;
    unsigned char tics_brush = 66666;
    
    switch(type){
        case "V":
            if(TI_GetTics(TimerProdVacas) = tics_milk && !rebellion){
                num_milk += awake_vacas;
            }
            break;
        case "G":
            if(TI_GetTics(TimerProdGallinas) = tic_eggs && !rebellion){
                num_eggs += awake_gallinas;
            }
            break;
        case "C":
            if(TI_GetTics(TimerProdCaballos) = tics_brush && !rebellion){
                num_brush +=  awake_caballos
            }
            break;
        case "P":
            if(TI_GetTics(&TimerProdCerdos) = tics_meat && !rebellion){
                num_meat += awake_cerdos;
            }
            break;     
    }  
}


void Create_Generation_Timer(type){
    switch(type){
        case "V":
            if(num_vacas == 1){
                TI_NewTimer(&TimerGenVacas);
            }
            break;
        case "G":
            if(num_gallinas == 1){
                TI_NewTimer(&TimerGenGallinas);
            }
            break;
        case "C":
            if(num_caballos == 1){
                TI_NewTimer(&TimerGenCaballos);
            }
            break;
        case "P":
            if(num_cerdos == 1){
                TI_NewTimer(&TimerGenCerdos);
            }
            break;     
    }
}


unsigned char Check_generation_time(unsigned char type, unsigned char input_interface_time){
    unsigned char isTime = 0;
    
    switch(type){
        case "V":
            if(TI_GetTics(TimerGenVacas)*1000 = input_interface_time){
                isTime = 1;
            }
            break;
        case "G":
            if(TI_GetTics(TimerGenGallinas)*1000 = input_interface_time){
                isTime = 1;
            }
            break;
        case "C":
            if(TI_GetTics(TimerGenCaballos)*1000 = input_interface_time){
                isTime = 1;
            }
            break;
        case "P":
            if(TI_GetTics(&TimerGenCerdos)* 1000 = input_interface_time){
                isTime = 1;
            }
            break;     
    }
    return isTime;
}


// Se entra cada vez que el timer llega a los tiempos de generacion
void Animal_Generation(unsigned char index, unsigned char type){ 
    // por default los animales estan despiertos y son vacas
    total_animals++;
    annimal[index]type = type;
    switch(type){
        
        case "V":
            if(num_vacas < 3){
                awake_vacas++;
                input_actual_time(animals, index);
                animals[index].number = num_vacas++;  
            }
            break;
        
        case "G":
            if(num_gallinas < 3){
                awake_gallinas++;
                input_actual_time(animals, index);
                animals[index].number = num_gallinas++;  
                animals[index].type = "G";
            }
            break;
                
        case "P":
            if(num_cerdos < 3){
                awake_cerdos++;
                input_actual_time(animals, index);
                animals[index].number = num_cerdos++;  
                animals[index].type = "P";
            }
            break; 
            
        case "C":
            if(num_caballos < 3){
                awake_caballos++;
                input_actual_time(animals, index);
                animals[index].number = num_caballos++;  
                animals[index].type = "C";
            }
            break;     
    }
}


void Check_if_put_sleep(unsigned char index, unsigned char imported_value){
    if(animal[index].last_sleep_month >= imported_value_month || animal[index].last_sleep_year >= imported_value_year || animal[index].last_sleep_minutes - imported_value_minutes <= 2){
        if(imported_value_minute - animal[index].last_sleep_minutes <= 2 && imported_value_seconds <  animal[index].last_sleep_seconds){
           return;
        } 
    }
    animal[index].state = "S";
    switch(animal[index].type){
        case "V":
            awake_vacas--;
            break;
        case "C":
            awake_caballos--;
            break;
        case "G":
            awake_gallinas--;
            break;
        case "P":
            awake_cerdos--;
            break;
    }
}


void awake_animal(unsigned char index){
    animal[index].last_sleep_day = 0; // no es 0, es valor input del SIO timer 
    animal[index].last_sleep_month = 0;// no es 0, es valor input del SIO timer 
    animal[index].last_sleep_hour = 0;// no es 0, es valor input del SIO timer 
    animal[index].last_sleep_minute = 0;// no es 0, es valor input del SIO timer 
    animal[index].last_sleep_second = 0;// no es 0, es valor input del SIO timer 
    animals[index].state = "A";
}


void consume_area(interface_option){
    switch(interface_option){
        case 0:
            if(num_eggs >= 1){
                num eggs--;
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

void motor_Animals(){
    static unsigned char state = 0;
    static unsigned char i = 0;
    
    switch(state){
        case 0:
            if(i < total_animals){
                Check_if_put_sleep(i); // hace falta añadir los valores de input de la SIO
                i++;
            }
            else{
                state = 1;
                i = 0;
            }
            break;
        case 1:
            if(i < NUM_TYPES){
                Check_generation_time(type,input_interface_time);
                Check_Prod_time(type);
            }
            else{
                state = 0;
                i = 0;
            }
            break;
    }
}




