#include <TM4C123GH6PM.h>
#include <stdint.h>
/*
Para saber como se estructuran los datos:
Tenemos 512 posiciones posibles de memoria que almacenan datos de 32 bits
(7 posiciones * 2 = 14 posiciones de memoria) -> 0 a 13
//vector que indica que dias estan setados, 0 es automatico (predeterminado), 1 es seteado
int dias1[7] = {1,0,0,0,0,0,0};
int dias2[7] = {0,0,0,0,0,0,0}; 

 //para testear, hay que agarrar del adc, sensor1 y sensor2 son valores de prueba 
 --------VALORES VOLÁTILES; NO USAMOS EEPROM PARA ESTOS
int sensor1 = 30, sensor2 = 30; // valores leidos del adc
char sensor1c[3] = "030", sensor2c[3] = "030"; //typecast char de sensor1 y sensor2 para poder escribir en display, predeterminado 30
---------------------------------------------------------

//HORAS, 8 de la mañana predeterminado
6 datos por vector, 2 sensores, 7 dias de la semana, inicio y fin (2), total de espacio = 6*2*7*2 = 168 espacios de memoria
char lunes1[6] = "080000",martes1[6]= "080000",miercoles1[6]= "080000",jueves1[6]= "080000",viernes1[6]= "080000",sabado1[6]= "080000",domingo1[6]= "080000";
char lunes2[6]= "080000",martes2[6]= "080000",miercoles2[6]= "080000",jueves2[6]= "080000",viernes2[6]= "080000",sabado2[6]= "080000",domingo2[6]= "080000";
//FIN, 10 minutos luego de las 8 de la mañana, horario de comienzo
char lunes12[6] = "081000",martes12[6]= "081000",miercoles12[6]= "081000",jueves12[6]= "081000",viernes12[6]= "081000",sabado12[6]= "081000",domingo12[6]= "081000";
char lunes22[6]= "081000",martes22[6]= "081000",miercoles22[6]= "081000",jueves22[6]= "081000",viernes22[6]= "081000",sabado22[6]= "081000",domingo22[6]= "081000";	
-------------------------------------------------------------------------------------------------------------------------

//THRESHOLDS DE DÍAS 70 predeterminado
14 datos y 4 vectores = 56 espacios de memoria
char set_11[14] = "70707070707070"; //limite inferior
char set_12[14] = "50505050505050"; //limite superior
char set_21[14] = "70707070707070"; //limite superior
char set_22[14] = "50505050505050"; //limite inferior
----------------- VARIABLES LOCALES PARA LEER DIAS HORA INICIO Y HORA FIN; O SET INICIO Y SET FIN
char lectura1_EEPROM[6];
char lectura2_EEPROM[6];
----------------------------------------------------------------------------------------------------------------
ASI DELIMITAMOS:
DIRECIONES DE 0-12: 
--ESTADO, AUTOMÁTICO O MANUAL-- esto es dias[codigo_dia] y j para elegir el sensor
0-6: sensor 1 MODO
7-12: sensor 2 MODO
-------------------
DIRECIONES DE 
--SENSOR 1-- // usamos codigo_dia para controlar, j nuevamente par el sensor
16-21: START lunes
22-27: END lunes
28-33: START martes
34-39: END martes
40-45: START miercoles
46-51: END miercoles
52-57: START jueves
58-63: END jueves
64-69: START viernes
70-75: END viernes
76-81: START sabado
82-87: END sabado
88-93: START domingo
94-99: END domingo
--SENSOR 2--
100-105: START lunes
106-111: END lunes
112-117: START martes
118-123: END martes
124-129: START miercoles
130-135: END miercoles
136-141: START jueves
142-147: END jueves
148-153: START viernes
154-159: END viernes
160-165: START sabado
166-171: END sabado
172-177: START domingo
178-183: END domingo
----------------
DIRECCIONES DE 
--SENSOR 1--
184-185: SET MAX lunes
186-187: SET MIN lunes
188-189: SET MAX martes
190-191: SET MIN martes
192-193: SET MAX miercoles
194-195: SET MIN miercoles
196-197: SET MAX jueves
198-199: SET MIN jueves
200-201: SET MAX viernes
202-203: SET MIN viernes
204-205: SET MAX Sabado
206-207: SET MIN Sabado
208-209: SET MAX domingo
210-211: SET MIN domingo
--SENSOR 2--
212-213: SET MAX lunes
214-215: SET MIN lunes
216-217: SET MAX martes
218-219: SET MIN martes
220-221: SET MAX miercoles
222-223: SET MIN miercoles
224-225: SET MAX jueves
226-227: SET MIN jueves
228-229: SET MAX viernes
230-231: SET MIN viernes
232-233: SET MAX Sabado
234-235: SET MIN Sabado
236-237: SET MAX domingo
238-239: SET MIN domingo
*/

