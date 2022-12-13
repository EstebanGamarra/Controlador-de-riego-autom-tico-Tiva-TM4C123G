#include <TM4C123GH6PM.h>
#include <string.h>
//aclaraciones y tipos de variables equivalentes para la biblioteca:
//uint8_t -> unsigned char
//uint16_t -> unsigned short
//uint32_t -> unsigned int
//uint64_t -> unsigned long long
#include <stdint.h>
#include "systick.h" //interrupciones
#include "int.h" //interrupciones
#include "I2C.h"
#include "DS3231.h"
#include "EEPROM.h"
//codigos definidos del datasheet del Display
#define Limpiar_Display 0x01 //bien 1.52ms
#define Return_Home 0x02 //bien 1.52ms
#define	Shift_left_m 0x04  //08 //bien 37 us, muevo a mi siguiente memoria izquierda
#define Shift_right_m 0x06 //bien 37 us, muevo a mi siguiente memoria derecha
#define Shift_right_display 0x05 //bien 37 us S=1 y R/W = 1
#define	Shift_left_display 0x07 //bien 37 us S=1 y R/W = 1
#define	Cursor_apaga 0x0C //standby + display prendido
#define	Cursor_prende 0x0E // standby + display prendido
#define Cursor_parpadea 0x0F // standby + display prendido + cursor prendido
#define Cursor_l1 0x80 //fila 1 primer lugar
#define	Cursor_l2	0xC0 //fila 2 primer lugar
#define Cursor_shift_r 0x14 //muevo cursor sin tocar mem
#define Cursor shift_l 0x10 //muevo cursor sin tocar mem
#define Cursor_swipe_r 0x18 //muevo cursor+display
#define Cursor_swipe_l 0x1C //muevo cursor+display
#define	Display_font 0x30  //20// 8 bits, 0000 significa 5x8 font duty factor 1/6 
#define Modo_8_bits 0x38 // dos lineas + 8 bits
//KEYPAD
unsigned char teclado[4][4] = {{ '1', '2',  '3', 'A'},      
															{ '4', '5',  '6', 'B'},      
															{ '7', '8',  '9', 'C'},      
															{ '*', '0',  '#', 'D'}}; 
//flags principales que usamos para organizar cada uno de los ajustes de los medidores
int codigo_pantalla = 0, codigo_dia=100,i=100,j=100,contador_int=100, k=0 ;
//vector que indica que dias estan setados, 0 es automatico (predeterminado), 1 es seteado
uint32_t modo[2][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};
//uint32_t dias2[7] = {0,0,0,0,0,0,0}; 
//variables que uso para el control y lectura del ADC, inicializo en un número cualquiera
int sensor1 = 0, sensor2 = 0; // valores leidos del adc
char sensor1c[3] = "000", sensor2c[3] = "000"; //typecast char de sensor1 y sensor2 para poder escribir en display, predeterminado 30
//HORAS START
char semana[2][14][6];
/*{"000000","000000","000000","000000",
										"000000","000000","000000","000000",
										"000000","000000","000000","000000",
										"000000","000000","000000","000000",
										"000000","000000","000000","000000",
										"000000","000000","000000","000000",
										"000000","000000","000000","000000"}*/
//char lunes1[6] = "000000",martes1[6]= "000000",miercoles1[6]= "000000",jueves1[6]= "000000",viernes1[6]= "000000",sabado1[6]= "000000",domingo1[6]= "000000";
//char lunes2[6]= "000000",martes2[6]= "000000",miercoles2[6]= "000000",jueves2[6]= "000000",viernes2[6]= "000000",sabado2[6]= "000000",domingo2[6]= "000000";
//FIN
//char lunes12[6] = "000000",martes12[6]= "000000",miercoles12[6]= "000000",jueves12[6]= "000000",viernes12[6]= "000000",sabado12[6]= "000000",domingo12[6]= "081000";
//char lunes22[6]= "000000",martes22[6]= "000000",miercoles22[6]= "000000",jueves22[6]= "000000",viernes22[6]= "000000",sabado22[6]= "000000",domingo22[6]= "081000";	
//THRESHOLDS DE DÍAS 70 predeterminado
char set[4][14]; //{"0000000","0000000","0000000","0000000"};
/*char set_11[14] = "00000000000000"; //limite superior sensor 1
char set_12[14] = "00000000000000"; //limite inferior sensor 1
char set_21[14] = "00000000000000"; //limite superior sensor 2
char set_22[14] = "00000000000000"; //limite inferior sensor 2*/
//----
char time[6];//"000000"; // para mostrar la hora
//variables de uso general para escribir en el Display
char texto[16];
char swipe[40];
//--------
//variable donde cargo lo leído por mi keypad
char tecla;
//variables que uso la EEPROM
uint32_t puntero_EEPROM;
//-------------------- valores del RTC
int val1, val2, val3, val4, val5, val6; //VALORES establecidos de mis cadenas
int hora, minuto, segundo; //Valor leído del RTC
//inicio de subrutinas
uint8_t segundos; 
uint8_t minutos;
uint8_t horas;
uint8_t dias;
//--------
void ini_LEDS(void){
	volatile int k;
	SYSCTL -> RCGCGPIO |= 0x03; // pines PD0 y PD1
	k = 12;
	GPIOD ->LOCK  = 0x4C4F434B;	// desbloqueamos
  GPIOD->CR  |= 0x03;             //Allow settings for all pins of PORTE, E4-E5
	GPIOD->AMSEL  &= ~0x03; 			// quiero que cere esos valores
	GPIOD->AFSEL  &= ~0x03; 			// quiero que cere esos valores
	GPIOD->PCTL  &= ~0x03; 			// quiero que cere esos valores
  GPIOD->DIR |= 0x03;             //PD0-D1 son pines de salida digital
  GPIOD->DEN |= 0x03;             //Set PORTD0-1 as digital pins
	GPIOD->DATA |= 0x00;		//inicializo valor de pins
}

