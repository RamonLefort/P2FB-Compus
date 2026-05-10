#include <xc.h>
#include "TAD_SIOInt.h"
#include "TAD_LCD.h"
#include "TAD_ANIMALS.h"
#include "TAD_SIOTime.h"
#include "TAD_TIMER.h"
#include "TAD_JSK.h"
#include "TAD_EEPROM.h"
#include "TAD_HB.h"

#define MAX_COMANDO 32
#define MASK_RX 0x1F

char comando[MAX_COMANDO], FarmName[17];
unsigned char Inicio, Fin, Quants;
Animals* animals;
const char* nombres[] = {"VACA\0", "GALLINA\0", "CAVALL\0", "PORC\0"};
const char* estados[] = {"AWAKE\0", "SLEEP\0"};
const unsigned char map_prod[] = {0, 3, 1, 2};
static unsigned char Timer;

void IFC_Init(){
    Inicio = Fin = Quants = 0;
    TI_NewTimer(&Timer);
}

void AddChar(char value){
    comando[Inicio++] = value;
    Inicio &= MASK_RX;
    Quants++;
}

char GetChar(){
    char value = comando[Fin++];
    Fin &= MASK_RX;
    Quants--;
    return value;
}

unsigned char atou(const char* s) {
    // 1. Asumimos siempre que el primer carácter es un número y lo convertimos.
    unsigned char res = s[0] - '0';
    
    // 2. Evaluamos estrictamente si el SEGUNDO carácter es un dígito válido.
    if (s[1] >= '0' && s[1] <= '9') {
        // Si hay un segundo dígito, significa que el primer carácter eran las DECENAS.
        // Aplicamos el álgebra de bits: multiplicamos el valor actual por 10 y sumamos las UNIDADES.
        res = (res << 3) + (res << 1) + (s[1] - '0');
    }
    // Si s[1] era '\0', un espacio, un '$' o basura de la SIO, el if se ignora
    // y devolvemos directamente el primer dígito como UNIDAD.
    return res;
}

char* IFC_GetFarmName(void) {
    return FarmName;
}

