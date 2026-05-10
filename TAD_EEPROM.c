#include <xc.h>
#include "TAD_EEPROM.h"
#include "TAD_ANIMALS.h"
#include "TAD_SIOInt.h"

static char ee_state = 0;

// Variables de control de bloque
static unsigned char ee_addr = 0;
static unsigned char *byte_ptr;
static unsigned int bytes_left = 0;

// ==========================================
// Funciones Públicas de Disparo (Triggers)
// ==========================================

void EEMOTOR_TriggerReset(void) {
    if (ee_state == 0) ee_state = 1;
}

void EEMOTOR_TriggerWriteAll(void) {
    if (ee_state == 0) ee_state = 2;
}

void EEMOTOR_TriggerReadAll(void) {
    if (ee_state == 0) ee_state = 5;
}

// ==========================================
// El Motor Cooperativo (Llamar en el while(1))
// ==========================================

void EEMOTOR_Task(void) {
    static char first = 1;
    // REGLA DE ORO FÍSICA: Si el hardware de la EEPROM está 
    // ocupado quemando un byte, no hacemos nada y cedemos la CPU.
    if (EE_IsWriting()) return;

    switch(ee_state) {
        static char i = 0;
        case 0:
            break;

        // ----------------------------------
        // FUNCIONALIDAD 1: RESETEAR
        // ----------------------------------
        case 1:
            // Para resetear la granja, basta con decir que hay 0 animales.
            // Los datos viejos se quedarán en memoria como "basura", pero 
            // el sistema los ignorará porque total_animals = 0.
            if(i < 0 || i > 4){
                EE_Write(i, 1);
            }else{
                EE_Write(i, 0);
            }
            i++;
            if(i >= 255){
                ANIMALS_Init();
                i = 0;
                ee_state--;
            }
            break;

        // ----------------------------------
        // FUNCIONALIDAD 2: ESCRIBIR
        // ----------------------------------
        case 2:
            EE_Write(0x00, ANIMALS_GetTotalAnimals()); // Guardamos el total
            // Preparamos para volcar num_especie (4 bytes)
            byte_ptr = (unsigned char*)ANIMALS_GetNumEspecie(); 
            bytes_left = 4;
            ee_addr = 0x01;
            ee_state++; 
            break;

        case 3:
            if (bytes_left > 0) {
                EE_Write(ee_addr++, *byte_ptr++);
                bytes_left--;
            } else {
                // Al terminar num_especie, pasamos a awake_especie (dirección 0x05)
                if (ee_addr == 0x05) {
                    byte_ptr = (unsigned char*)ANIMALS_GetAwakeEspecie();
                    bytes_left = 4;
                    ee_state = 3;
                } else {
                    // Finalmente, el array de animales empieza en 0x09
                    byte_ptr = (unsigned char*)ANIMALS_GetAnimals();
                    bytes_left = ANIMALS_GetTotalAnimals() * sizeof(Animals);
                    ee_addr = 0x09;
                    ee_state++;
                }
            }
            break;

        case 4:
            if (bytes_left > 0) {
                EE_Write(ee_addr++, *byte_ptr++);
                bytes_left--;
            } else {
                ee_state = 0; // Terminado
            }
            break;

        // ----------------------------------
        // FUNCIONALIDAD 3: LEER
        // ----------------------------------
        case 5:
            // 1. Leemos el total de animales
            unsigned char num = EE_Read(0x00);
            if (num > 24){
                num = 0; // Seguridad
            }
            ANIMALS_setTotalAnimals(num);

            // 2. Preparamos para leer num_especie (4 bytes)
            byte_ptr = (unsigned char*)ANIMALS_GetNumEspecie(); 
            bytes_left = 4;
            ee_addr = 0x01;
            ee_state++;
            break;

        case 6:
            if (bytes_left > 0) {
                *byte_ptr++ = EE_Read(ee_addr++);
                bytes_left--;
            } else {
                // 3. Preparamos para leer awake_especie (4 bytes)
                byte_ptr = (unsigned char*)ANIMALS_GetAwakeEspecie();
                bytes_left = 4;
                ee_addr = 0x05;
                ee_state++;
            }
            break;

        case 7:
            if (bytes_left > 0) {
                *byte_ptr++ = EE_Read(ee_addr++);
                bytes_left--;
            } else {
                // 4. Finalmente, leemos el array de animales
                byte_ptr = (unsigned char*)ANIMALS_GetAnimals();
                bytes_left = ANIMALS_GetTotalAnimals() * sizeof(Animals);
                ee_addr = 0x09;
                ee_state++;
            }
            break;
            
        case 8:
            // Lectura cooperativa estricta: 1 byte por cada ciclo del procesador.
            // Garantiza un tiempo de ejecución O(1) de ~2 microsegundos por llamada,
            // eliminando el Jitter que podría desincronizar la SIO por software.
            if (bytes_left > 0) {
                *byte_ptr++ = EE_Read(ee_addr++); //Escribe en el array de animales directamente, ya que nos pasan el puntero
                bytes_left--;
            } else {
                // Solo cuando el contador llega a 0, liberamos la máquina de estados
                ee_state = 0; 
            }
            break;
    }
}

void EE_Write(char address, char data){
    EEADR = address;
    EEDATA = data;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    di();
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    ei();
}

//Retorna 1 si segueix escrivint
char EE_IsWriting(void){
    return EECON1bits.WR;
}

unsigned char EE_Read(char address){
    EEADR = address;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}