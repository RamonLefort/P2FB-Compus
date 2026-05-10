#include <xc.h>
#include "TAD_TIMER.h"

#define START_BIT 0
#define IN PORTBbits.RB4
#define OUT LATBbits.LATB3

// Tres timers independientes para evitar colisiones
static unsigned char Timer, TimerClock;
static unsigned char segs, mins, hours, days = 1, months = 1;
const char MSG_OK[] = "\nDate and time correct\r\n";
const char MSG_ERR[] = "\nPlease input a correct date\r\n";

// Variables subidas a nivel de archivo para ser compartidas con las funciones de envío
static char tx_busy = 0, index = 0;
char bytesRebuts, paritat, byteRebut;
char byteAEnviar, bitsEnviats, paritatTX;
static unsigned long timestamp = 0;
static char input[17];

void TIME_Init(void) {
    TRISBbits.TRISB3 = 0; 
    TRISBbits.TRISB4 = 1;
    //TRISD = 0x00;
    //LATD = 0x00;
    OUT = 1;
    index = 0;
    TI_NewTimer(&Timer);
    TI_NewTimer(&TimerClock);
}

// Getters 
unsigned char TIME_GetMonth()       { return months; }
unsigned char TIME_GetDay()         { return days; }
unsigned char TIME_GetHour()        { return hours; }
unsigned char TIME_GetMinute()      { return mins; }
unsigned char TIME_GetSecond()      { return segs; }
unsigned long TIME_GetTimestamp()   { return timestamp; }

//Función de envío de chars por terminal
unsigned char TIME_SendChar(char c) {
    if (!tx_busy) {
        byteAEnviar = c;
        tx_busy = 1;
        return 1; // Éxito: El hardware aceptó el byte
    }
    return 0; // Fallo: El hardware está ocupado
}

void CLOCK_Motor(void){
    if (TI_GetTics(TimerClock) >= 1000){
        TI_ResetTics(TimerClock);
        timestamp++;
        if (++segs > 59) {
            segs = 0;
            if (++mins > 59) {
                mins = 0;
                if (++hours > 23) {
                    hours = 0;
                    if (++days > 30) {
                        days = 1;
                        if (++months > 12){
                            months = 1;
                        }
                    }
                }
            }
        }
    }
}

// --- NUEVO MÓDULO O SECCIÓN DE MENSAJES ---

// Puntero a la ROM. El '0' (NULL) indica que el motor está libre.
static const char* current_msg = 0; 
static unsigned char msg_index = 0;

// Función para encolar un nuevo mensaje
void MSG_SendString(const char* msg) {
    // Protección: Solo iniciamos si el motor de mensajes está libre
    if (current_msg == 0) {
        current_msg = msg;
        msg_index = 0;
    }
}

// El Motor Cooperativo (Llamar desde el main en el bucle infinito)
void MSG_Motor(void){
    // 1. ¿Hay un mensaje pendiente de envío?
    if (current_msg != 0) {
        
        // 2. Leemos el carácter actual de la memoria Flash
        char c = current_msg[msg_index];
        
        // 3. ¿Hemos llegado al final del string?
        if (c != '\0') {
            // Intentamos inyectar el carácter en la capa física
            if (TIME_SendChar(c) == 1) {
                // SOLO si la capa física lo aceptó, avanzamos el índice.
                // Si devuelve 0, en el próximo ciclo del Motor lo reintentará con la misma letra.
                msg_index++;
            }
        } else {
            // Fin de la transmisión. Liberamos el motor superior.
            current_msg = 0;
        }
    }
}

unsigned char TIME_HasElapsed(unsigned long start_time, unsigned char timeout_segs) {
    if ((timestamp - start_time) >= timeout_segs) {
        return 1;
    }
    return 0;
}