void IFC_Motor(){
    static unsigned char state = 0;
    static unsigned char cmdtype = 0, idx, word = 0, i, j, tipo_animal = 0, estado_animal = 0, target_num;
    static unsigned char bytes_procesados, total_payload;
    static char target_type, search_idx;
    static char str[16];
    
    switch(state){
        case 0:
            if(SIO_RXAvail()){
                char value = SIO_GetChar();
                if(value == '\r' || value == '\n'){
                    if(Quants > 0) state = 1;
                }else{
                    AddChar(value);
                }
            }
            break;
            
        case 1: {
            char c1 = comando[Fin];
            char c2 = comando[(Fin + 1) & MASK_RX]; 
            unsigned char es_cooperativo = 0;
            
            // Decodificación
            if (c1 == 'R' && c2 == 'S') {
                //SIO_PutString("Cmd: Reset\n");
                EEMOTOR_TriggerReset();
                LCD_ResetLCD();
                ANIMALS_Init();
            } 
            if (c1 == 'S') {
                switch(c2) {
                    case 'A': 
                        //SIO_PutString("Cmd: Sleep Animal\n");
                        cmdtype = 3;
                        idx = (Fin + 2) & MASK_RX;
                        total_payload = Quants - 2;
                        es_cooperativo = 1;
                        state++;
                        break;
                    case 'T': 
                        //SIO_PutString("Cmd: Start Rebellion\n");
                        ANIMALS_StartRebellion();
                        HB_setRebellion(1);
                        //TODO: Poner HB Led a 1
                        break;
                    case 'P':
                        //SIO_PutString("Cmd: Stop Rebellion\n");
                        ANIMALS_StopRebellion();
                        HB_setRebellion(0);
                        //TODO: Poner HB Led a 0
                        break;
                }
            }
            if (c1 == 'I') {
                //LSFarmSIO_PutString("Cmd: Inicializar\n");
                cmdtype = 1;
                idx = (Fin + 1) & MASK_RX;
                total_payload = Quants - 1;
                es_cooperativo = 1;
                EEMOTOR_TriggerReadAll();
                state++;
            }
            if (c1 == 'C') { 
                //SIO_PutString("Cmd: Consumir\n");
                cmdtype = 2;
                idx = (Fin + 1) & MASK_RX;
                total_payload = Quants - 1;
                es_cooperativo = 1;
                state++;
            }
            if (c1 == 'A') {
                //SIO_PutString("Cmd: Get Animal\n");
                animals = ANIMALS_GetAnimals();
                es_cooperativo = 1;
                state = 3;
            }
            if (c1 == 'P') {
                //SIO_PutString("Cmd: Get Products\n");
                es_cooperativo = 1;
                state = 4;
            }
            
            if (es_cooperativo) {
                word = 0;
                i = 0;
                bytes_procesados = 0;
            } else {
                Fin = (Fin + Quants) & MASK_RX;
                Quants = 0;
                state--;
            }
            break;
        }
            
        case 2: {
            char c = comando[idx];
            idx = (idx + 1) & MASK_RX; 
            bytes_procesados++;
            unsigned char fin_de_trama = (bytes_procesados == total_payload);

            switch(cmdtype){
                case 1: // 'I' (Nom, Vaca, Gallina, Cavall, Porc)
                    if(c == '$' || fin_de_trama){
                        if (c != '$'){
                            str[i++] = c;
                        }
                        str[i] = '\0';
                        //SIO_PutString(str);
                        //SIO_PutString("\r\n");
                        if (word == 0) {
                            FarmName[16] = '\0';
                            LCD_PushMsg(0, 0, TIME_GetDay(), TIME_GetMonth());
                        } else if (word >= 1 && word <= 4) {
                            ANIMALS_PutTime(word - 1, atou(str));
                        }
                        word++;
                        i = 0;
                    }else{
                        if(i < 16){
                            if(word == 0){
                                FarmName[i++] = c;
                            }else{
                                str[i++] = c;
                            }
                        }
                    }
                    
                    if(fin_de_trama){
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        state = 0;
                    }
                    break;
                    
                case 2: // 'C' (NumOpción)
                    if(SIO_TXAvail() >= 3){
                        ANIMALS_Consume(c - '0');
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        state = 0;
                    }
                    break;
                    
                case 3: // 'SA' (Animal, Num) SAVACA$1
                    
                    if (i == 0) {
                        // El primer carácter del nombre (posición 0) nos da el tipo ('V', 'G', 'C', 'P')
                        target_type = c;
                        //SIO_PutChar(target_type);
                        // Reseteamos el índice. El siguiente carácter que entre será el número.
                        i++;
                    }
                    
                    if(fin_de_trama && SIO_TXAvail() > 1){
                        target_num = c - '0';
                        //SIO_PutChar(target_num + '0');
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        search_idx = 0; // Reseteamos el índice de búsqueda
                        state = 5;      // Saltamos al nuevo estado buscador
                    }
                    break;
            }
            break;
        }
        case 3:
            if(i < ANIMALS_GetTotalAnimals()){
                switch(word) {
                    case 0:
                        if (SIO_TXAvail() >= MAX_RX) {
                            SIO_PutChar('D');
                            SIO_PutChar('A');
                            char type_cache = animals[i].type;
                            tipo_animal = 0; // Vaca por defecto
                            if(type_cache == 'G') tipo_animal = 1;
                            if(type_cache == 'C') tipo_animal = 2;
                            if(type_cache == 'P') tipo_animal = 3;

                            j = 0;
                            word++;
                        }
                        break;
                    case 1:
                        if (SIO_TXAvail() > 0) {
                            if (nombres[tipo_animal][j] != '\0') {
                                SIO_PutChar(nombres[tipo_animal][j++]); 
                            }else{
                                word++;
                            }
                        }
                        break;
                    case 2:
                        if (SIO_TXAvail() >= 3) {
                            SIO_PutChar('$');
                            SIO_PutChar(animals[i].number + '0');
                            SIO_PutChar('$');
                            estado_animal = 0; // 0 = AWAKE
                            if(animals[i].state == 'S'){
                                estado_animal = 1; // 1 = SLEEP
                            }
                            j = 0;
                            word++;
                        }
                        break;
                    case 3:
                        if (SIO_TXAvail() > 0) {
                            if (estados[estado_animal][j] != '\0') {
                                SIO_PutChar(estados[estado_animal][j++]); 
                            } else {
                                word++;
                            }
                        }
                        break;
                    case 4:
                        if (SIO_TXAvail() >= 2) {
                            SIO_PutChar('\r');
                            SIO_PutChar('\n');
                            word = 0; 
                            i++;
                        }
                        break;
                }
            }else{
                // Ya no hay más animales
                if (SIO_TXAvail() >= 3) {
                    SIO_PutChar('F');
                    SIO_PutChar('\r');
                    SIO_PutChar('\n');
                    Fin = (Fin + Quants) & MASK_RX;
                    Quants = 0;
                    state = 0;
                }
            }
            break;
        case 4:
            switch(word) {
                case 0:
                    if (SIO_TXAvail() >= 2) {
                        SIO_PutChar('D');
                        SIO_PutChar('P');
                        i = 0;
                        word++;
                    }
                    break;
                case 1:
                    if (SIO_TXAvail() >= 4) {
                        unsigned char num = ANIMALS_GetProd(map_prod[i]);
                        if(num >= 10){
                            SIO_PutChar((num / 10) + '0');
                        }
                        SIO_PutChar((num % 10) + '0');
                        if(i < 3){
                            SIO_PutChar('$');
                        }else{
                            SIO_PutChar('\r');
                            SIO_PutChar('\n');
                            Fin = (Fin + Quants) & MASK_RX;
                            Quants = 0;
                            state = 0;
                        }
                        i++;
                    }
                    break;
            }
            break;
        case 5: 
        {
            animals = ANIMALS_GetAnimals();
            unsigned char total = ANIMALS_GetTotalAnimals();
            
            // Evaluamos solo el índice actual
            if (search_idx < total) {
                if (animals[search_idx].type == target_type && animals[search_idx].number == target_num) {
                    TI_ResetTics(Timer);
                    // Pasamos al estado de espera condicional
                    state++;
                } else {
                    // Si no es este animal, avanzamos el índice. 
                    // El chequeo se hará en la SIGUIENTE llamada a IFC_Motor().
                    search_idx++;
                }
            } else {
                // Hemos recorrido toda la granja y no existe. Abortamos.
                state = 0;
            }
            break;
        }
        case 6:
        {   
            // CONDICIÓN A: Interrupción por hardware (Botón pulsado)
            if (JSK_getLight() == 1) {
                ANIMALS_Awake(search_idx);
                if(SIO_TXAvail() > 2){
                    SIO_PutChar('S');
                    SIO_PutChar('S');
                    state++;
                }
            }
            // CONDICIÓN B: Timeout de 5 segundos
            else if (TI_GetTics(Timer) >= 5000 && JSK_getLight() == 0) {
                if(SIO_TXAvail() > 2){
                    SIO_PutChar('S');
                    SIO_PutChar('U');
                    state++;
                }
            }
            break;
        }
        case 7:
            if(SIO_TXAvail() > 2){
                SIO_PutChar('\r');
                SIO_PutChar('\n');
                EEMOTOR_TriggerWriteAll();
                state = 0;
            }
            break;
    }
}