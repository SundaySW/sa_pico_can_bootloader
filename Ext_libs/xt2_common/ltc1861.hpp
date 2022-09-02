//SA Dec 21, 2020
#ifndef APP_LTC1861_HPP_
#define APP_LTC1861_HPP_

#include "stm32g4xx_hal.h"

class LTC1861
{
public:
	LTC1861(GPIO_TypeDef* _cs_port, uint16_t _cs_pin, SPI_HandleTypeDef* _hspi2)
	{
		cs_port = _cs_port;
		cs_pin = _cs_pin;
		hspi2 = _hspi2;
	}

	void init()
	{
		getAdcResult(0);
	}

	uint16_t getAdcResult(uint8_t next_channel)
	{
		uint16_t result = 0;

		ModeToTr[0] = 0b10000000; // single ended for one channel #0
		if (next_channel)
			ModeToTr[0] = 0b11000000; // single ended for one channel #0

		//switch to spi mode 1,1
		hspi2->Init.CLKPolarity = SPI_POLARITY_HIGH;
		hspi2->Init.CLKPhase = SPI_PHASE_2EDGE;
		if (HAL_SPI_Init(hspi2) != HAL_OK) {
			return -1;
		}


		//lower ADC_CS pin (select chip)
		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);

	//	HAL_SPI_Receive(&hspi2, RxBuffer, 2, 5000);

		//receive adc result
		switch (HAL_SPI_TransmitReceive(hspi2, ModeToTr, RxB, 2, 10))
		{
		case HAL_OK:
			result = (((uint16_t)RxB[0] << 8) + (uint16_t)RxB[1]) >> 4;
			break;
		case HAL_TIMEOUT:
		// SPI ERROR
		case HAL_ERROR:
		// SPI ERROR
		//  Error_Handler();
		  break;
		default:
		  break;
		}

		//raise ADC_CS pin (release chip)
		HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
		//Delay 3.3uS
		//HAL_Delay(1);

		return result;
	}

	uint16_t GetAvgResult(uint8_t channel)
	{
		uint16_t result = 0;
		if (Count[channel])
			{
				result = Sum[channel] / Count [channel];
			}
		Sum[channel] = 0;
		Count[channel] = 0;
		return result;
	}

	void OnTimer()
	{
		Sum[CurrentChannel] += getAdcResult(NextChannel);
		Count[CurrentChannel]++;
		if (CurrentChannel++ >= 1)
			CurrentChannel = 0;
		if (NextChannel++ >= 1)
			NextChannel = 0;
	}



private:
	float Sum[2] = {0};
	uint32_t Count[2] = {0};
	uint8_t CurrentChannel = 0, NextChannel = 1;

	uint8_t ModeToTr[4] = {0};
	uint8_t RxB[4] = {0};

	GPIO_TypeDef* cs_port = 0;
	uint16_t cs_pin = 0;
	SPI_HandleTypeDef* hspi2;

};







#endif /* APP_LTC1861_HPP_ */