void ini_ADC(void){
	volatile int k;
	SYSCTL -> RCGCGPIO |= 0x08; //puerto D clock
	k=12;
	GPIOD -> LOCK  = 0x4C4F434B;	// desbloqueamos
	GPIOD -> DIR &= ~0x0C; // configuro como entrada
	GPIOD -> AFSEL |= 0x0C; //Configuración para función alternativa
	GPIOD -> DEN &= ~0x0C; // Deshabilitar digital enable para los pines
	GPIOD -> AMSEL |= 0x0C; //Puertos D2 y D3 para la entrada del ADC

	SYSCTL -> RCGC0 |= 0x00010000; // habilito el ADC 0
	k=k+12; // mini delay
	SYSCTL -> RCGC0 &= ~0x00000300; // habilito el ADC 0 para la menor cantidad de muestras posibles
	k=k+12; // mini delay
	//secuenciador 1
	ADC0 -> SSPRI = 0011;
	ADC0 -> ACTSS &= ~0x0002; //ceramos para evitar problemas
	ADC0 -> EMUX &= ~0xF000; // conv seq 3
	ADC0 -> SSMUX1 &= ~0x000F;//limpiamos (ceramos) la posición del SS1
	ADC0 -> SSMUX1 += 5; //AIN 5 = PD2 (pin number=64)
	ADC0 -> SSCTL1 = 0x0006; //enable interrupción y end of sequence, differential output select (0) 
	ADC0 -> ACTSS |= 0x0002; //activamos SS1
	k=k+12;
	//secuenciador 2
	ADC0 -> ACTSS &= ~0x0004; //ceramos para evitar problemas
	ADC0 -> EMUX &= ~0xF000; // conv seq 3
	ADC0 -> SSMUX2 &= ~0x000F; //limpiamos (ceramos) la posición del SS2
	ADC0 -> SSMUX2 += 4; //AIN 4 = PD3 (pin number=63)
	ADC0 -> SSCTL2 = 0x0006; //IDEM (como el anterior, no se usan |= ni &= porque este es un valor cte. predeterminado)
	ADC0 -> ACTSS |= 0x0004; //activamos  SS2
	k=k+12;
}

int read_ADC1(void){
	unsigned int YL_0;
	int conv;
	ADC0 -> PSSI = 0x02; //SS1 inicia secuencia
	while((ADC0->RIS&0x02)==0){}; // mientras se realice la operación de conversión, RIS -> pin activo 
	YL_0 = ADC0 -> SSFIFO1&0xFFF; //cargo en mi variable lo que se cargó en mi FIFO, resolución 12 bits
	ADC0 -> ISC = 0x0002; // ack interrupt
	conv= (YL_0*100)/4095; //valor del fifo hasta 4095, regla de 3 para sacar porcentaje
	return conv;
}

int read_ADC2(void){
	unsigned int YL_1;
	int conv;
	ADC0 -> PSSI = 0x04; //SS2 inicia secuencia
	while((ADC0->RIS&0x04)==0){}; // mientras se realice la operación de conversión, RIS -> pin activo 
	YL_1 = ADC0 -> SSFIFO2&0xFFF;  //cargo en mi variable lo que se cargó en mi FIFO, resolución 12 bits
	ADC0 -> ISC = 0x0004; // ack interrupt
	conv = (YL_1*100)/4095; //valor del fifo hasta 4095, regla de 3 para sacar porcentaje
	return conv;
}

void ini_teclado(void){
  volatile int k;
	SYSCTL->RCGCGPIO |= 0x14;        //Enable clock to PORTC and PORTE  
  k = 12;
	GPIOC->LOCK  = 0x4C4F434B;	// desbloqueamos, E ya está desbloqueado porque inicializamos LEDS primero
  GPIOC->CR  |= 0xF0;             //Allow settings for all pins of PORTC
  GPIOE->CR  |= 0x0F;             //Allow settings for all pins of PORTE
  GPIOE->DIR |= 0x00;             //PE1-PE4 are used with row and set them as digital input pins
	GPIOC->DIR |= 0xF0;             //PC4-PC7 are used with col and set them as digital output pins
  GPIOE->PDR |= 0x0F;             //Enable pull down resistor on PORTE
  GPIOC->DEN |= 0xF0;             //Set PORTC as digital pins
  GPIOE->DEN |= 0x0F;             //Set PORTE as digital pins
	GPIOC->DATA |= 0x00; 
}

void delay_ms(int n){
 int i,j;
 for(i=0;i<n;i++)
 for(j=0;j<3180;j++)
 {}
}

