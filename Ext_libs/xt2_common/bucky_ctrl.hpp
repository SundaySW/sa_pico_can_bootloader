#ifndef APP_BUCKY_CTRL_HPP_
#define APP_BUCKY_CTRL_HPP_


#include "port_pin.hpp"


class BuckyCtrl
{
public:
	struct Pinout
	{
		PortPin BuckyCall_toA4;
		PortPin BuckyCall_fromGen;
		PortPin BuckyReady_fromA4;
		PortPin BuckyReady_toGen;
		PortPin OnTomo_toA4;
		PortPin Grid_120;
		PortPin Grid_180;
		PortPin BuckyBrake_fromA4;
	};

	BuckyCtrl(const Pinout& pins)
		: Pins(pins)
	{

	}

	void OnPoll()
	{

	}


private:
	Pinout Pins;



};


#endif /* APP_BUCKY_CTRL_HPP_ */
