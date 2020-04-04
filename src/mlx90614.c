
#define  F_CPU   32000000UL

#include "mlx90614.h"

#include <util/delay.h>



/* int16_t _rawObject = 0;
int16_t _rawAmbient = 0;

uint8_t _defaultUnit = TEMP_C;
uint8_t _deviceAddress;

uint16_t id[4]; */
#define SDA PIN0_bm;
#define SCL PIN1_bm;


void setupi2c(uint32_t freq){
	
	i2c_init(&TWIE, TWI_BAUD(freq, BAUD_100K));
	PORTE.DIRSET = PIN1_bm|PIN0_bm;
	PORTE.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc; //SDA
	PORTE.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc; //SCL
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
}

/*  uint8_t mlx_begin(mlx90614_t *pMlx)
{
	//pMlx->_deviceAddress = address; // Store the address in a private member
	pMlx->_defaultUnit = TEMP_C;
	//! TODO: read a register, return success only if the register
	//! produced a known-good value.
	return 1; // Return success
}  */



uint8_t mlx_read(mlx90614_t *pMlx)
{
	// read both the object and ambient temperature values
	if (readObject(pMlx) && readAmbient(pMlx))
	{
		// If the reads succeeded, return success
		return 1;
	}
	return 0; // Else return fail
}

uint8_t readObject(mlx90614_t *pMlx)
{
	int16_t rawObj;
	// Read from the TOBJ1 register, store into the rawObj variable
	if (I2CReadWord( MLX90614_TOBJ1, &rawObj))
	{
		// If the read succeeded
		if (rawObj & 0x8000) // If there was a flag error
		{
			return 0; // Return fail
		}
		// Store the object temperature into the class variable
		pMlx->_rawObject = rawObj;
		return 1;
	}
	return 0;
}

uint8_t readAmbient(mlx90614_t *pMlx)
{
	int16_t rawAmb;
	// Read from the TA register, store value in rawAmb
	if (I2CReadWord( MLX90614_TA, &rawAmb))
	{
		// If the read succeeds, store the read value
		pMlx->_rawAmbient = rawAmb; // return success
		return 1;
	}
	return 0; // else return fail
}


uint8_t I2CReadWord( uint8_t address, int16_t * dest)
{
i2c_start(&TWIE, MLX90614_I2CADDR, I2C_WRITE );
i2c_write(&TWIE, address); //write that you want to know the temperature
	
		i2c_restart(&TWIE, MLX90614_I2CADDR, I2C_READ);
	
	uint8_t lsb = i2c_read(&TWIE, I2C_ACK);
	uint8_t msb = i2c_read(&TWIE, I2C_ACK);
	uint8_t pec = i2c_read(&TWIE, I2C_NACK);
	
	i2c_stop(&TWIE);
			
	uint8_t crc = crc8(0, (MLX90614_I2CADDR << 1));
	crc = crc8(crc, address);
	crc = crc8(crc, (MLX90614_I2CADDR << 1) + 1);
	crc = crc8(crc, lsb);
	crc = crc8(crc, msb);
	
	if (crc == pec)
	{
		*dest = (msb << 8) | lsb;
		return 1;
	}
	else
	{
		return 0;
	}
}

float calcTemperature(mlx90614_t *pMlx, int16_t rawTemp)
{
	float retTemp;
	
	if (pMlx->_defaultUnit == TEMP_RAW)
	{
		retTemp = (float) rawTemp;
	}
	else
	{
		retTemp = (float) rawTemp * 0.02;
		if (pMlx->_defaultUnit != TEMP_K)
		{
			retTemp -= 273.15;
			if (pMlx->_defaultUnit == TEMP_F)
			{
				retTemp = retTemp * 9.0 / 5.0 + 32;
			}
		}
	}
	
	return retTemp;
}

float mlx_object(mlx90614_t *pMlx)
{
	// Return the calculated object temperature
	return calcTemperature(pMlx, pMlx->_rawObject);
}

float mlx_ambient(mlx90614_t *pMlx)
{
	// Return the calculated ambient temperature
	return calcTemperature(pMlx, pMlx->_rawAmbient);
}


 uint8_t writeEEPROM(uint8_t address, int16_t data)
{	
	// Clear out EEPROM first:
	if (I2CWriteWord(address, 0) != 0)
		return 0; // If the write failed, return 0
	_delay_ms(5); // Delay tErase
	
	uint8_t i2cRet = I2CWriteWord(address, data);
	_delay_ms(5); // Delay tWrite
	
	if (i2cRet == 0)
		return 1;
	else
		return 0;	
}

uint8_t I2CWriteWord(uint8_t address, int16_t data)
{
	uint8_t crc;
	uint8_t lsb = data & 0x00FF;
	uint8_t msb = (data >> 8);
	
	crc = crc8(0, (MLX90614_I2CADDR << 1));
	crc = crc8(crc, address);
	crc = crc8(crc, lsb);
	crc = crc8(crc, msb);
	
i2c_start(&TWIE, MLX90614_I2CADDR, I2C_WRITE );
i2c_write(&TWIE, address); 
i2c_write(&TWIE, lsb); 
i2c_write(&TWIE, msb); 
i2c_write(&TWIE, crc); 
	i2c_stop(&TWIE);
	return 0;
} 


