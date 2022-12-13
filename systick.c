#include <TM4C123GH6PM.h>


/*
Inicializa el SysTick para que cuente 1 segundo.
Suponemos una frecuencia de reloj de 16 MHz.
Parámetros:
	Ninguno
Retorno:
	Ninguno
 */
void inicSysTick1s (){
	SysTick->CTRL = 0x04;	// Detenemos el conteo
	SysTick->LOAD = 16000000 -1;	// Esperamos 1 s
	SysTick->VAL = 0;		// contador inicialmente en cero
	SysTick->CTRL = 0x05;	// Enable en uno, Clock source en uno y Int enable en cero
}

/*
Inicializa el SysTick para que cuente un valor dado (contador módulo n).
El tiempo de conteo depende de la frecuencia del reloj.
Parámetros:
	n -> valor del registro de recarga
Retorno:
	Ninguno
 */
void inicSysTickN (unsigned int n){
	SysTick->CTRL = 0x04;	// Detenemos el conteo
	SysTick->LOAD = n -1;	// Contador módulo n
	SysTick->VAL = 0;		// contador inicialmente en cero
	SysTick->CTRL = 0x05;	// Enable en uno, Clock source en uno y Int enable en cero
}

/*
Inicializa el SysTick para que cuente un valor dado (contador módulo n).
El tiempo de conteo depende de la frecuencia del reloj.
Esta función activa la interrupción hardware.
Parámetros:
	n -> valor del registro de recarga
Retorno:
	Ninguno
 */
void inicSysTickNInt (unsigned int n){
	SysTick->CTRL = 0x04;	// Detenemos el conteo
	SysTick->LOAD = n -1;	// Contador módulo n
	SysTick->VAL = 0;		// contador inicialmente en cero
	SysTick->CTRL = 0x07;	// Enable en uno, Clock source en uno y Int enable en uno
}


/*
Verifica que haya transcurrido el tiempo. Inicia el conteo.
Parámetros:
	Ninguno
Retorno:
	El valor de la bandera COUNT del Systick en el bit menos significativo
*/
int SysTickCero (){
	return (SysTick->CTRL & 0x010000) >> 16;
}
