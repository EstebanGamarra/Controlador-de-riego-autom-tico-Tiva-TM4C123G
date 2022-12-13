/*
Inicializa el SysTick para que cuente 1 segundo.
Inicia el conteo.
Parámetros:
	Ninguno
Retorno:
	Ninguno
 */
void inicSysTick1s (void);

/*
Inicializa el SysTick para que cuente un valor dado (contador módulo n).
El tiempo de conteo depende de la frecuencia del reloj.
Parámetros:
	n -> valor del registro de recarga
Retorno:
	Ninguno
 */
void inicSysTickN (unsigned int n);

/*
Inicializa el SysTick para que cuente un valor dado (contador módulo n).
El tiempo de conteo depende de la frecuencia del reloj.
Esta función activa la interrupción hardware.
Parámetros:
	n -> valor del registro de recarga
Retorno:
	Ninguno
 */
void inicSysTickNInt (unsigned int n);

/*
Verifica que haya transcurrido el tiempo
Parámetros:
	Ninguno
Retorno:
	El valor de la bandera COUNT del Systick
*/
int SysTickCero (void);
