#ifndef SIOTIME_H
#define	SIOTIME_H

void TIME_Init();
//Pre: -
//Post: Inicializa la SIO Time

void TIME_Motor();
//Pre: -
//Post: Motor de la SIO Time, se encarga de realizar la recepción y la transmisión de bytes

char TIME_GetMonth();
//Pre: -
//Post: Devuelve el mes mes

char TIME_GetDay();
//Pre: -
//Post: Devuelve el dia actual

char TIME_GetHour();
//Pre: -
//Post: Devuelve la hora actual

char TIME_GetMinute();
//Pre: -
//Post: Devuelve el minuto actual

char TIME_GetSecond();
//Pre: -
//Post: Devuelve el segundo actual

unsigned long TIME_GetTimestamp();
//Pre: -
//Post: Devuelve el timestamp actual en segundos

unsigned char TIME_HasElapsed(unsigned long start_time, unsigned char timeout_segs);
//Pre: -
//Post: Devuelve 1 si el tiempo ha pasado o 0 si no ha pasado aún

void CLOCK_Motor();
//Pre: -
//Post: Motor del reloj, se encarga de llevar el contaje del tiempo del sistema

void MSG_Motor(void);
//Pre: -
//Post: Motor de los mensajes, se encarga de escribir un mensaje por la terminal

#endif	/* SIOTIME_H */