int set_EEPROM(void){
	volatile int i;
	SYSCTL->RCGCEEPROM |= 0x01;	// Ini EEPROM Clock	
	i = 10; //primer delay, para que se ponga en 1 el clock
	while (i--){};	// delay de 6 ciclos + call overhead
	while ((EEPROM->EEDONE & 0x01)){};	// loop hasta que EEDONE haya terminado, una vez que working = 0 salgo
	if ((EEPROM->EESUPP & 0x0C) != 0){//pregunto por algún error leyendo PRETRY y ERETRY
		SYSCTL->SREEPROM |= 0x01;	//EEPROM reset
		i = 10;
		while (i--){};	// delay de 6 ciclos
		SYSCTL->SREEPROM &= ~0x01;	// seteamos la EEPROM
		SYSCTL->RCGCEEPROM |= 0x01;	// Ponemos clock a la EEPROM
		i = i+1;
		i = 10; 
		while (i--){};	// delay de 6 ciclos + call overhead
		while ((EEPROM->EEDONE & 0x01)){};	// termino EEDONE, salgo del while
		if ((EEPROM->EESUPP & 0x0C) != 0){ // vuelvo a preguntar si hubo un error
			return -1;	// ERROR 
		} else {
			return 0;		//Se realizo todo con éxito
		}
	}
	return 0;	// Se realizó todo el proceso de forma satisfactoria
}
int32_t w_EEPROM (uint16_t dir, uint32_t val){
	volatile int i;
	if ((dir >> 4) > 31){ //esto es porque, el valor máximo de dirección es 512
	//recordando que la EEPROM es 2kbytes de memorias con 512 direcciones que almacenan palabras de 4 bytes
		return -1;	// error para acceder
	}
	else{
		EEPROM->EEBLOCK = dir >> 4;	// operación de write
		i = 10;
		while (i--);	// delay
		EEPROM->EEOFFSET = dir & 0x0F;	// escribimos el offset (valor que se modifica y usa en conjunto al usar EEBLOCK)
		EEPROM->EERDWR = val;		// se escribe el valor
		while ((EEPROM->EEDONE & 0x01)){};	// se espera a que EEDONE 1 deje de trabajar
		return EEPROM->EEDONE; // 0x??0, realizó la operación deseada, EEDONE se limpia cuando termina satisfactoriamente
	}
}
int32_t r_EEPROM (uint16_t dir, uint32_t *val){
	volatile int i;
	if ((dir >> 4) > 31){
		return -1;	// error en el bloque
	}
	else{
	EEPROM->EEBLOCK = dir >> 4;	// operación de write
	i = 10;
	while (i--);	// delay
	EEPROM->EEOFFSET = dir & 0x0F;	// se escribe el offset (valor que se modifica y usa en conjunto al usar EEBLOCK)
	*val = EEPROM->EERDWR;		// se lee el valor
	return EEPROM->EEDONE; //0x??0, realizó la operación deseada, EEDONE se limpia cuando termina satisfactoriamente
	}
}
