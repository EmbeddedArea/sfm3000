/**
 ********************************************************************************
 * @file    sfm3000.c - Source file for SFM3000 Mass Flow Sensor
 * @author  EmbeddedArea (embeddedarea.com)
 * @date    May 5, 2020
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "sfm3000.h"

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/

/************************************
 * GLOBAL VARIABLES
 ************************************/
const uint32_t timeoutParam = TRANSMISSON_TIMEOUT_PARAMETER;

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/

/************************************
 * GLOBAL FUNCTIONS
 ************************************/

/**
 * @brief Initiates the data reading from the sensor, sends the two byte command to start continuous measurement.
 * @return
 */
uint8_t sfm3000init(void){
	uint8_t Buffer[SFM3000_DEVICE_I2C_COMMAND_SIZE];
	Buffer[0] = READ_FLOW_DATA_BYTE_1;
	Buffer[1] = READ_FLOW_DATA_BYTE_2;
	return (uint8_t) I2CwriteBytes(SFM3000_I2C_ADDRESS, Buffer, SFM3000_DEVICE_I2C_COMMAND_SIZE);
}


/**
 * @brief Reads the data, after the sfm3000init() which starts the continuous measurement. Offset and scale values
 * for the air defined as  SFM3000_OFFSET_PARAMETER and SFM3000_SCALE_PARAMETER in .h file with respect to datasheet.
 * @param offset
 * @param scale
 * @param Status
 * @return
 */
float sfm3000getValue(float offset, float scale, uint8_t * Status){
	uint8_t Buffer[3] = {0};
	*Status = I2CreadBytes(SFM3000_I2C_ADDRESS, Buffer, SFM3000_READ_DATA_SIZE);
#ifdef CRC_ON
	if(HAL_OK == sfm3000crcCheck(Buffer[0], Buffer[1], Buffer[2]))
		return ((((float) ((Buffer[0] << 8) | Buffer[1])) - offset) / scale);
	else
		return HAL_ERROR;
#else
	return ((((float) ((Buffer[0] << 8) | Buffer[1])) - offset) / scale);
#endif

}


/**
 * @brief Initiates soft reset to the sensor, sends the two byte command to reset.
 * @return
 */
uint8_t sfm3000reset(void){
	uint8_t Buffer[SFM3000_DEVICE_I2C_COMMAND_SIZE];
	Buffer[0] = SOFT_RESET_BYTE_1;
	Buffer[1] = SOFT_RESET_BYTE_2;
	return (uint8_t) I2CwriteBytes(SFM3000_I2C_ADDRESS, Buffer, SFM3000_DEVICE_I2C_COMMAND_SIZE);
}


/**
 * @brief Calculates the CRC byte with respect to input.
 * @return
 */
uint32_t sfm3000readSerialDevId(void){
	uint8_t Buffer[SFM3000_SERIAL_ID_SIZE];
	Buffer[0] = READ_SERIAL_ID_BYTE_1;
	Buffer[1] = READ_SERIAL_ID_BYTE_2;
	if(HAL_OK == I2CwriteBytes(SFM3000_I2C_ADDRESS, Buffer, SFM3000_DEVICE_I2C_COMMAND_SIZE)){
		HAL_Delay(5);
		I2CreadBytes(SFM3000_I2C_ADDRESS, Buffer, SFM3000_SERIAL_ID_SIZE);
#ifdef CRC_ON
		if(HAL_OK == (sfm3000crcCheck(Buffer[0], Buffer[1], Buffer[2]) & sfm3000crcCheck(Buffer[3], Buffer[4], Buffer[5])))
			return ((Buffer[0] << 24) | (Buffer[1] << 16) | (Buffer[3] << 8) | (Buffer[4]));
		else
			return HAL_ERROR;
#endif
		return ((Buffer[0] << 24) | (Buffer[1] << 16) | (Buffer[3] << 8) | (Buffer[4]));
	}
	return HAL_ERROR;
}


/**
 * @brief Calculates the CRC byte with respect to input.
 * @param data
 * @param crc
 * @return
 */
uint8_t sfm3000crc8(const uint8_t data, uint8_t crc){
	crc ^= data;
	for (uint8_t i = 8; i; --i )
		crc = ( crc & 0x80 ) ? (crc << 1) ^ 0x31 : (crc << 1);
	return crc;
}


/**
 * @brief Checks the response from the sensor with respect to CRC.
 * @param in1
 * @param in2
 * @param crcRead
 * @return
 */
uint8_t sfm3000crcCheck(uint8_t in1, uint8_t in2, uint8_t crcRead){
	uint8_t crcTemp = 0xFF;
	crcTemp = sfm3000crc8(in1, crcTemp);
	if(crcRead == sfm3000crc8(in2, crcTemp))
		return HAL_OK;
	else
		return HAL_ERROR;
}


/*
 *	Library can be used with different I2C libraries by only changing these functions without
 *	changing the SFM3000 related functions.
 *
 */
uint8_t I2CwriteBytes(uint16_t devAddr, uint8_t * buffer, uint8_t size){
	return (uint8_t) HAL_I2C_Master_Transmit(SFM3000_I2C_INS, devAddr, buffer, size, timeoutParam);
}
uint8_t I2CreadBytes(uint16_t devAddr, uint8_t * buffer, uint8_t size){
	return (uint8_t) HAL_I2C_Master_Receive(SFM3000_I2C_INS, devAddr, buffer, size, timeoutParam);
}
