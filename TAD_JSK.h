#ifndef TAD_JSK_H
#define	TAD_JSK_H

void JSK_Init();
//Pre: -
//Post: Inicializa el joystick y el light

void JSK_Motor();
//Pre: -
//Post: Motor del joystick y del light, va realizando conversiones de ADC

char JSK_getLight();
//Pre: -
//Post: Devuelve el estado del sensor light, 0 si esta en estado normal o 1 si esta oscuro

#endif	/* TAD_JSK_H */

