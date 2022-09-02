//SA 14.03.21
#ifndef APP_STEP_DRIVER_HPP_
#define APP_STEP_DRIVER_HPP_

#include "port_pin.hpp"


struct StepDriverCfg
{
	PortPin Step;
	PortPin Dir;
	PortPin En;
};

class StepDriver
{
public:
	StepDriver(const StepDriverCfg& config)
	{
		step_port = config.Step.Port;
		step_pin  = config.Step.Pin;
		dir_port  = config.Dir.Port;
		dir_pin  = config.Dir.Pin;
		en_port  = config.En.Port;
		en_pin  = config.En.Pin;
	}

	void enable()
	{
		HAL_GPIO_WritePin(en_port, en_pin, GPIO_PIN_RESET);
	}
	void disable()
	{
		HAL_GPIO_WritePin(en_port, en_pin, GPIO_PIN_SET);
	}
	void halfStep()
	{
		HAL_GPIO_TogglePin(step_port, step_pin);
	}
	void setDirection(uint8_t dir)
	{
		HAL_GPIO_WritePin(dir_port, dir_pin, (GPIO_PinState)dir);
	}

private:
	GPIO_TypeDef* step_port = 0;
	uint16_t step_pin = 0;
	GPIO_TypeDef* dir_port = 0;
	uint16_t dir_pin = 0;
	GPIO_TypeDef* en_port = 0;
	uint16_t en_pin = 0;
};




#endif /* APP_STEP_DRIVER_HPP_ */
