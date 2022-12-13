#include <stdint.h>

int set_EEPROM(void);
int32_t w_EEPROM(uint16_t dir, uint32_t val);
int32_t r_EEPROM(uint16_t dir, uint32_t *val);
