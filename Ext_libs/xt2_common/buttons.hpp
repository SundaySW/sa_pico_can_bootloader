//SA Sep 23, 2020
#ifndef SRC_BUTTONS_HPP_
#define SRC_BUTTONS_HPP_

#include "stm32g4xx_hal.h"


class Button
{
public:
	enum PRESS_DELAY
	{
		ANTIBOUNCE = 50,

	};
	enum RETURN_STATE
	{
		NO_PRESS = 0,
		PRESSED  = 1,
		SF_ERROR = 5,
	};

	Button (GPIO_TypeDef * _port, uint16_t _pin)
	{
		port = _port;
		pin = _pin;
		SFCheck = 0;
	}
	Button (GPIO_TypeDef* _port, uint16_t _pin, Button* _sfcheck, uint8_t _id)
	{
		port = _port;
		pin = _pin;
		SFCheck = _sfcheck;
		id = _id;
	}

	GPIO_TypeDef* port = 0;
	uint16_t pin = 0;
	Button* SFCheck = 0;
	uint8_t id = 0;

	uint8_t state = 0;
	uint32_t timerCounter = 0, ms = 0;
	uint8_t isPressed = 0;

	virtual void OnPress()
	{

	}

	void throwSFError()
	{
		//TODO: throw single fault error of button.
	}

	bool IsPinActive() const
	{
		return HAL_GPIO_ReadPin(port, pin);
	}

	RETURN_STATE Poll()
	{
		state = HAL_GPIO_ReadPin(port, pin);
		ms = HAL_GetTick();
		uint8_t SFState = HAL_GPIO_ReadPin(SFCheck->port, SFCheck->pin);

		if (state == GPIO_PIN_RESET)
		{
			// SF state for buttons with shared SF line - can be 0 or 1 if other button is pressed

			timerCounter = ms;
			isPressed = false;
			return NO_PRESS;
		}

		else if (SFState != GPIO_PIN_RESET)
		{
			throwSFError();
			return SF_ERROR;
		}

		else if ((ms - timerCounter) > PRESS_DELAY::ANTIBOUNCE)
		{
			OnPress();
			isPressed = true;
			return PRESSED;
		}

		return NO_PRESS;
	}
};

#endif /* SRC_BUTTONS_HPP_ */
