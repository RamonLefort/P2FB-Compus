#ifndef TAD_IFC_H
#define	TAD_IFC_H

void IFC_Init();
//Pre: -
//Post: Inicializa la interficie

void IFC_Motor();
//Pre: -
//Post: Motor de la interficie, se encarga de capturar y parsear los comandos y de realizar las acciones necesarias con ellos

char* IFC_GetFarmName();
//Pre: -
//Post: Devuelve el nombre de la granja

#endif	/* TAD_IFC_H */
