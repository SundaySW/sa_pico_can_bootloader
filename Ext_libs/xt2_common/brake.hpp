//SA DEc 29, 2020
#ifndef BRAKE_H_
#define BRAKE_H_

#include "stm32g4xx_hal.h"
#include "port_pin.hpp"
#include "system_def.hpp"

class BrakeC
{
public:
	BrakeC ()
	{

	}
	BrakeC (const PortPin& config)
	{
		port = config.Port;
		pin  = config.Pin;
	}

	bool IsLocked() const
	{
		return (HAL_GPIO_ReadPin(port, pin) == 0);
	}

	bool IsReleased() const
	{
		return !IsLocked();
	}

	void Lock()
	{
		Timeout = TIMEOUT_INFINITE;
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
	}

	void OnTimer(int ms)
	{
		if (IsReleased() && Timeout != TIMEOUT_INFINITE)
		{
			if (Timeout) Timeout--;
			if (Timeout == 0) {
				Lock();
			}
		}
	}

	void Release(int timeout = TIMEOUT_INFINITE)
	{
		Timeout = timeout;
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	}

	GPIO_TypeDef* port = 0;
	uint16_t pin = 0;
	int  Timeout = TIMEOUT_INFINITE; //release timeout
};

#endif /* BRAKE_H_ */
