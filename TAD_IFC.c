#include <xc.h>
#include "TAD_SIOInt.h"

#define MAX_COMANDO 32
#define MASK_RX 0x1F

char comando[MAX_COMANDO];
unsigned char Inicio, Fin, Quants;

void IFC_Init(){
    Inicio = Fin = Quants = 0;
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

void UCharToStr_Optimized(unsigned char val) {
    unsigned char hundreds = 0;
    unsigned char tens = 0;
    char buffer[4];

    // Contar centenas (¡Como val <= 255, este bucle se ejecuta máximo 2 veces!)
    while (val >= 100) {
        hundreds++;
        val -= 100;
    }

    // Contar decenas (Este bucle se ejecuta máximo 9 veces)
    while (val >= 10) {
        tens++;
        val -= 10;
    }

    // Las unidades son lo que sobra en 'val'

    // Formatear a ASCII evitando ceros a la izquierda (Zero-Suppression)
    if (hundreds > 0) {
        buffer[2] = hundreds + '0';
        buffer[1] = tens + '0';     // Si hay centenas, siempre imprimimos las decenas (ej: 105)
    } else if (tens > 0) {
        buffer[1] = tens + '0';
    }
    
    buffer[0] = val + '0'; // Las unidades siempre se imprimen
    buffer[3] = '\0';        // Cierre obligatorio de string
    SIO_PutString(buffer);
    SIO_PutString("\r\n");
}

void IFC_Motor(){
    static unsigned char state = 0;
    static unsigned char cmdtype = 0, idx, word = 0, i;
    static unsigned char bytes_procesados, total_payload;
    static char str[12];
    
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
                SIO_PutString("Cmd: Reset\n");
            } 
            else if (c1 == 'S') {
                switch(c2) {
                    case 'A': 
                        //SIO_PutString("Cmd: Sleep Animal\n");
                        cmdtype = 3;
                        idx = (Fin + 2) & MASK_RX;
                        total_payload = Quants - 2;
                        es_cooperativo = 1;
                        break;
                    case 'T': SIO_PutString("Cmd: Start Rebellion\n"); break;
                    case 'P': SIO_PutString("Cmd: Stop Rebellion\n"); break;
                }
            }
            else if (c1 == 'I') {
                //LSFarmSIO_PutString("Cmd: Inicializar\n");
                cmdtype = 1;
                idx = (Fin + 1) & MASK_RX;
                total_payload = Quants - 1;
                es_cooperativo = 1;
            }
            else if (c1 == 'P') {
                SIO_PutString("Cmd: Productos\n");
            }
            else if (c1 == 'C') { 
                SIO_PutString("Cmd: Consumir\n");
                cmdtype = 2;
                idx = (Fin + 1) & MASK_RX;
                total_payload = Quants - 1;
                es_cooperativo = 1;
            }
            
            if (es_cooperativo) {
                state = 2;
                word = 0;
                i = 0;
                bytes_procesados = 0;
            } else {
                Fin = (Fin + Quants) & MASK_RX;
                Quants = 0;
                state = 0;
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
                        if (c != '$') str[i++] = c;
                        str[i] = '\0';
                        SIO_PutString(str);
                        SIO_PutString("\r\n");
                        word++;
                        i = 0;
                    }else{
                        if(i < 11) str[i++] = c;
                    }
                    
                    if(fin_de_trama){
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        state = 0;
                    }
                    break;
                    
                case 2: // 'C' (NumOpción)
                    SIO_PutChar(c);
                    SIO_PutString("\r\n");
                    
                    Fin = (Fin + Quants) & MASK_RX;
                    Quants = 0;
                    state = 0;
                    break;
                    
                case 3: // 'SA' (Animal, Num, Estado)
                    if(c == '$' || fin_de_trama){
                        if (c != '$') str[i++] = c; 
                        str[i] = '\0';
                        SIO_PutString(str);
                        SIO_PutString("\r\n");
                        word++;
                        i = 0;
                    }else{
                        if(i < 11) str[i++] = c;
                    }
                    
                    if(fin_de_trama){
                        Fin = (Fin + Quants) & MASK_RX;
                        Quants = 0;
                        state = 0;
                    }
                    break;
            }
            break;
        }
    }
}