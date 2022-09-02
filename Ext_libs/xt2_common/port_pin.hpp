//30.03.21

#ifndef APP_PORT_CFG_HPP_
#define APP_PORT_CFG_HPP_


class PortPin
{
public:
	PortPin(const PortPin& portpin)
	{
		Port = portpin.Port;
		Pin = portpin.Pin;
	}
	PortPin(GPIO_TypeDef* port, uint16_t pin ){
		Port = port;
		Pin = pin;
	}

	bool Read() const {
		return HAL_GPIO_ReadPin(Port, Pin);
	}
	void Write(bool state){
		HAL_GPIO_WritePin(Port, Pin, (GPIO_PinState)state);
	}

	GPIO_TypeDef* Port = 0;
	uint16_t Pin = 0;
};




#endif /* APP_PORT_CFG_HPP_ */
