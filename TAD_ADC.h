#ifndef TAD_ADC_H
#define	TAD_ADC_H

void ADC_Init(char quinCanal);
//Pre: quinCanal està entre 0 i 12.
//Post: Prepara el microcontrolador per a que adquireixi del canal quinCanal.

void ADC_IniciaConversio(void);
//Pre: -
//Post: Dóna l?ordre al microcontrolador per a que executi la conversió.

char ADC_HiHaMostra(void);
//Pre: - 
//Post: Retorna 1 si s?ha acabat d?adquirir una mostra, 0 altrament

int ADC_GetMostra(void);
//Pre: ADC_HiHaMostra()
//Post: Retorna el valor de l?última mostra capturada


#endif	/* TAD_ADC_H */