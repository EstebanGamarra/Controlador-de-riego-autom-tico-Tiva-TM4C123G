/*
Inicializa el SysTick para que cuente 1 segundo.
Inicia el conteo.
Par�metros:
	Ninguno
Retorno:
	Ninguno
 */
void inicSysTick1s (void);

/*
Inicializa el SysTick para que cuente un valor dado (contador m�dulo n).
El tiempo de conteo depende de la frecuencia del reloj.
Par�metros:
	n -> valor del registro de recarga
Retorno:
	Ninguno
 */
void inicSysTickN (unsigned int n);

/*
Inicializa el SysTick para que cuente un valor dado (contador m�dulo n).
El tiempo de conteo depende de la frecuencia del reloj.
Esta funci�n activa la interrupci�n hardware.
Par�metros:
	n -> valor del registro de recarga
Retorno:
	Ninguno
 */
void inicSysTickNInt (unsigned int n);

/*
Verifica que haya transcurrido el tiempo
Par�metros:
	Ninguno
Retorno:
	El valor de la bandera COUNT del Systick
*/
int SysTickCero (void);
