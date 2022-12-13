#include <stdint.h>

uint32_t I2C_Send1(int8_t slave, uint8_t data1);
uint8_t I2C_Recv(int8_t slave);
uint32_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2);
void i2cInit(void);
