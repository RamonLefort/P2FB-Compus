#ifndef SIOTIME_H
#define	SIOTIME_H

void TIME_Init();

void TIME_Motor();

char TIME_GetMonth();

char TIME_GetDay();

char TIME_GetHour();

char TIME_GetMinute();

char TIME_GetSecond();

unsigned long TIME_GetTimestamp();

unsigned char TIME_HasElapsed(unsigned long start_time, unsigned char timeout_segs);

void CLOCK_Motor();

void MSG_Motor(void);

#endif	/* SIOTIME_H */