void delay_us(int n){
 int i,j;
 for(i=0;i<n;i++)
 for(j=0;j<3;j++)
 {}
}
char tecla_presionada(void){
	static int m, n, val1, val2;
	static int estado = 0;
	static char valor; //valor de retorno
	m=0;
	n=0;
  while(1)
  {
    for(m= 0; m < 4; m++)    //Scan columns loop
    {
      GPIOC->DATA = (0x01 << (m+4));
      delay_us(2);
      for(n = 0; n < 4; n++)  //Scan rows
      {
        if((GPIOE->DATA  & 0x0F )&(0x01 << n)){
          valor = teclado [n][m];
					val1 = (GPIOE->DATA  & 0x0F)&(0x01 << n);
					switch (estado) { //antirrebote
					case 0: //EN_CERO
						if (val1){
							val2 = val1;
							estado = 1;
						}
						else{
							estado = 0;
						}
						break;
					case 1: //SUBIDA
						if (val1==val2) {
							estado = 2;
						} else {
							estado = 0;
						}
						break;
					case 2: //FLANCO
							estado = 0;
							return valor;
						}
					}
				}	
			}
	}
}
void Escribir_Codigo(char codigo){
	codigo &= 0xFF;
	GPIOA -> DATA = 0x00; //cero todo
	GPIOA -> DATA = 0x20; //habilito el ENABLE para los comandos
	// se envian los datos
	delay_us(0);
	GPIOB -> DATA = codigo; //pongo en el bus los datos que voy a enviar
	GPIOA -> DATA = 0x00; // reinicio mi control
}

void Escribir_Display(char dato){
	dato &= 0xFF;
	GPIOA -> DATA = 0x80; // habilito RS A7
	GPIOB -> DATA = dato; //cargo dato en el bus
	// no le gusta aplicar directamente encima el valor 0xA0, 
	// |= para que funcione (dos operaciones)
	GPIOA -> DATA = 0x20 | 0x80; // aplico el ENABLE A5 y A7
	delay_us(0);
	GPIOA -> DATA = 0x00; //reinicio mi control
	delay_us (40); //espero 37 us, WRITE
	delay_ms(1);
}

void Cadena_Display(char *str){
	int i;
	for (i = 0; str[i]!=0;i++){
			Escribir_Display(str[i]);
			delay_us(40);
	}
}

void Comando_Display(char codigo){
		Escribir_Codigo(codigo & 0xFF); //paso el dato
		if (codigo < 4) { //si son comandos de clear o return homre
			delay_ms (2); //comandos esperamos 1.57 ms
		}
		else{
			delay_us(40); //otros comandos solo 37 us
		}
}

