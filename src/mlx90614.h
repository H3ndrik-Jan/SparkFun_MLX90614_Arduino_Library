#include <avr/io.h>
#include <stdio.h>
#include "i2c.h"

#define BAUD_100K	130000UL
#define TWI_BAUD(F_SYS, F_TWI)   ((F_SYS / (2 * F_TWI)) - 5)

#define MLX90614_I2CADDR 0x5A

// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x0E
#define MLX90614_REG_ADDR 0x2E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F
#define MLX90614_SLEEP 0xFF // Not really a register, but close enough

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


typedef enum {
	TEMP_RAW,
	TEMP_K,
	TEMP_C,
	TEMP_F
} temperature_units;

typedef struct mlx90614_struct
{
	TWI_t *pTwi;
	
	uint8_t _deviceAddress;
	uint8_t _defaultUnit;
	int16_t _rawObject;
	int16_t _rawAmbient;
	uint16_t id[4];
} mlx90614_t;


void setupi2c(uint32_t freq);
void mlx_begin(mlx90614_t *pMlx, TWI_t *pTwi, uint8_t address);
 
void setUnit(mlx90614_t *pMlx, temperature_units unit);
uint8_t readObject(mlx90614_t *pMlx);
uint8_t readAmbient(mlx90614_t *pMlx);
uint8_t I2CReadWord(mlx90614_t *pMlx, uint8_t reg, int16_t * dest);

uint32_t getIDH(mlx90614_t *pMlx);
uint32_t getIDL(mlx90614_t *pMlx);
uint8_t readID(mlx90614_t *pMlx);
uint8_t crc8(uint8_t inCrc, uint8_t inData);	//	Cyclic redundancy check for received data
float mlx_object(mlx90614_t *pMlx);
float mlx_ambient(mlx90614_t *pMlx);
float calcTemperature(mlx90614_t *pMlx, int16_t rawTemp);
uint8_t mlx_read(mlx90614_t *pMlx);

float readEmissivity(mlx90614_t *pMlx);
uint8_t setEmissivity(mlx90614_t *pMlx, float emis);

uint8_t writeEEPROM(mlx90614_t *pMlx, uint8_t reg, int16_t data);
uint8_t I2CWriteWord(mlx90614_t *pMlx, uint8_t reg, int16_t data);

uint8_t readAddress(mlx90614_t *pMlx);



//uint8_t mlx_sleep();
//uint8_t mlx_wake(uint32_t freq);