// Función de validación y parseo (Devuelve 1 si es válido, 0 si hay error)
unsigned char TIME_ParseDateTime(void) {
    // 1. Verificación estructural rápida (Costo: 5 bytes de comprobación)
    // Buscamos los delimitadores en sus posiciones exactas
    if (input[2] != '/' || input[5] != ' ' || input[8] != ':' || input[11] != ':') {
        return 0; // Fallo de estructura, abortar al instante
    }

    // 2. Extracción directa con álgebra ASCII (Costo: Instrucciones aritméticas simples)
    unsigned char d   = (input[0] - '0') * 10 + (input[1] - '0');
    unsigned char m   = (input[3] - '0') * 10 + (input[4] - '0');
    unsigned char h   = (input[6] - '0') * 10 + (input[7] - '0');
    unsigned char min = (input[9] - '0') * 10 + (input[10] - '0');
    unsigned char s   = (input[12] - '0') * 10 + (input[13] - '0');

    // 3. Validación lógica de rangos
    if (d < 1 || d > 31 || m < 1 || m > 12 || h > 23 || min > 59 || s > 59) {
        return 0; // Datos fuera de rango lógico
    }

    // 4. Si todo es correcto, aplicamos los cambios
    // Bloqueo de interrupciones no es estrictamente necesario aquí porque estamos en la máquina de estados cooperativa,
    // pero garantizamos una actualización limpia.
    days = d; 
    months = m; 
    hours = h; 
    mins = min; 
    segs = s;
    
    // d: día (1-30), m: mes (1-12)
    unsigned int total_dias = (unsigned int)(m - 1) * 30 + (d - 1);
    
    // Matemática a 32 bits forzada (Sufijo UL) sino ponemos UL la multiplicación estará corrupta porque la ALU trabaja con 16 bits
    timestamp = ((unsigned long)total_dias * 86400UL) + ((unsigned long)h * 3600UL) + ((unsigned long)min * 60UL) + (unsigned long)s;
    
    return 1;
}

void TIME_Motor(void) {
    static char stateRX = 0;
    static char stateTX = 0;

    //Recepción
    if (!tx_busy) {
        switch(stateRX) {
            case 0:
                if (IN == START_BIT) {
                    TI_ResetTics(Timer);
                    stateRX++;
                }
                break;
            case 1:
                if (TI_GetTics(Timer) >= 1) {
                    TI_ResetTics(Timer);
                    stateRX++;
                }
                break;
            case 2:
                if (TI_GetTics(Timer) >= 2) {
                    if (bytesRebuts < 8) {
                        byteRebut = ((byteRebut >> 1) & 0x7F) + (IN == 1 ? 0x80 : 0);
                        bytesRebuts++;
                        paritat ^= IN;
                        TI_ResetTics(Timer);
                    } else {
                        //Bit de paridad, podríamos comparar
                        TI_ResetTics(Timer);
                        stateRX++;
                    }
                }
                break;
            case 3:
                if (TI_GetTics(Timer) >= 2) {
                    TIME_SendChar(byteRebut);
                    // 1. ALMACENAMIENTO SEGURO
                    // Solo guardamos si hay espacio físico en el array (evita Buffer Overflow)
                    if (index < 16) {
                        input[index++] = byteRebut;
                    }
                    // 2. TRIGGER DE EVALUACIÓN
                    if (byteRebut == '\r') {
                        // Ponemos el terminador nulo en la posición actual (seguro porque input es de 17)
                        // Si la trama es perfecta, index vale 16 (del 0 al 15 están los chars de la fecha)
                        input[index] = '\0'; 
                        
                        // 3. VALIDACIÓN (Sin paradojas lógicas)
                        if (index == 15 && TIME_ParseDateTime()) {
                            MSG_SendString(MSG_OK);  
                        } else {
                            MSG_SendString(MSG_ERR); 
                        }
                        // Vaciamos el buffer tras procesar una trama completa (correcta o no)
                        index = 0; 
                    } 
                    // 4. RESET POR BASURA EN LÍNEA
                    else if (index >= 15) {
                        // Si el buffer se llenó pero nunca llegó el '\n', es basura. Reiniciamos.
                        index = 0;
                    }
                    bytesRebuts = paritat = byteRebut = 0;
                    stateRX = 0;
                }
                break;
        }
    }

    //Transmisión
    switch(stateTX) {
        case 0:
            if (tx_busy) {
                OUT = 0;
                bitsEnviats = 0;
                paritatTX = 0;
                TI_ResetTics(Timer);
                stateTX++;
            }
            break;
        case 1:
            if (TI_GetTics(Timer) >= 2) {
                OUT = byteAEnviar & 0x01;
                paritatTX ^= OUT;
                byteAEnviar >>= 1;
                bitsEnviats++;
                TI_ResetTics(Timer);
                if (bitsEnviats >= 8){
                    stateTX++;
                }
            }
            break;
        case 2: //Bit de Paridad
            if (TI_GetTics(Timer) >= 2) {
                OUT = paritatTX;
                TI_ResetTics(Timer);
                stateTX++;
            }
            break;
        case 3: // STOP BIT
            if (TI_GetTics(Timer) >= 2) {
                OUT = 1; 
                TI_ResetTics(Timer);
                stateTX++;
            }
            break;
        case 4:
            if (TI_GetTics(Timer) >= 2) {
                tx_busy = 0;
                stateTX = 0;
            }
            break;
    }
}