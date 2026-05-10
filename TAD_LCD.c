// TODO: hay que cambiar las funciones que utilizan el timer, porque estan mal hechas.

#include <xc.h>
#include "TAD_TIMER.h"
#include "TAD_LCD.h"
#include "TAD_IFC.h"
#include "TAD_SIOTime.h"
#include "TAD_SIOInt.h"

//
//--------------------------------CONSTANTS---AREA-----------
//
#define FUNCTION_SET	0x20
#define BITS_8			0x10
#define DISPLAY_CONTROL	0x08
#define DISPLAY_ON		0x04
#define CURSOR_ON		0x02
#define DISPLAY_CLEAR	0x01
#define ENTRY_MODE		0x04
#define SET_DDRAM		0x80
//
//---------------------------End--CONSTANTS---AREA-----------
//


//
//--------------------------------VARIABLES---AREA-----------
//
static unsigned char Rows, Columns;
static unsigned char RowAct, ColumnAct;
static int Timer;
static unsigned char t_msg;
//
//---------------------------End--VARIABLES---AREA-----------
//

//
//--------------------------------PROTOTIPE--AREA-----------
//
void Espera(int Timer, int ms);
void CantaIR(char IR);
void CantaData(char Data);
void WaitForBusy(void);
void EscriuPrimeraOrdre(char);

//
//---------------------------End--PROTOTIYPE--AREA-----------
//


//
//--------------------------------PUBLIQUES---AREA-----------
//
void LcInit(char rows, char columns) {
// Pre: Rows = {1, 2, 4}  Columns = {8, 16, 20, 24, 32, 40 }
// Pre: It needs 40ms of tranquility between VCC raising until this constructor is called.
// Pre: There is a free timer
// Post: This routine can last until 100ms
// Post: The display remains cleared, the cursor is turned OFF and at the position (0, 0).
	int i;
	TI_NewTimer(&Timer);
    TI_NewTimer(&t_msg);
	Rows = rows; Columns = columns;
	RowAct = ColumnAct = 0;
	SetControlsSortida();
	for (i = 0; i < 2; i++) {
		Espera(Timer, 100);
		// This sequence is set by the manual.

		EscriuPrimeraOrdre(CURSOR_ON | DISPLAY_CLEAR);
		Espera(Timer, 5);
		EscriuPrimeraOrdre(CURSOR_ON | DISPLAY_CLEAR);
		Espera(Timer, 1);
		EscriuPrimeraOrdre(CURSOR_ON | DISPLAY_CLEAR);
		Espera(Timer, 1);
		// .. three times. 
		// Now one at 4 bits
		EscriuPrimeraOrdre(CURSOR_ON);
		Espera(Timer, 1);
		CantaIR(FUNCTION_SET | DISPLAY_CONTROL); 	// 4bits, 1 row, font 5x7
		// The first line is erased here 
		// Now we can wait for busy
		WaitForBusy(); 	CantaIR(DISPLAY_CONTROL);  	// Display Off
		WaitForBusy(); 	CantaIR(DISPLAY_CLEAR);	   	// All spaces
		Espera(Timer,3); // 1.64ms V1.1
		WaitForBusy(); 	CantaIR(DISPLAY_ON | CURSOR_ON); // Auto Increment and shift
		WaitForBusy(); 	CantaIR(DISPLAY_CONTROL | DISPLAY_ON | CURSOR_ON | DISPLAY_CLEAR); 		// Display On
	}
	//The manual says that it should work but it doesn't initialize 
    //correctly after 40ms. Therefore, there is a loop with two initializations 
    //from here the initialization works correctly if a reset is made or if
    //the supply is turned ON and OFF. 
}

void LcEnd(void) {
// The destructor
	TI_CloseTimer(Timer); // It is not needed anymore
}

void LcClear(void) {
// Post: Erases the display and sets the cursor to its previous state. 
// Post: The next order can last up to 1.6ms. 
	WaitForBusy(); 	CantaIR(DISPLAY_CLEAR);	   //Spaces
	Espera(Timer, 3); // V1.1
}

void LcCursorOn(void) {
// Post: Turn on the cursor
// Post: The next order can last up to 40us. 
	WaitForBusy();
	CantaIR(DISPLAY_CONTROL | DISPLAY_ON | CURSOR_ON);
}

void LcCursorOff(void) {
// Post: Turns off the cursor
// Post: The next order can last up to 40us. 
	WaitForBusy();
	CantaIR(DISPLAY_CONTROL | DISPLAY_ON);
}

