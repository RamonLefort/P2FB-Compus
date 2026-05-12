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
const unsigned char map_gen[] = {0, 3, 2, 1};
static unsigned char Timer;

void IFC_Init(){
    Inicio = Fin = Quants = 0;
    TI_NewTimer(&Timer);
}

//Función para añadir un caracter a la variable del comando
void AddChar(char value){
    comando[Inicio++] = value;
    Inicio &= MASK_RX;
    Quants++;
}

//Función que convierte un valor de 2 carácteres en ASCI a valor en Char
unsigned char atou(const char* s) {
    //Convertimos el primer carácter
    unsigned char res = s[0] - '0';
    
    //Miramos si el segundo carácter es un número
    if (s[1] >= '0' && s[1] <= '9') {
        //Si hay un segundo carácter multiplicamos el anterior por 10 y le sumamos el nuevo
        res = (res << 3) + (res << 1) + (s[1] - '0');
    }
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
        case 0: //Estado de recolección de caracteres del comando
            if(SIO_RXAvail()){
                char value = SIO_GetChar();
                if(value == '\r' || value == '\n'){
                    if(Quants > 0) state = 1;
                }else{
                    AddChar(value);
                }
            }
            break;
            
        case 1: //Estado de identificación del comando
            char c1 = comando[Fin];
            char c2 = comando[(Fin + 1) & MASK_RX]; 
            unsigned char es_cooperativo = 0;
            
            if (c1 == 'R' && c2 == 'S') {
                //Comando de Reset, reseteamos el sistema
                EE_Reset();
                LCD_ResetLCD();
                ANIMALS_Init();
            } 
            if (c1 == 'S') {
                switch(c2) {
                    case 'A': 
                        //Comando de Sleep Animal, pasamos al estado de parsing del comando
                        cmdtype = 3;
                        idx = (Fin + 2) & MASK_RX;
                        total_payload = Quants - 2;
                        es_cooperativo = 1;
                        state++;
                        break;
                    case 'T': 
                        //Comando de Start Rebellion, iniciamos la rebelión
                        ANIMALS_StartRebellion();
                        HB_setRebellion(1);
                        break;
                    case 'P':
                        //Comando de Stop Rebellion, finalizamos la rebelión
                        ANIMALS_StopRebellion();
                        HB_setRebellion(0);
                        break;
                }
            }
            if (c1 == 'I') {
                //Comando de incialización del sistema, pasamos al estado de parsing del comando
                cmdtype = 1;
                idx = (Fin + 1) & MASK_RX;
                total_payload = Quants - 1;
                es_cooperativo = 1;
                EE_ReadAll();
                state++;
            }
            if (c1 == 'C') { 
                //Comando del consumo de productos, pasamos al estado de parsing del sistema
                cmdtype = 2;
                idx = (Fin + 1) & MASK_RX;
                total_payload = Quants - 1;
                es_cooperativo = 1;
                state++;
            }
            if (c1 == 'A') {
                //Comando de muestreo de animales, pasamos al estado de escritura del sistema
                animals = ANIMALS_GetAnimals();
                es_cooperativo = 1;
                state = 3;
            }
            if (c1 == 'P') {
                //Comando de muestreo de productos, pasamos al estado de escritura del sistema
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
            
        case 2:
            char c = comando[idx];
            idx = (idx + 1) & MASK_RX; 
            bytes_procesados++;
            unsigned char fin_de_trama = (bytes_procesados == total_payload);

            switch(cmdtype){
                case 1: //Comando de inicialización del sistema 'I' (Nom$TempsVaca$TempsGallina$TempsCavall$TempsPorc)
                    if(c == '$' || fin_de_trama){
                        if (c != '$'){
                            str[i++] = c;
                        }
                        str[i] = '\0';
                        //SIO_PutString(str);
                        //SIO_PutString("\r\n");
                        if (word == 0) {
                            //Si es la primera palabra sabemos que es el nombre de la granja
                            FarmName[16] = '\0';
                            //Actualizamos el mensje de bienvenida
                            LCD_PushMsg(0, 0, TIME_GetDay(), TIME_GetMonth());
                        } else if (word >= 1 && word <= 4) {
                            //Si no es ula primera palabra sabemos que es el tiempo de generación de algún animal
                            ANIMALS_PutTime(map_gen[word - 1], atou(str));
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
                        //Miramos el número de opción que es y lo ejecutamos
                        ANIMALS_Consume(c - '0');
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        state = 0;
                    }
                    break;
                    
                case 3: // 'SA' (Animal$Num)
                    
                    if (i == 0) {
                        //Sabemos que el primer caracter nos dará el tipo ('V', 'G', 'C', 'P')
                        target_type = c;
                        //SIO_PutChar(target_type);
                        i++;
                    }
                    
                    if(fin_de_trama && SIO_TXAvail() > 1){
                        //Sabemos que el anterior carácter a acabar la trama es el número del animal
                        target_num = c - '0';
                        //SIO_PutChar(target_num + '0');
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        search_idx = 0;
                        //Nos vamos al estado de búsqueda del animal
                        state = 5;
                    }
                    break;
            }
            break;
        case 3: //Estado de envío de los animales por la SIO
            if(i < ANIMALS_GetTotalAnimals()){
                switch(word) {
                    case 0: //Enviamos la clave del comando 'DA'
                        if (SIO_TXAvail() >= MAX_RX) {
                            SIO_PutChar('D');
                            SIO_PutChar('A');
                            //Identificamos el tipo de animal
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
                        //Enviamos el tipo de animal, todo el nombre del tipo de animal, letra por letra
                        if (SIO_TXAvail() > 0) {
                            if (nombres[tipo_animal][j] != '\0') {
                                SIO_PutChar(nombres[tipo_animal][j++]); 
                            }else{
                                word++;
                            }
                        }
                        break;
                    case 2: //Ponemos el delimitador '$', enviamos el número del animal y miramos su estado
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
                    case 3: //Enviamos el estado de animal, toda la palabra del estado de animal, letra por letra
                        if (SIO_TXAvail() > 0) {
                            if (estados[estado_animal][j] != '\0') {
                                SIO_PutChar(estados[estado_animal][j++]); 
                            } else {
                                word++;
                            }
                        }
                        break;
                    case 4: //Enviamos la finalización del comando
                        if (SIO_TXAvail() >= 2) {
                            SIO_PutChar('\r');
                            SIO_PutChar('\n');
                            word = 0; 
                            i++;
                        }
                        break;
                }
            }else{
                //Enviamos el comando 'F' avisando de que se han acabado los animales a enviar
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
        case 4: //Estado de envío de productos por la SIO
            switch(word) {
                case 0:
                    //Enviamos la clave del comando 'DP'
                    if (SIO_TXAvail() >= 2) {
                        SIO_PutChar('D');
                        SIO_PutChar('P');
                        i = 0;
                        word++;
                    }
                    break;
                case 1: //Enviamos las cantidades de los productos caracter por caracter
                    if (SIO_TXAvail() >= 4) {
                        unsigned char num = ANIMALS_GetProd(map_prod[i]);
                        if(num >= 10){
                            SIO_PutChar((num / 10) + '0');
                        }
                        SIO_PutChar((num % 10) + '0');
                        if(i < 3){
                            SIO_PutChar('$');
                        }else{
                            //Enviamos la finalización del comando
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
        case 5: //Estado de busqueda del animal en la estructura de animales
            animals = ANIMALS_GetAnimals();
            unsigned char total = ANIMALS_GetTotalAnimals();
            
            if (search_idx < total) {
                //Miramos si se trata del animal actual
                if (animals[search_idx].type == target_type && animals[search_idx].number == target_num) {
                    TI_ResetTics(Timer);
                    state++;
                } else {
                    search_idx++;
                }
            } else {
                //Si el animal no existe, que no debería ocurrir, volvemos al estado incial
                state = 0;
            }
            break;
        case 6: //Estado de revisión del sueño del animal
            //Si el animal duerme antes de que pasen 5 segundos se le despierta
            //Miramos si el animal duerme mediante si el LDR detecta oscuridad o no
            if (JSK_getLight() == 1) {
                ANIMALS_Awake(search_idx);
                if(SIO_TXAvail() > 2){
                    SIO_PutChar('S');
                    SIO_PutChar('S');
                    state++;
                }
            }
            //Si pasan 5 segundos y el LDR no detecta oscuridad abortamos el sueño del animal
            else if (TI_GetTics(Timer) >= 5000 && JSK_getLight() == 0) {
                if(SIO_TXAvail() > 2){
                    SIO_PutChar('S');
                    SIO_PutChar('U');
                    state++;
                }
            }
            break;
        case 7: //Estado de envio de la finalización del comando
            if(SIO_TXAvail() > 2){
                SIO_PutChar('\r');
                SIO_PutChar('\n');
                EE_WriteAll();
                state = 0;
            }
            break;
    }
}