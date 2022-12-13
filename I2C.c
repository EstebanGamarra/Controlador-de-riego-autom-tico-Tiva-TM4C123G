#include <TM4C123GH6PM.h>
#include <stdint.h>
#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up


void i2cInit(){
	volatile unsigned delay; //VALVANO, yo le suelo llamar k
	SYSCTL->RCGCI2C |= 0x04; // Activate I2C2
	SYSCTL->RCGCGPIO |= 0x10; //Activate GPIOE
	delay=12; // delay
	
	GPIOE->AFSEL |= 0x30;//GPIOE4-GPIOE5
	
	//Registro de control de drenaje abierto
	GPIOE->ODR |= 0x20;//Set SDA Open Drain
	GPIOE->DEN |= 0x30;//Digital inputs for SDA-SCL
	GPIOE->PCTL |= 0x330000;//PCTL multiplexer selector, 3 for each port used
	//El registro PCTL selecciona la senal periferica especifica para cada pin GPIO
	GPIOE->AMSEL&= ~0x30;//No AMSEL
	I2C2->MCR=0x00000010;//I2C MASTER Init
	I2C2->MTPR=0x07;//
}

uint8_t I2C_Recv(int8_t esclavo){
  int retryCounter = 1;
  do{
    while((I2C2->MCS) & I2C_MCS_BUSY){};    // wait for I2C ready
    I2C2->MSA = (esclavo<<1)&0xFE;          // MSA[7:1] is slave address
    I2C2->MSA |= 0x01;                      // MSA[0] is 1 for receive
    I2C2->MCS = (0
                        // & ~I2C_MCS_ACK   // negative data ack (last byte)
                         | I2C_MCS_STOP     // generate stop
                         | I2C_MCS_START    // generate start/restart
                         | I2C_MCS_RUN);    // master enable
			
    while((I2C2->MCS)&I2C_MCS_BUSY){};        // wait for transmission done
    retryCounter = retryCounter + 1;        // increment retry counter
  }                                         // repeat if error
  while((((I2C2->MCS)&(I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0) && (retryCounter <= MAXRETRIES));
  return ((I2C2->MDR)&0xFF);                  // usually returns 0xFF on error
}

uint32_t I2C_Send1(int8_t esclavo, uint8_t data1){
  while((I2C2->MCS)&I2C_MCS_BUSY){};         // wait for I2C ready
  I2C2->MSA = (esclavo<<1)&0xFE;           // MSA[7:1] is slave address
  I2C2->MSA &= ~0x01;                      // MSA[0] is 0 for send
  I2C2->MDR = data1&0xFF;                  // Se escribe el dato en el registro I2CMDR
  I2C2->MCS = (0													//Se lee el registro I2CMCS
                    //   & ~I2C_MCS_ACK   // no data ack (no data on send)
                       | I2C_MCS_STOP     // generate stop
                       | I2C_MCS_START    // generate start/restart
                       | I2C_MCS_RUN);    // master enable
  while((I2C2->MCS)&I2C_MCS_BUSY){};// Se espera que se trasmita bien
                                          // return error bits
  return ((I2C2->MCS)&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}
uint32_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2){
  while((I2C2->MCS)&I2C_MCS_BUSY){};// wait for I2C ready
  I2C2->MSA = (slave<<1)&0xFE;    // MSA[7:1] is slave address
  I2C2->MSA &= ~0x01;             // MSA[0] is 0 for send
  I2C2->MDR = data1&0xFF;         // prepare first byte
  I2C2->MCS = (0
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                    //   & ~I2C_MCS_STOP    // no stop
                       | I2C_MCS_START    // generate start/restart
                       | I2C_MCS_RUN);    // master enable
  while((I2C2->MCS)&I2C_MCS_BUSY){};// wait for transmission done
                                          // check error bits
  if(((I2C2->MCS)&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C2->MCS = (0                // send stop if nonzero
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // stop
                     //  & ~I2C_MCS_START   // no start/restart
                     //  & ~I2C_MCS_RUN    // master disable
                        );   
                                          // return error bits if nonzero
    return ((I2C2->MCS)&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C2->MDR = data2&0xFF;         // prepare second byte
  I2C2->MCS = (0
                      // & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // generate stop
                      // & ~I2C_MCS_START   // no start/restart
                       | I2C_MCS_RUN);    // master enable
  while((I2C2->MCS)&I2C_MCS_BUSY){};// wait for transmission done
                                          // return error bits
  return ((I2C2->MCS)&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}
