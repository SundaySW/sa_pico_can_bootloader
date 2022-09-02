//SA 14.03.21
#ifndef AXIS_HPP_
#define AXIS_HPP_

#include "stepper.hpp"
#include "stepper_pid.hpp"
#include "brake.hpp"
#include "adc_param.hpp"
#include "calibration.hpp"
#include "position_control.hpp"



class Axis
{
public:
//	Axis (Calibration* _calibration, PositionCtrl* _positionCtrl)
//	{
//		calibration = _calibration;
//		positionCtrl = _positionCtrl;
//	}

	Axis (const CalibConfig& calib, PositionCtrl* _positionCtrl)
		: Calib(calib)
	{
		positionCtrl = _positionCtrl;
	}

	void Init()
	{
		Calib.Init();
		//.....
	}

	uint16_t Convert_mmToAdc(uint16_t mm)
	{
		if (calibration->mult)
			return ((mm - calibration->offset) / calibration->mult);
	}

	uint16_t Convert_adcToMM(uint16_t adc)
	{
		return (adc * calibration->mult + calibration->offset);
	}

	void UpdatePosition()
	{
		position = Convert_adcToMM((uint16_t)(positionCtrl->adc->GetValue()));
	}

	void Calibrate(uint8_t point, uint16_t pos, uint16_t adc)
	{
		Calib.writePoint(point, pos, adc);
		Calib.Init();
	}

//	bool Move_ADC (uint16_t posToGo, uint16_t speed)
//	{
//		if (posToGo > positionCtrl->adcMax || posToGo < positionCtrl->adcMin)
//		{
//			//out of range  TODO: process error
//			return 0;
//		}
//
//		positionCtrl->PosToGo = posToGo;
//
//		positionCtrl->Vcmd = speed;
//		if (positionCtrl->Vcmd > positionCtrl->Vmax)
//			positionCtrl->Vcmd = positionCtrl->Vmax;
//
//		if (abs(positionCtrl->V) <= positionCtrl->Vmin)
//		{
//			if (positionCtrl->PosToGo - adc->GetValue() > 0)
//				V = Vtoset = Vmin;
//			else
//				V = Vtoset = -Vmin;
//
//			positionCtrl->enable();
//			__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));
//		}
//		return 1;
//	}
//
//	 bool Move (uint16_t mm, uint16_t speed)
//	{
//		return Move_ADC((mm), speed);
//	}

	uint16_t position = 0;

	Calibration* calibration = 0;
	Calibration Calib;
	PositionCtrl* positionCtrl = 0;
};



#endif /* AXIS_HPP_ */
