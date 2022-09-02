//SA 15.02.21
#ifndef APP_25AA02UID_HPP_
#define APP_25AA02UID_HPP_

#include "stm32g4xx_hal.h"

class C_25AA02UID
{
public:

	enum INSTRUCTION
	{
		  READ  =	0x03
		, WRITE = 	0x02
		, WREN  = 	0x06
		, WRDI  =	0x04
		, RDSR  =	0x05
		, WRSR  = 	0x01
	};

	C_25AA02UID(GPIO_TypeDef* _cs_port, uint16_t _cs_pin, SPI_HandleTypeDef* _hspi)
	{
		cs_port = _cs_port;
		cs_pin = _cs_pin;
		hspi = _hspi;
	}

	uint32_t ReadUID()
	{
		uint8_t buf[6] = {0};
		if (!Read(0xFA, 6, buf))
			return 0;

		if (buf[0] == 0x29 && buf[1] == 0x51) //for 25AA02UID iC 0x51 is SPI and 2kbit density.The manufacturer code is fixed as 0x29
			return ((buf[2] << 24) + (buf[3] << 16) + (buf[4] << 8) + buf[5]);
		else
			return 0;
	}

	bool Read(uint8_t addr, uint8_t len, uint8_t* RxBuf)
	{
		//switch to spi mode 0,0
	    hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
	    hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
	    if (HAL_SPI_Init(hspi) != HAL_OK) {
	    	return false;
	    }
		//lower ADC_CS pin (select chip)
		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);

		uint8_t TxBuf[2] = {READ, addr};

		if (HAL_SPI_Transmit(hspi, TxBuf, 2, 10) != HAL_OK)
			return false;
		//receive data
		if (HAL_SPI_Receive(hspi, RxBuf, len, 10) != HAL_OK)
			return false;

		//raise ADC_CS pin (release chip)
		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

		return true;
	}

	bool Write(uint8_t addr, uint8_t len, uint8_t* TxBuf)
	{
		if ((len > 16) || (addr > 0xF9))
			return false;
		//switch to spi mode 0,0
		hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
		hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
		if (HAL_SPI_Init(hspi) != HAL_OK) {
			return false;
		}

		//lower ADC_CS pin (select chip)
		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);

		//unlock write possiblity and write function
		uint8_t tx_header[2] = {WREN, addr};
		if (HAL_SPI_Transmit(hspi, tx_header, 1, 10) != HAL_OK)
			return false;

		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
		//need to wait a bit (@64Mhz and spi 8Mhz - no additional delay needed)

		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);

		tx_header[0] = WRITE;
		if (HAL_SPI_Transmit(hspi, tx_header, 2, 10) != HAL_OK)
			return false;

		if (HAL_SPI_Transmit(hspi, TxBuf, len, 10) != HAL_OK)
			return false;

		//lock write - locked after write internally (p.9 of datasheet)

		//raise ADC_CS pin (release chip)
		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

		return true;
	}


private:

	GPIO_TypeDef* cs_port = 0;
	uint16_t cs_pin = 0;
	SPI_HandleTypeDef* hspi;
};
#endif /* APP_25AA02UID_HPP_ */
