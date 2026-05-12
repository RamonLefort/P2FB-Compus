#include <xc.h>
#include "TAD_TIMER.h"

#define START_BIT 0
#define IN PORTBbits.RB4
#define OUT LATBbits.LATB3

static unsigned char Timer, TimerClock;
static unsigned char segs, mins, hours, days = 1, months = 1;
const char MSG_OK[] = "\nDate and time correct\r\n";
const char MSG_ERR[] = "\nPlease input a correct date\r\n";

static char tx_busy = 0, index = 0;
char bytesRebuts, paritat, byteRebut;
char byteAEnviar, bitsEnviats, paritatTX;
static unsigned long timestamp = 0;
static char input[17];
static const char* current_msg = 0; 
static unsigned char msg_index = 0;

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
        return 1;
    }
    return 0;
}

void CLOCK_Motor(void){
    //Cada seggundo del timer revisa el tiempo
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

//Función para poner un mensaje nuevo en la cola
void MSG_SendString(const char* msg) {
    //Si ya hay un mensaje escribiendose no se pone
    if (current_msg == 0) {
        current_msg = msg;
        msg_index = 0;
    }
}

void MSG_Motor(void){
    //Si hay un mensaje guardado lo escribe, sino sigue
    if(current_msg != 0){
        char c = current_msg[msg_index];
        if (c != '\0') {
            if (TIME_SendChar(c) == 1) {
                msg_index++;
            }
        }else{
            current_msg = 0;
        }
    }
}

unsigned char TIME_HasElapsed(unsigned long start_time, unsigned char timeout_segs) {
    //Mira si ha pasado el tiempo o no
    if ((timestamp - start_time) >= timeout_segs) {
        return 1;
    }
    return 0;
}

//Función para comprobar el comando de la terminal
unsigned char TIME_ParseDateTime(void) {
    //Miramos si encontramos en el comando todo lo que sabemos que ha de estar
    if (input[2] != '/' || input[5] != ' ' || input[8] != ':' || input[11] != ':') {
        return 0;
    }
    //Convertimos los números de caracteres a números
    unsigned char d   = (input[0] - '0') * 10 + (input[1] - '0');
    unsigned char m   = (input[3] - '0') * 10 + (input[4] - '0');
    unsigned char h   = (input[6] - '0') * 10 + (input[7] - '0');
    unsigned char min = (input[9] - '0') * 10 + (input[10] - '0');
    unsigned char s   = (input[12] - '0') * 10 + (input[13] - '0');
    //Revisamos si los nñumeros están dentro de lo permitido
    if (d < 1 || d > 31 || m < 1 || m > 12 || h > 23 || min > 59 || s > 59) {
        return 0;
    }
    //Actualizamos las variables
    days = d; 
    months = m; 
    hours = h; 
    mins = min; 
    segs = s;
    unsigned int total_dias = (unsigned int)(m - 1) * 30 + (d - 1);
    // Matemática a 32 bits forzada (Sufijo UL) sino ponemos UL la multiplicación estará corrupta porque la ALU trabaja con 16 bits
    timestamp = ((unsigned long)total_dias * 86400UL) + ((unsigned long)h * 3600UL) + ((unsigned long)min * 60UL) + (unsigned long)s;
    return 1;
}

void TIME_Motor(void) {
    static char stateRX = 0;
    static char stateTX = 0;
    //Rutina de recepción de bytes
    if (!tx_busy) {
        switch(stateRX) {
            case 0: //Start bit
                if (IN == START_BIT) {
                    TI_ResetTics(Timer);
                    stateRX++;
                }
                break;
            case 1: //Nos ponemos en el centro del bit
                if (TI_GetTics(Timer) >= 1) {
                    TI_ResetTics(Timer);
                    stateRX++;
                }
                break;
            case 2: //Leemos el resto de bits hasta el stop bit
                if (TI_GetTics(Timer) >= 2) {
                    if (bytesRebuts < 8) {
                        byteRebut = ((byteRebut >> 1) & 0x7F) + (IN == 1 ? 0x80 : 0);
                        bytesRebuts++;
                        paritat ^= IN;
                        TI_ResetTics(Timer);
                    } else {
                        //Bit de paridad, podríamos compararlo
                        TI_ResetTics(Timer);
                        stateRX++;
                    }
                }
                break;
            case 3: //Stop bit
                if(TI_GetTics(Timer) >= 2){
                    TIME_SendChar(byteRebut);
                    if(index < 16){
                        input[index++] = byteRebut;
                    }
                    if(byteRebut == '\r'){
                        input[index] = '\0';
                        //Miramos el comando recibido y respondemos como toque
                        if (index == 15 && TIME_ParseDateTime()) {
                            MSG_SendString(MSG_OK);  
                        } else {
                            MSG_SendString(MSG_ERR); 
                        }
                        index = 0; 
                    }else if(index >= 15){
                        //Si el comando es incorrecto controlamos que no haya buffer overflow
                        index = 0;
                    }
                    bytesRebuts = paritat = byteRebut = 0;
                    stateRX = 0;
                }
                break;
        }
    }

    //Rutina de transmisión de bytes
    switch(stateTX) {
        case 0: //Start bit
            if (tx_busy) {
                OUT = 0;
                bitsEnviats = 0;
                paritatTX = 0;
                TI_ResetTics(Timer);
                stateTX++;
            }
            break;
        case 1: //Enviamos el byte
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
        case 3: //Stop bit
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