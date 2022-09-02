//SA 05.01.21
#ifndef APP_ADC_PARAM_HPP_
#define APP_ADC_PARAM_HPP_

#include "base_param.h"
#include "ltc1861.hpp"

class ADC_Param : public UpdateParam, public FloatParam
{
public:
	ADC_Param()
{

}
	ADC_Param(LTC1861* _pLTC1861, uint8_t _channel, float _calibOffset, float _calibMult)
	{
		pLTC1861 = _pLTC1861;
		channel = _channel;
		calibOffset = _calibOffset;
		calibMult = _calibMult;
	}

	bool UpdateValue ()
	{
		Value = (pLTC1861->GetAvgResult(channel) + calibOffset) * calibMult;
		return true;
	}

	uint8_t GetChannelNumber()
	{
		return channel;
	}

private:
	LTC1861* pLTC1861;
	uint8_t channel = 0xff;
	float calibOffset = 0, calibMult = 1;
};




#endif /* APP_ADC_PARAM_HPP_ */