void LcGotoXY(char Column, char Row) {
// Pre : Column between 0 and 39, row between 0 and 3. 
// Post: Sets the cursor to those coordinates. 
// Post: The next order can last until 40us.
	int Fisics;
	// calculating the effective address of the LCD ram. 
	switch (Rows) {
		case 2:
			Fisics = Column + (!Row ? 0 : 0x40); break;
		case 4:
			Fisics = Column;
			if (Row == 1) Fisics += 0x40; else
			if (Row == 2) Fisics += Columns;      /* 0x14; */ else
			if (Row == 3) Fisics += 0x40+Columns; /* 0x54; */
			break;
		case 1:
		default:
			Fisics = Column; break;
	}
	// applying the command
	WaitForBusy();
	CantaIR(SET_DDRAM | Fisics);
	// Finally, I refresh the local images.
	RowAct    = Row;
	ColumnAct = Column;
}

void LcPutChar(char c) {
// Post: Paints the char in the actual cursor position and increments 
// its position. If the column gets to 39 it returns to 0.
// The row of the LCD is increased when this happens until the second
// row and then it is reset back to row 0 if it has 2 rows total. 
// If the LCD has 4 rows it will reset back to row 0 when it
// reaches row 4 and the columns will go till 39 before reseting to 0.
// The one row LCDs returns to 0 when a column gets to 39. 
// The row is never increased. 
	// The char is written
	WaitForBusy(); CantaData(c);
	// The cursor position is recalculated.
	++ColumnAct;
	if (Rows == 3) {
		if (ColumnAct >= 20) {
			ColumnAct = 0;
			if (++RowAct >= 4) RowAct = 0;
			LcGotoXY(ColumnAct, RowAct);
		}
	} else
	if (Rows == 2) {
		if (ColumnAct >= 40) {
			ColumnAct = 0;
			if (++RowAct >= 2) RowAct = 0;
			LcGotoXY(ColumnAct, RowAct);
		}
	} else
	if (RowAct == 1) {
		if (ColumnAct >= 40) ColumnAct = 0;
		LcGotoXY(ColumnAct, RowAct);
	}
}


void LcPutString(char *s) {
// Post: Paints the string from the actual cursor position. 
// The coordinate criteria is the same as the LcPutChar. 
// Post: Can last up to 40us for each char of a routine output.
	while(*s) LcPutChar(*s++);
}

//
//---------------------------End--PUBLIC---AREA-----------
//

//
//--------------------------------PRIVATE----AREA-----------
//

void Espera(int Timer, int ms) {
	TI_ResetTics(Timer);
	while(TI_GetTics(Timer) < ms);
}

void CantaPartAlta(char c) {
	 SetD7(c & 0x80 ? 1 : 0);
	 SetD6(c & 0x40 ? 1 : 0);
	 SetD5(c & 0x20 ? 1 : 0);
	 SetD4(c & 0x10 ? 1 : 0);
}

void CantaPartBaixa(char c) {
	 SetD7(c & 0x08 ? 1 : 0);
	 SetD6(c & 0x04 ? 1 : 0);
	 SetD5(c & 0x02 ? 1 : 0);
	 SetD4(c & 0x01 ? 1 : 0);
}

void CantaIR(char IR) {
	SetD4_D7Sortida();
	RSDown();
	RWDown();
	EnableUp();
	CantaPartAlta(IR); 		// Data Setup = 80ns
	EnableUp();				// Making sure the pulse lasts 500ns
	EnableDown();   		// The pulse width "enable" is higher than 230ns
	EnableDown();
	EnableUp();
	CantaPartBaixa(IR); 	// Data Setup = 80ns
	EnableUp();				// Making sure the pulse lasts 500ns
	EnableDown();   		// The pulse width "enable" is higher than 230ns
	SetD4_D7Entrada();
}

void CantaData(char Data) {
	SetD4_D7Sortida();
	RSUp();
	RWDown();
	EnableUp();
	CantaPartAlta(Data); 	// Data Setup = 80ns
	EnableUp();				// Making sure the pulse lasts 500ns
	EnableDown();   		// The pulse width "enable" is higher than 230ns
	EnableDown();
	EnableUp();
	CantaPartBaixa(Data); 	// Data Setup = 80ns
	EnableUp();				// Making sure the pulse lasts 500ns
	EnableDown();   		// The pulse width "enable" is higher than 230ns
	SetD4_D7Entrada();
}

void WaitForBusy(void) { char Busy;
	SetD4_D7Entrada();
	RSDown();
	RWUp();
	TI_ResetTics(Timer);
	do {
		EnableUp();EnableUp(); //Making sure the 500ns of the pulse time
		Busy = GetBusyFlag();
		EnableDown();
		EnableDown();
		EnableUp();EnableUp();
		// The lower part of the address counter, it is not interesting for us. 
		EnableDown();
		EnableDown();
		if (TI_GetTics(Timer)) break; // More than one ms means that the LCD has gone mad.
	} while(Busy);
}