uint8_t crc8(uint8_t inCrc, uint8_t inData)
{
	uint8_t i;
	uint8_t data;
	data = inCrc ^ inData;
	for ( i = 0; i < 8; i++ )
	{
		if (( data & 0x80 ) != 0 )
		{
			data <<= 1;
			data ^= 0x07;
		}
		else
		{
			data <<= 1;
		}
	}
	return data;
}

void setUnit(mlx90614_t *pMlx, temperature_units unit)
{
	pMlx->_defaultUnit = unit; // Store the unit into a private member
}



  uint8_t setEmissivity(float emis)
{
	// Make sure emissivity is between 0.1 and 1.0
	if ((emis > 1.0) || (emis < 0.1))
		return 0; // Return fail if not
	// Calculate the raw 16-bit value:
	uint16_t ke = (uint16_t)65535.0 * emis;
	ke = constrain(ke, 0x2000, 0xFFFF);

	// Write that value to the ke register
	return writeEEPROM(MLX90614_EMISS, (int16_t)ke);
} 

float readEmissivity(void)
{
	int16_t ke;
	if (I2CReadWord(MLX90614_EMISS, &ke))
	{
		// If we successfully read from the ke register
		// calculate the emissivity between 0.1 and 1.0:
		return (((float)((uint16_t)ke)) / 65535.0);
	}
	return 0; // Else return fail
}




uint8_t readID(mlx90614_t *pMlx)
{
	for (int i=0; i<4; i++)
	{
		int16_t temp = 0;
		// Read from all four ID registers, beginning at the first:
		if (!I2CReadWord(MLX90614_ID1 + i, &temp))
		return 0;
		// If the read succeeded, store the ID into the id array:
		pMlx->id[i] = (uint16_t)temp;
	}
	return 1;
}

uint32_t getIDH(mlx90614_t *pMlx)
{
	// Return the upper 32 bits of the ID
	return ((uint32_t)pMlx->id[3] << 16) | pMlx->id[2];
}

uint32_t getIDL(mlx90614_t *pMlx)
{
	// Return the lower 32 bits of the ID
	return ((uint32_t)pMlx->id[1] << 16) | pMlx->id[0];
} 

uint8_t readAddress(void)
{
	int16_t tempAdd;
	// Read from the 7-bit I2C address EEPROM storage address:
	if (I2CReadWord(MLX90614_REG_ADDR, &tempAdd))
	{
		// If read succeeded, return the address:
		return (uint8_t) tempAdd;
	}
	return 0; // Else return fail
}

/* uint8_t mlx_sleep(void)
{
	// Calculate a crc8 value.
	// Bits sent: _deviceAddress (shifted left 1) + 0xFF
	uint8_t crc = crc8(0, (MLX90614_I2CADDR << 1));
	crc = crc8(crc, MLX90614_SLEEP);
	
	// Manually send the sleep command:
	i2c_start(&TWIE, MLX90614_I2CADDR, I2C_WRITE );
	i2c_write(&TWIE, MLX90614_SLEEP); 
	i2c_write(&TWIE, crc); 
//	i2c_stop(&TWIE);
	
	// Set the SCL pin LOW, and SDA pin HIGH (should be pulled up)
// 	PORTE.DIRSET = SCL;
// 	PORTE.OUTCLR = SCL;
// 	PORTE.DIRCLR = SDA;

PORTE.DIRCLR = SCL;
	PORTE.DIRSET = SDA;
	PORTE.OUTCLR = SDA;
	_delay_ms(2);
	PORTE.DIRCLR = SDA;
	PORTE.DIRSET = SCL;
	PORTE.OUTCLR = SCL;
}

uint8_t mlx_wake(uint32_t freq)
{
	// Wake operation from datasheet
	//Wire.end(); // stop i2c bus to send wake up request via digital pins
// 		PORTE.DIRSET = SCL;
// 		PORTE.OUTCLR = SCL;
// 		PORTE.DIRCLR = SDA;
// 		_delay_ms(50);
// 	PORTE.DIRCLR = SCL;
// 	PORTE.DIRSET = SDA; // SDA low
// 	PORTE.OUTCLR = SDA; // SDA low
// 	_delay_ms(40); // delay at least 33ms
// 	PORTE.OUTSET = SDA; // SDA high
// 	_delay_ms(300);
// 		PORTE.OUTCLR = SCL;
// 		_delay_ms(10);
// 		PORTE.OUTSET = SCL;

		PORTE.DIRCLR = SCL;
		PORTE.DIRSET = SDA;
		PORTE.OUTCLR = SDA;
		_delay_ms(15);
		PORTE.OUTSET = SDA;
		
		
		
	//i2c_init(&TWIE, TWI_BAUD(freq, BAUD_100K));
		PORTE.DIRSET = PIN1_bm|PIN0_bm;
		printf("Should be waked\n");
} */
