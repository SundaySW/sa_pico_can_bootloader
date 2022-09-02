//SA 30.03.21
#ifndef APP_AXIS_BASE_HPP_
#define APP_AXIS_BASE_HPP_

#include "calibration.hpp"
#include "adc_param.hpp"
#include "brake.hpp"
#include "port_pin.hpp"

enum BRAKE_TYPE
{
	EM = 0,
	STEP_DRIVER = 1,
};

class BaseAxis
{
public:
	BaseAxis(const CalibConfig& calib, ADC_Param* adc)
		: Calib(calib)
		, Adc(adc)
	{
		BrakeType = STEP_DRIVER;
	}
	BaseAxis(const PortPin& brake, const CalibConfig& calib, ADC_Param* adc)
		: Calib(calib)
		, Brake(brake)
		, Adc(adc)
	{
		BrakeType = EM;
	}

	void Init()
	{
		Calib.Init();
	}

	void OnTimer(int ms)
	{
		Brake.OnTimer(ms);
	}

	void SetCalibPoint(uint8_t point, uint16_t pos, uint16_t adc)
	{
		Calib.writePoint(point, pos, adc);
		Calib.Init();
	}
	Point GetCalibPoint(uint8_t point)
	{
		Point p;
		Calib.readPoint(point, &p.pos, &p.adc);
		return p;
	}

	int16_t GetCurrentPosition()
	{
		int16_t position = 0;
		if (Calib.type == CALIB_TYPE::LINEAR){
			position = (int16_t)(Adc->GetValue() * Calib.mult + Calib.offset);
		}
		else if (Calib.type == CALIB_TYPE::LUT){
			//TODO: LUT for 5 points
			int adc = Adc->GetValue();
			int i;
			for (i = 0; i < 5;) // 5 = sizeof(Calib.P)/sizeof(Calib.P[0])
			{
				if (adc < Calib.P[i].adc)
					break;
				i++;
			}
			//check area is correct

			//interpolate linear:
			if (i > 0 && i < 5) {
				if (Calib.P[i].adc - Calib.P[i - 1].adc)
					position = Calib.P[i-1].pos + (adc - Calib.P[i - 1].adc) * (Calib.P[i].pos - Calib.P[i - 1].pos) / (Calib.P[i].adc - Calib.P[i - 1].adc);
			}

			else if (i == 0)
			{
				//exrapolate below P[0]
				if (Calib.P[1].adc - Calib.P[0].adc)
					position = Calib.P[0].pos - (Calib.P[0].adc - adc) * (Calib.P[1].pos - Calib.P[0].pos) / (Calib.P[1].adc - Calib.P[0].adc);
			}
			else if (i == 5)
			{
				//extrapolate above P[4]
				if (Calib.P[4].adc - Calib.P[3].adc)
					position = Calib.P[4].pos + (adc - Calib.P[4].adc) * (Calib.P[4].pos - Calib.P[3].pos) / (Calib.P[4].adc - Calib.P[3].adc);
			}
		}
		return position;
	}

	void ReleaseBrake(int timeout)
	{
		if (BrakeType == EM)
			Brake.Release(timeout );

	}
	void LockBrake()
	{
		if (BrakeType == EM)
			Brake.Lock();
	}

	uint16_t ConvertPosToAdc(int16_t pos)	// mm or deg
	{
		if (Calib.mult)
			return ((pos - Calib.offset) / Calib.mult);
		return 0;
	}

	int16_t ConvertAdcToPos(uint16_t adc)
	{
		return (adc * Calib.mult + Calib.offset);
	}

	int ConvertPosToSteps(int16_t pos)	// mm or deg
	{
		//TODO check coeff !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		return (ConvertPosToAdc(pos) * Calib.stepsToADC);
	}

public:
	BRAKE_TYPE BrakeType;
	Calibration Calib;
	BrakeC Brake;
	ADC_Param* Adc = 0;
};


#endif /* APP_AXIS_BASE_HPP_ */