void EscriuPrimeraOrdre(char ordre) {
	// Write the first as if there are 8 bits.
	SetD4_D7Sortida();  RSDown(); RWDown();
	EnableUp(); EnableUp();
	 SetD7(ordre & 0x08 ? 1 : 0);
	 SetD6(ordre & 0x04 ? 1 : 0);
	 SetD5(ordre & 0x02 ? 1 : 0);
	 SetD4(ordre & 0x01 ? 1 : 0);
	EnableDown();
}

static MsgLCD cola[3];
static unsigned char n_msgs = 0;
static unsigned char state = 0;
const char* prot_nombres[] = {"Llet ", "Ous  ", "Pinzell ", "Carn "};
const char* anim_nombres[] = {"Vaca", "Gallina", "Cavall", "Porc"};

void LCD_PutInt(unsigned char val) {
    if (val >= 10) LcPutChar((val / 10) + '0');
    LcPutChar((val % 10) + '0');
}

void LCD_ResetLCD(){
    state = 0;
    n_msgs = 0;
    LcClear();
}

void LCD_PushMsg(unsigned char tipo, unsigned char id, unsigned char v1, unsigned char v2) {
    if (n_msgs >= 3) return;

    // Evitar duplicar el mensaje de bienvenida en la cola
    if (tipo == 0) {
        cola[n_msgs].tipo = tipo;
        n_msgs++;
        return;
    }

    cola[n_msgs].tipo = tipo;
    cola[n_msgs].id = id;
    cola[n_msgs].val1 = v1;
    cola[n_msgs].val2 = v2;
    n_msgs++;
}

void LCD_Motor(void) {
    static char* ptr = 0;
    static unsigned char char_idx = 0;

    if (n_msgs == 0){
        return;
    }

    switch(state) {
        case 0: // Enviar comando de borrado
            LcClear();
            TI_ResetTics(t_msg); // Reutilizamos el timer de los mensajes
            state = 1; 
            break;

        case 1: // ESPERA ASÍNCRONA: Esperar a que el hardware del LCD termine (2 ms)
            if (TI_GetTics(t_msg) >= 2) {
                char_idx = 0;
                if (cola[0].tipo == 0){
                    ptr = IFC_GetFarmName();
                }else if(cola[0].tipo == 1){
                    ptr = "Nou Producte \0";
                }else{
                    ptr = "Nou Animal \0";
                }
                state++;
            }
            break;

        case 2: // Imprimir prefijo o FarmName
            if (ptr && ptr[char_idx] != '\0') {
                LcPutChar(ptr[char_idx++]);
            } else {
                char_idx = 0;
                state++;
                
                //Baja a la segunda línea
                LcGotoXY(0, 1);
            }
            break;

        case 3: // Imprimir Datos Variables
            if (cola[0].tipo == 0) { // Fecha
                if(char_idx == 0){
                    LCD_PutInt(TIME_GetDay());
                }
                if(char_idx == 1){
                    LcPutChar('/');
                }
                if(char_idx == 2){
                    LCD_PutInt(TIME_GetMonth());
                }
                if(char_idx == 3){ 
                    ptr = "/2026"; state++; char_idx = 0; return;
                }
            } else { // Producto o Animal
                if (char_idx == 0) {
                    ptr = (cola[0].tipo == 1) ? prot_nombres[cola[0].id] : anim_nombres[cola[0].id];
                    state++;
                    char_idx = 0;
                    return;
                }
            }
            char_idx++;
            break;
            
        case 4: // Imprimir nombres de las tablas y sufijos
            if (ptr && ptr[char_idx] != '\0') {
                LcPutChar(ptr[char_idx++]);
            } else {
                if (cola[0].tipo != 0) { // Valor final para notificaciones
                    LcPutChar(':');
                    LcPutChar(' ');
                    LCD_PutInt(cola[0].val1);
                }
                state++;
            }
            break;

        case 5: 
            if (cola[0].tipo == 0) {
                LcCursorOff();
                // Es la pantalla base. Si llega algo a la cola (n_msgs > 1), la destruimos ahora mismo.
                if (n_msgs > 1) {
                    cola[0] = cola[1];
                    cola[1] = cola[2];
                    n_msgs--;
                    state = 0; // Ir directo a pintar la alerta
                }
            } else {
                // Es una alerta (tipo 1 o 2). Iniciamos la cuenta atrás de 3s.
                TI_ResetTics(t_msg);
                LcCursorOff();
                state++;
            }
            break;

        case 6: 
            // A este estado SÓLO se entra si estamos mostrando una alerta temporal
            if (TI_GetTics(t_msg) >= 3000){
                // Rotar cola (Shift Left)
                cola[0] = cola[1];
                cola[1] = cola[2];
                n_msgs--;
                
                // Si ya no quedan alertas, inyectamos la pantalla base por defecto
                if(n_msgs == 0){
                    cola[0].tipo = 0;
                    n_msgs++;
                }
                state = 0; // Volver a pintar
            }
            break;
    }
}

//
//---------------------------End--PRIVATE----AREA-----------
//