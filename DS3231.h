#include <stdint.h>

uint8_t Seg(void);
uint8_t Min(void);
uint8_t Hora(void);
uint8_t Dia(void);
void Seg_set(uint8_t var);
void Min_set(uint8_t var);
void Hora_set(uint8_t var);
void Dia_set(uint8_t var);
void RTC_Init(uint8_t seg, uint8_t min, uint8_t hora,uint8_t dia);
