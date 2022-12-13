#include <stdint.h>
#include <stdbool.h>
#include "I2C.h"

//direccion de registros del DS3231
#define direccion 0x68 //con 0 el bit de start
//toda la información del DS3231 retorna en formato BCD
#define seg_dir 0x00
#define min_dir 0x01
//bit 6 declara modo de 12 horas o 24 horas, 0 24 y 1 12
//bit 5 para AM/PM indica PM si está en 1
//bit 5 para 24 hs indica 20-23 hs
#define hora_dir 0x02 
//incrementa a la medianoche de cada día, declarado por el usuario y deben ser secuenciales
#define dia_dir 0x03
// para setear hora y dia usar buffers para evitar problemas
#define fecha_dir 0x04
#define control_dir 0xE0

//SET -> BINARIO A BCD para segundos y minutos
//READ -> BCD A BINARIO para segundos y minutos 

uint8_t BCD_a_DEC(int BCD){
	return(((BCD>>4)*10) + (BCD & 0xF));
}

uint8_t DEC_a_BCD(int DEC){
	return(((DEC/10)<<4) + (DEC%10));
}

uint8_t Seg(void){
	uint8_t val;
	I2C_Send1(direccion,seg_dir);
	val = I2C_Recv(direccion);
	val = BCD_a_DEC(val);
	return val;
}	
uint8_t Min(void){
	uint8_t val;
	I2C_Send1(direccion,min_dir);
	val = I2C_Recv(direccion);
	val = BCD_a_DEC(val);
	return val;
}	
uint8_t Hora(void){
	uint8_t val;
	I2C_Send1(direccion,hora_dir);
	val = I2C_Recv(direccion);
	val &= 0x3F; // para leer completo 20hs y 10hs
	if (val >= 0x20){ //20 hs activo
		val &= 0xF;
		val = val + 20;
 	}
	else if (val >= 0x10){ //10 hs activo
		val &= 0xF;
		val = val + 10;
	}
	else{ //otro caso, dejamos como está
		val &= 0xF;
	}
	return val;
}	
uint8_t Dia(void){
	uint8_t val;
	I2C_Send1(direccion,dia_dir);
	val = I2C_Recv(direccion);
	return val;
}
void Seg_set(uint8_t var){
	uint8_t val;
	val = DEC_a_BCD(var);
	I2C_Send2(direccion,seg_dir,val);
}
void Min_set(uint8_t var){
	uint8_t val;
	val = DEC_a_BCD(var);
	I2C_Send2(direccion,min_dir,val);
}
void Hora_set(uint8_t var){
	uint8_t val;
	if (var > 20){
		val = 0x20 + (var%10);
	}
	else if(var>10){
		val = 0x10 + (var%10);
	}
	else{
		val = var;
	}
	I2C_Send2(direccion,hora_dir,val);
}
void Dia_set(uint8_t var){
	uint8_t val;
	if (var > 7 || var <= 0){
		val = 1;
	}
	else{
		val = var;
	}	
	I2C_Send2(direccion,dia_dir,val);
}
void RTC_Init(uint8_t seg, uint8_t min, uint8_t hora, uint8_t dia){
	Seg_set(seg);
	Min_set(min);
	Hora_set(hora);
	Dia_set(dia);
}