void Ini_Display(void){
	volatile int k;
	SYSCTL  -> RCGCGPIO |= 0x03; //habilito B(datos) y A (control)
	k = 12;
	GPIOB -> DIR |= 0xFF; //todos los pines de B son salidas
	GPIOB -> DEN |= 0xFF; //todos los pines de B son digitales 
	GPIOA -> DIR |= 0xE0; //pined A5-A7 son salidas
	GPIOA -> DEN |= 0xE0; //pines de A digitales
	//inicializo el diplay por medio de comandos
	Comando_Display(Display_font); //inicializar el modo de display
	delay_ms(2); //
	Comando_Display(Display_font); //en caso de haber fallado vuelvo a intentarlo
	delay_ms(2) ;
	// luego se pone en modo de 8 bits, donde queda implicito tambien Display font 
	// misma operación  
	Comando_Display(Modo_8_bits);
	Comando_Display(Limpiar_Display); 
	Comando_Display(Cursor_parpadea);
	Comando_Display(Shift_right_m);
}
void trans_EEPROM(){
	i = 16;
	j = 0; //usamos para referirnos al START Y FIN o MAX y MIN
	k = 0; //usamos para referirnos a los índices 0-6 y 0-1
	for (i=16;i<240;i++){
		//SENSOR 1 LUNES
		if (i >= 16 && i <= 27){
			if(i == 22){
				j = 1;
				k = 0;
			}
			else if (i == 16){
				j = 0;
				k = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
	//MARTES
		else if (i >= 28 && i <= 39){
			if(i == 34){
				k = 0;
				j= 3;
			}
			else if ( i == 28){
				k = 0;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//MIERCOLES
		else if (i >= 40 && i <= 51){
			if(i == 46){
				k = 0;
				j = 5;
			}
			else if ( i == 40){
				k = 0;
				j = 4;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//JUEVES
		else if (i >= 52 && i <= 63){
			if(i == 58){
				k = 1;
				j = 7;
			}
			else if ( i == 52){
				k = 0;
				j = 6;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//VIERNES
		else if (i >= 64 && i <= 75){
			if(i == 70){
				k = 0;
				j = 9;
			}
			else if ( i == 64){
				k = 0;
				j = 8;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//SABADO
		else if (i >= 76 && i <= 87){
			if(i == 82){
				k = 0;
				j = 11;
			}
			else if ( i == 76){
				k = 0;
				j = 10;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//DOMINGO
		else if (i >= 88 && i <= 93){
			if(i == 94){
				k = 0;
				j = 13;
			}
			else if ( i == 88){
				k = 0;
				j = 12;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[0][j][k]=puntero_EEPROM;
			k = k+1;
			}
//----------SENSOR 2 DIAS-------------------
			//LUNES		
			else if (i >= 100 && i <= 111){
			if(i == 106){
				k = 0;
				j = 1;
			}
			else if (i == 100){
				k = 0;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
	//MARTES
		else if (i >= 112 && i <= 123){
			if(i == 118){
				k = 1;
				j= 3;
			}
			else if ( i == 112){
				k = 0;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//MIERCOLES
		else if (i >= 124 && i <= 135){
			if(i == 130){
				k = 0;
				j = 5;
			}
			else if ( i == 124){
				k = 0;
				j = 4;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//JUEVES
		else if (i >= 136 && i <= 147){
			if(i == 142){
				k = 1;
				j = 7;
			}
			else if ( i == 136){
				k = 0;
				j = 6;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//VIERNES
		else if (i >= 148 && i <= 159){
			if(i == 154){
				k = 0;
				j = 9;
			}
			else if ( i == 148){
				k = 0;
				j = 8;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//SABADO
		else if (i >= 160 && i <= 171){
			if(i == 166){
				k = 0;
				j = 11;
			}
			else if ( i == 160){
				k = 0;
				j = 10;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
		//DOMINGO
		else if (i >= 172 && i <= 183){
			if(i == 178){
				k = 0;
				j = 13;
			}
			else if ( i == 172){
				k = 0;
				j = 12;
			}
			r_EEPROM(i,&puntero_EEPROM);
			semana[1][j][k]=puntero_EEPROM;
			k = k+1;
			}
//--------SENSOR 1 SET-------------
		//LUNES
		else if (i >= 184 && i <= 187){
			if ( i == 186){
				k = 0;
				j = 1;
			}
			else if (i == 184){
				k = 0;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//MARTES
		else if (i >= 188 && i <= 191){
			if ( i == 190){
				k = 2;
				j = 1;
			}
			else if (i == 188){
				k = 2;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//MIERCOLES
		else if (i >= 192 && i <= 195){
			if ( i == 194){
				k = 4;
				j = 1;
			}
			else if (i == 192){
				k = 4;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] = puntero_EEPROM;
			k = k+1;
		}
		//JUEVES
		else if (i >= 196 && i <= 199){
			if ( i == 198){
				k = 6;
				j = 1;
			}
			else if (i == 196){
				k = 6;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//VIERNES
		else if (i >= 200 && i <= 203){
			if ( i == 202){
				k = 8;
				j = 1;
			}
			else if (i == 200){
				k = 8;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//SABADO
		else if (i >= 204 && i <= 207){
			if ( i == 206){
				k = 10;
				j = 1;
			}
			else if (i == 204){
				k = 10;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//DOMINGO
		else if (i >= 208 && i <= 211){
			if ( i == 210){
				k = 12;
				j = 1;
			}
			else if (i == 208){
				k = 12;
				j = 0;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
//--------SENSOR 2 SET---------------
		//LUNES
		else if (i >= 212 && i <= 215){
			if ( i == 214){
				k = 0;
				j = 3;
			}
			else if (i == 212){
				k = 0;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//MARTES
		else if (i >= 216 && i <= 219){
			if ( i == 218){
				k = 2;
				j = 3;
			}
			else if (i == 216){
				k = 2;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//MIERCOLES
		else if (i >= 220 && i <= 223){
			if ( i == 222){
				k = 4;
				j = 3;
			}
			else if (i == 220){
				k = 4;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] = puntero_EEPROM;
			k = k+1;
		}
		//JUEVES
		else if (i >= 224 && i <= 227){
			if ( i == 226){
				k = 6;
				j = 3;
			}
			else if (i == 224){
				k = 6;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//VIERNES
		else if (i >= 228 && i <= 231){
			if ( i == 228){
				k = 8;
				j = 3;
			}
			else if (i == 231){
				k = 8;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//SABADO
		else if (i >= 232 && i <= 235){
			if ( i == 234){
				k = 10;
				j = 3;
			}
			else if (i == 232){
				k = 10;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
		//DOMINGO
		else if (i >= 236 && i <= 239){
			if ( i == 238){
				k = 12;
				j = 3;
			}
			else if (i == 236){
				k = 12;
				j = 2;
			}
			r_EEPROM(i,&puntero_EEPROM);
			set[j][k] =puntero_EEPROM;
			k = k+1;
		}
	}// for end
}

void SysTick_Handler (void) { //codigo systick (SWIPE Y WRITE GPIOS LEDS (VALVULAS))
	static int var1; //esto es para comparar valores 
	static int var2;
	static int var3;
	static int var4;
	if (codigo_pantalla == 0 || codigo_pantalla == 31 ||
			codigo_pantalla == 21 || codigo_pantalla == 41){ //swipe para las pantallas con mucho texto
		Comando_Display(Cursor_swipe_r);
	}
	segundos=Seg();
	minutos=Min();
	horas=Hora();
	dias=Dia();
	var3 = read_ADC1();
	var4 = read_ADC2();
	if (modo[0][dias-1] == 1){ //MODO DIA SENSOR 1
		var1 = (semana[0][dias-1][0]-48)*10 + semana[0][dias-1][1]-48; //inicio
		var2 = (semana[0][dias-1][0]-48)*10 + semana[0][dias-1][1]-48; //fin
		if (horas >= var1 && horas <= var2){ // se confirmó la hora
			var1 = (semana[0][dias-1][2]-48)*10 + (semana[0][dias-1][3]-48); //inicio
			var2 = (semana[0][dias-1][2]-48)*10 + (semana[0][dias-1][3]-48); //fin
			if (minutos >= var1 && minutos <= var2){ //se confirmó el minuto
					GPIOD -> DATA |= 0x01; //prendo mi LED;
				}
			else {
				GPIOD -> DATA &= ~0x01; //apago mi LED;
				}
		}
		else{
				GPIOD -> DATA &= ~0x01; //apago mi LED;
		}
	}
	if (modo[1][dias-1] == 1){ //MODO DIA SENSOR 2
		var1 = (semana[1][dias-1][0]-48)*10 + semana[1][dias-1][1]-48; //inicio
		var2 = (semana[1][dias-1][0]-48)*10 + semana[1][dias-1][1]-48; //fin
		if (horas >= var1 && horas <= var2){ // se confirmó la hora
			var1 = (semana[1][dias-1][2]-48)*10 + (semana[1][dias-1][3]-48); //inicio
			var2 = (semana[1][dias-1][2]-48)*10 + (semana[1][dias-1][3]-48); //fin
			if (minutos >= var1 && minutos <= var2){ //se confirmó el minuto
					GPIOD -> DATA |= 0x02; //prendo mi LED;
				}
			else{
					GPIOD -> DATA &= ~0x02; //apago mi LED;
			}
		}
		else{
				GPIOD -> DATA &= 0x02; //apago mi LED;
		}
	}
	if (modo[0][dias-1] == 0){ //MODO AUTOMATICO SENSOR 1
		var1 = (set[0][dias-1]-48)*10 + set[0][dias-1]-48;
		var2 = (set[1][dias-1]-48)*10 + set[1][dias-1]-48;
		if (var3 >= var1 && var3 <= var2){ //para que riege, prendo mi sensor
			GPIOD -> DATA |= 0x01; //prendo mi LED;
		}
		else{
			GPIOD -> DATA &= ~0x01; //apago mi LED;
		}
	}
	if (modo[1][dias-1] == 0){ //MODO AUTOMATICO SENSOR 2
		var1 = (set[2][dias-1]-48)*10 + set[2][dias-1]-48;
		var2 = (set[3][dias-1]-48)*10 + set[3][dias-1]-48;
		if (var4 >= var1 && var4 <= var2){ //para que riege, prendo mi sensor
			GPIOD -> DATA |= 0x02; //prendo mi LED;
		}
		else{
			GPIOD -> DATA &= ~0x02; //apago mi LED;
		}
	}
}
int main(){	
	uint8_t dia_var; //para el RTC
	uint8_t time_var; //para el RTC
	uint32_t dire_var; //para la EEPROM
	//--------------------------
	ini_ADC();
	ini_LEDS();
	ini_teclado();	
	Ini_Display();
	set_EEPROM();
	trans_EEPROM();
	i2cInit();
	//RTC_Init(0,35,15,6); una sola vez, es free running el RTC
	inicSysTickNInt(16000000); // systick cada 1 segundo, = 16 MHz
	EnableInterrupts();
	while(1){
	switch (codigo_pantalla){
		case 0: //HAY SWIPE
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-Funciones:");
			Cadena_Display(texto);
			strcpy(swipe,"-1LEER;2SETEAR DIA;3AUTOMATICO;4STATUS");
			Comando_Display(Cursor_l2);
			Cadena_Display(swipe);
			Comando_Display(Cursor_apaga);
			tecla = 'A'; //inicializacion siempre en un valor que no hace nada, para que no se vuelva loco el display
			tecla=tecla_presionada();
			switch (tecla){
					case '1':
						codigo_pantalla = 10; // LEER
					break;
					case '2':
						codigo_pantalla = 20; // SET
						break;
					case '3':
						codigo_pantalla = 30; // AUTOMATICO
						break;
					case '4':
						codigo_pantalla = 40; //LEER HORA, STATUS
						break;
					default:
						codigo_pantalla = 0; //VUELVO A PANTALLA DE INICIO
						break;
					}
			delay_ms(500);
			break;
			case 10: //PANTALLA LEER
				sensor1 = read_ADC1(); 
				if (sensor1 == 0x64){
					sensor1c[0] = '1';
					sensor1c[1] = '0';
					sensor1c[2] = '0';
				}
				else{
					sensor1c[0] = '0';
					sensor1c[1] = (sensor1/10) + '0'; //pasamos de int a char para poder luego escribir en display
					sensor1c[2] = (sensor1%10) + '0';
				}
				sensor2 = read_ADC2();
				if (sensor2 == 0x64){
					sensor2c[0] = '1';
					sensor2c[1] = '0';
					sensor2c[2] = '0';
				}
				else{
					sensor2c[0] = '0';
					sensor2c[1] = (sensor2/10) + '0'; //pasamos de int a char para poder luego escribir en display
					sensor2c[2] = (sensor2%10) + '0';
				}
				Comando_Display(Limpiar_Display);
				delay_ms(2);
				Comando_Display(Cursor_l1);
				strcpy(texto,"-1:");
				Cadena_Display(texto);
				//Cadena_Display(sensor1c); no funciona
				Escribir_Display(sensor1c[0]); //valor del sensor 1
				Escribir_Display(sensor1c[1]); //valor del sensor 1
				Escribir_Display(sensor1c[2]); //valor del sensor 1
				Escribir_Display('%');
				strcpy(texto,"  #B");
				Cadena_Display(texto); //# para ver la hora y dia
				Comando_Display(Cursor_l2);
				delay_ms(2);
				strcpy(texto,"-2:"); // % y 2:
				Cadena_Display(texto);
				Escribir_Display(sensor2c[0]); //valor del sensor 2
				Escribir_Display(sensor2c[1]);
				Escribir_Display(sensor2c[2]);
				Escribir_Display('%'); // %
				strcpy(texto,"  *H/D");
				Cadena_Display(texto); //# para ver la hora y dia
				Comando_Display(Cursor_apaga);
				tecla = 'A';
				tecla = tecla_presionada();
				if (tecla == '*'){
						codigo_pantalla = 11; // leer dia y hora
				}
				else if (tecla=='#'){
						codigo_pantalla = 0;  //vuelve al inicio
				}
				else{
					codigo_pantalla = 10; //no hago nada
				}
		delay_ms(500); //delay despues de cada switch sino se vuelve loca esta cosa
		break;
		case 11:
			contador_int=0;
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-Dia:");
			Cadena_Display(texto);
			dia_var = Dia(); //Leo día de mi rtc
			//dias va desde 1 viernes hasta jueves 7
			if (dia_var == 1){
				delay_us(50);
				strcpy(texto,"Lunes");
				Cadena_Display(texto);
			}
			else if (dia_var==2){
				delay_us(50);
				strcpy(texto,"Martes");
				Cadena_Display(texto);
			}
			else if (dia_var==3){
				delay_us(50);
				strcpy(texto,"Miercoles");
				Cadena_Display(texto);
			}
			else if (dia_var==4){
				delay_us(50);
				strcpy(texto,"Jueves");
				Cadena_Display(texto);
			}
			else if (dia_var==5){
				delay_us(50);
				strcpy(texto,"Viernes");
				Cadena_Display(texto);
			}
			else if (dia_var==6){
				delay_us(50);
				strcpy(texto,"Sabado");
				Cadena_Display(texto);
			}
			else if (dia_var==7){
				delay_us(50);
				strcpy(texto,"Domingo");
				Cadena_Display(texto);
			}
			delay_ms(2);
			Comando_Display(Cursor_l2);
			strcpy(texto,"-Hora:");
			Cadena_Display(texto);
			for (contador_int=0;contador_int<6; contador_int++){
				if (contador_int==0 || contador_int==1) { //las horas
					time_var = Hora();
					time[0] = (time_var/10) + '0'; //pasamos de int a char para poder luego escribir en display
					time[1] = (time_var%10) + '0';
				} //horas end
				else if (contador_int==2 || contador_int==3){ // minutos
					time_var = Min();
					time[2] = (time_var/10) + '0'; //pasamos de int a char para poder luego escribir en display
					time[3] = (time_var%10) + '0';
				} //else minutos end
				else if (contador_int==4 || contador_int==5){
					time_var = Seg();
					time[4] = (time_var/10) + '0'; //pasamos de int a char para poder luego escribir en display
					time[5] = (time_var%10) + '0';
				} 
			}// for end
			delay_ms(2);
			Escribir_Display(time[0]);
			delay_ms(2);
			Escribir_Display(time[1]);
			delay_ms(2);
			Escribir_Display(':');
			delay_ms(2);
			Escribir_Display(time[2]);
			delay_ms(2);
			Escribir_Display(time[3]);
			delay_ms(2);
			Escribir_Display(':');
			delay_ms(2);
			Escribir_Display(time[4]);
			delay_ms(2);
			Escribir_Display(time[5]);
			contador_int=0;
			tecla = 'A';
			tecla = tecla_presionada();
			if (tecla == '*'){
				codigo_pantalla = 10; // leer dia 
			}
			else if (tecla=='#'){
				codigo_pantalla = 0;  //vuelve al inicio
			}
			else{
				codigo_pantalla = 10; //no hago nada
			}
			delay_ms(500);
		break;
		case 20: //PANTALA SET DIA, ELEGIMOS EL SENSOR
				Comando_Display(Limpiar_Display);
				delay_ms(2);
				Comando_Display(Cursor_l1);
				strcpy(texto,"-MEDIDOR: #B");
				Cadena_Display(texto);
				strcpy(texto,"-1:S1-2:S2 ");
				Comando_Display(Cursor_l2);
				Cadena_Display(texto);
				Comando_Display(Cursor_apaga);
				tecla = 'A';
				tecla = tecla_presionada();
				if (tecla == '#'){
						codigo_pantalla = 0;
				}
				else if (tecla == '1'){
					codigo_pantalla = 21; //setear día
						j = 1;
				}
				else if (tecla == '2'){
						codigo_pantalla = 21; //setear día
						j=2;
					}
		delay_ms(500);
		break;
		case 21: //PANTALLA DE SET DIA, HAY SWIPE
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-SET DIA: #B");
			Cadena_Display(texto);
			strcpy(swipe,"-1:Lu-2:Ma-3:Mi-4:Ju-5:Vi-6:Sa-7:Do ");
			Comando_Display(Cursor_l2);
			Cadena_Display(swipe);
			Comando_Display(Cursor_apaga);
			tecla = 'A';
			tecla = tecla_presionada();
			switch (tecla){
				case '#':
					codigo_pantalla = 0;
					break;
				case '1':
					codigo_pantalla = 22; //setear día lunes
					codigo_dia = 0;
					break;
				case '2':
					codigo_pantalla = 22; //setear día martes
					codigo_dia = 1;
					break;
				case '3':
					codigo_pantalla = 22; //setear día miercoles
					codigo_dia = 2;
					break;
				case '4':
					codigo_pantalla = 22; //setear día jueves
					codigo_dia = 3;
					break;
				case '5':
					codigo_pantalla = 22; //setear día viernes
					codigo_dia = 4;
					break;
				case '6':
					codigo_pantalla = 22; //setear día sabado
					codigo_dia = 5;
					break;
				case '7':
					codigo_pantalla = 22; //setear día domingo
					codigo_dia = 6;
					break;
				}
			modo[j-1][codigo_dia] = 1; 
			//ESCRIBO EN MI EEPROM
			//CORROBORADO
			if (j == 1){
				dire_var = codigo_dia;
			}
			else{
				dire_var = codigo_dia + 7;
			}
			w_EEPROM(dire_var,1); // 1 es para días
		delay_ms(500);
		break;
		case 22: //PANTALLA DE HORA START
			contador_int=0;
			i=0;
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-HORA:");
			Cadena_Display(texto);
			Comando_Display(Cursor_parpadea);
			if (j == 1){
				dire_var = 16 + 12*codigo_dia;
			}
			else if (j == 2){
				dire_var = 100 + 12*codigo_dia;
			}
			for (contador_int=0;contador_int<8; contador_int++){ // requerimos 6 inputs de teclado, contamos ello, pero también contamos los dos puntos HH:MM:SS(ultimo S, valor 7)
					if ((contador_int==2)||(contador_int==5)){
						Escribir_Display(':');
					}
					else{
						tecla = 'A';
						tecla = tecla_presionada();
						delay_ms(500);
						if (tecla == '#'){
							codigo_pantalla = 0; // volvemos
						}
						else{
						Escribir_Display(tecla);
						semana[j-1][codigo_dia*2][i] = tecla; //CORROBORADO
						w_EEPROM(dire_var+i,tecla);
						i=i+1;
						}
					}
				} //salimos del for de HORA
				codigo_pantalla = 23; 
		delay_ms(500);
		break;
		case 23: // PANTALA DE FIN HORA
			contador_int=0;
			i=0;
			delay_ms(2);
			Comando_Display(Cursor_l2);
			strcpy(texto,"-FIN:");
			Cadena_Display(texto);
			if (j == 1){
				dire_var = 16 + 12*codigo_dia + 6;
			}
			else if (j == 2){
				dire_var = 100 + 12*codigo_dia + 6;
			}
			for(contador_int=0;contador_int<8;contador_int++){ // requerimos 6 inputs de teclado, contamos ello, pero también contamos los dos puntos HH:MM:SS(ultimo S, valor 7)
				if ((contador_int==2)||(contador_int==5)){ // si estamos en la posición 2 o 5 ponemos :, porque HH(:)MM(:)SS
					Escribir_Display(':');
				}
				else{
					tecla = 'A';
					tecla = tecla_presionada();
					delay_ms(500);
					if (tecla == '#'){
						codigo_pantalla=0;
					}
					else{
					Escribir_Display(tecla);
					semana[j-1][codigo_dia*2+1][i] = tecla; //CORROBORADO
					w_EEPROM(dire_var+i,tecla);
					i=i+1;	
					}
				}
			}
			codigo_dia = 100; //no elegimos ningún día
			codigo_pantalla = 0; //voy al comienzo
		delay_ms(500);
		break;
		case 30: //PANTALLA AUTO
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-SEL SENSOR: #B");
			Cadena_Display(texto);
			Comando_Display(Cursor_l2);
			strcpy(texto,"-1:SEN1-2:SEN2");
			Cadena_Display(texto);
			Comando_Display(Cursor_apaga);
			tecla = 'A';
			tecla = tecla_presionada();
				switch (tecla){
					case '#':
						codigo_pantalla = 0;
						break;
					case '1':
						codigo_pantalla = 31; //setear automatico el primer sensor
						j=1;
						break;
					case '2':
						codigo_pantalla = 31; //setear automatico el segundo sensor
						j=2;
						break;
				}
		delay_ms(500);
		break;
		case 31: 	//SET DIA AUTO, HAY SWIPE
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-SET DIA: #B");
			Cadena_Display(texto);
			strcpy(swipe,"-1:Lu-2:Ma-3:Mi-4:Ju-5:Vi-6:Sa-7:Do ");
			Comando_Display(Cursor_l2);
			Cadena_Display(swipe);
			Comando_Display(Cursor_apaga);
			tecla = 'A';
			tecla = tecla_presionada();
			switch (tecla){
				case '#':
					codigo_pantalla = 0;
				break;
				case '1':
					codigo_pantalla = 32;
					codigo_dia = 0;
				break;
				case '2':
					codigo_pantalla = 32;
					codigo_dia = 1;
				break;
				case '3':
					codigo_pantalla = 32;
					codigo_dia = 2;
				break;
				case '4':
					codigo_pantalla = 32;
					codigo_dia = 3;
				break;
				case '5':
					codigo_pantalla = 32;
					codigo_dia = 4;
				break;
				case '6':
					codigo_pantalla = 32;
					codigo_dia = 5;
				break;
				case '7':					
					codigo_pantalla = 32;
					codigo_dia = 6;
				break;
			}
			//CORROBORADO
			modo[j-1][codigo_dia] = 0;
			if ( j==1){
				dire_var = codigo_dia;
			}
			else{
				dire_var = codigo_dia+7;
			}
			w_EEPROM(dire_var, 0); // 0 es para automatico
		delay_ms(500);
		break;
		case 32:  //AUTO MAX
			if (j==1){
				dire_var=184 + codigo_dia*4;
			}
			else{
				dire_var=212 + codigo_dia*4;
			}
			contador_int=0;
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-LIMITE %: #B");
			Cadena_Display(texto);
			delay_ms(2);
			Comando_Display(Cursor_l2);
			strcpy(texto,"-UP:");
			Cadena_Display(texto);
			Comando_Display(Cursor_parpadea);
			for (contador_int=0;contador_int<2;contador_int++){ //setear porcentaje, cada dia tiene su porcentaje
				tecla = 'A';
				tecla = tecla_presionada();
				delay_ms(500);
				if (tecla < 58 && 47 < tecla){
						Escribir_Display(tecla);
							set[(j-1)*2][(codigo_dia*2)+contador_int] = tecla; //CORROBORADO
							w_EEPROM(dire_var+contador_int,tecla);
				}
				else if (tecla == '#'){
					codigo_pantalla = 0; //fuerzo a salir
					contador_int = 2;
				}
			}
			codigo_pantalla = 33;
		delay_ms(500);
		break;
		case 33: //AUTO MIN
			if (j==1){
				dire_var=184 + codigo_dia*4 + 2;
			}
			else{
				dire_var=212 + codigo_dia*4 + 2;
			}
			contador_int=0;
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-LIMITE %: #B");
			Cadena_Display(texto);
			delay_ms(2);
			Comando_Display(Cursor_l2);
			strcpy(texto," -DWN:");
			Cadena_Display(texto);
			Comando_Display(Cursor_parpadea);
			for (contador_int=0;contador_int<2;contador_int++){ //CORROBORADO
				tecla = 'A';
				tecla = tecla_presionada();
				delay_ms(500);
				if (tecla < 58 && 47 < tecla){
					Escribir_Display(tecla);
					set[((j-1)*2)+1][(codigo_dia*2)+contador_int] = tecla; // CORROBORADO
					w_EEPROM(dire_var+contador_int,tecla);
				}
				else if (tecla == '#'){
					codigo_pantalla = 0; //fuerzo a salir
					contador_int = 2;
				}
			}
		j=0;
		codigo_dia = 100;
		codigo_pantalla = 0;
		break;
		case 40:	//pantalla de status
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_apaga);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-Sensor: -#B");
			Cadena_Display(texto);
			Comando_Display(Cursor_l2);
			strcpy(texto,"-1:SEN1-2:SEN2");
			Cadena_Display(texto);
			tecla = 'A';
			tecla = tecla_presionada();
			switch (tecla){
				case '#':
					codigo_pantalla = 0;
					break;
				case '1':
					codigo_pantalla = 41;
					j = 1;
					break;
				case '2':
					codigo_pantalla = 41;
					j =2;
					break;
				}
		delay_ms(500);
		break;
		case 41:	//HAY SWIPE LEER, ESCOJO MI DIA PARA LEER
			Comando_Display(Limpiar_Display);
			delay_ms(2);
			Comando_Display(Cursor_l1);
			strcpy(texto,"-SELEC DIA: #B");
			Cadena_Display(texto);
			strcpy(swipe,"-1:Lu-2:Ma-3:Mi-4:Ju-5:Vi-6:Sa-7:Do ");
			Comando_Display(Cursor_l2);
			Cadena_Display(swipe);
			Comando_Display(Cursor_apaga);
			tecla = 'A';
			tecla = tecla_presionada();
			switch (tecla){
				case '#':
					codigo_pantalla = 0;
					break;
				case '1':
					codigo_pantalla = 42;
					codigo_dia = 0; 
					break;
				case '2':
					codigo_pantalla = 42;
					codigo_dia = 1;
					break;
				case '3':
					codigo_pantalla = 42;
					codigo_dia = 2;
					break;
				case '4':
					codigo_pantalla = 42;
					codigo_dia = 3;
					break;
				case '5':
					codigo_pantalla = 42;
					codigo_dia = 4;
					break;
				case '6':
					codigo_pantalla = 42;
					codigo_dia = 5;
					break;
				case '7':
					codigo_pantalla = 42;
					codigo_dia = 6;
					break;
				}
		delay_ms(500);
		break;
		case 42: // LEER YA SEA DIA U THRESHOLD DEL DIA SELECCIONADO
		i=0;
		contador_int=0;
		if (modo[j-1][codigo_dia] == 0){ // si está en automatico
					Comando_Display(Limpiar_Display);
					delay_ms(2);
					Comando_Display(Cursor_apaga);
					Comando_Display(Cursor_l1);
					strcpy(texto,"-UP:");
					Cadena_Display(texto);
					i = 2*codigo_dia;
					for (i=(2*codigo_dia);i<(2*codigo_dia)+2;i++){
						Escribir_Display(set[(j-1)*2][i]); //CORROBORADO
						delay_ms(2);
					}
					i=2*codigo_dia;
					Escribir_Display('%');
					Comando_Display(Cursor_l2);
					delay_ms(2);
					strcpy(texto,"-DOWN:");
					Cadena_Display(texto);
					for (i=(2*codigo_dia);i<(2*codigo_dia)+2;i++){
						Escribir_Display(set[((j-1)*2)+1][i]); //CORROBORADO
						delay_ms(2);
					}
					Escribir_Display('%');
				}
		else if (modo[j-1][codigo_dia] == 1){ // si está en dias
					Comando_Display(Limpiar_Display);
					delay_ms(2);
					Comando_Display(Cursor_l1);
					strcpy(texto,"-START:");
					Cadena_Display(texto);
					Comando_Display(Cursor_apaga);
					for(contador_int=0;contador_int<8;contador_int++){	
						delay_ms(2);
						if (contador_int == 2 || contador_int == 5){
							Escribir_Display(':');
						}
						else{
							Escribir_Display(semana[j-1][codigo_dia*2][i]);  //CORROBORADO
							i=i+1;
						}
					}
					i=0;
					contador_int=0;
					delay_ms(2);
					Comando_Display(Cursor_l2);
					strcpy(texto,"-FIN:");
					Cadena_Display(texto);
					for(contador_int=0;contador_int<8;contador_int++){
						delay_ms(2);
						if (contador_int == 2 || contador_int==5){
							Escribir_Display(':');
						}
						else{
							Escribir_Display(semana[j-1][(codigo_dia*2)+1][i]); //corroborado
							i=i+1;
						}
					}
				}
				while (!tecla_presionada());
				codigo_pantalla = 0; //vuelvo al inicio
			break;
		} //switch grande end
	}	 //while 1 end
} //main end
