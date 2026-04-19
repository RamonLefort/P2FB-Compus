#ifndef TAD_ADC_H
#define	TAD_ADC_H

void ADC_Init();
//Pre: -
//Post: Prepara el microcontrolador per a que adquireixi del canal quinCanal.

void ADC_PickChannel(char i);
//Pre: 0 <= i < 1
//Post: Posa el canal de lectura analógic al valor de i

void ADC_IniciaConversio(void);
//Pre: -
//Post: Dona l'ordre al microcontrolador per a que executi la conversi?.

char ADC_HiHaMostra(void);
//Pre: - 
//Post: Retorna 1 si s?ha acabat d?adquirir una mostra, 0 altrament

char ADC_GetMostra(void);
//Pre: ADC_HiHaMostra()
//Post: Retorna el valor de l??ltima mostra capturada


#endif	/* TAD_ADC_H */