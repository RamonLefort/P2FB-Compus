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
    if (ee_state == 0) ee_state = 4;
}

// ==========================================
// El Motor Cooperativo (Llamar en el while(1))
// ==========================================

void EEMOTOR_Task(void) {
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
            EE_Write(i, 0);
            if(++i == 0) { // Al dar la vuelta de 255 a 0
                ANIMALS_Init();
                ee_state = 0;
            }
            break;

        // ----------------------------------
        // FUNCIONALIDAD 2: ESCRIBIR
        // ----------------------------------
        case 2:
            // 1. Guardamos el dato raíz
            EE_Write(0x00, ANIMALS_GetTotalAnimals());
            
            // 2. Preparamos puntero al array de animales
            byte_ptr = (unsigned char*)ANIMALS_GetAnimals();
            bytes_left = ANIMALS_GetTotalAnimals() * sizeof(Animals);
            ee_addr = 0x01; // El array empieza justo después del total
            ee_state = 3;
            break;

        case 3:
            if (bytes_left > 0) {
                EE_Write(ee_addr++, *byte_ptr++);
                bytes_left--;
            } else {
                ee_state = 0; // Guardado finalizado
            }
            break;

        // ----------------------------------
        // FUNCIONALIDAD 3: LEER
        // ----------------------------------
        case 4:
            // 1. Leemos el total
            unsigned char num = EE_Read(0x00);
            if (num > 24) num = 0; // Validamos integridad básica
            
            ANIMALS_setTotalAnimals(num);

            // 2. Preparamos para volcar el bloque de animales a RAM
            byte_ptr = (unsigned char*)ANIMALS_GetAnimals();
            bytes_left = num * sizeof(Animals);
            ee_addr = 0x01;
            ee_state = 5;
            break;

        case 5:
            if (bytes_left > 0) {
                *byte_ptr++ = EE_Read(ee_addr++);
                bytes_left--;
            } else {
                // AQUÍ: Disparamos la reconstrucción cooperativa en el TAD_ANIMALS
                ANIMALS_Rebuild(); 
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