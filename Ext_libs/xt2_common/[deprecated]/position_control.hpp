//SA 14.03.21
#ifndef APP_POSITION_CONTROL_HPP_
#define APP_POSITION_CONTROL_HPP_

#include "stm32g4xx_hal.h"

#include "adc_param.hpp"
#include "brake.hpp"
#include "step_driver.hpp"

#include "variant.h"
#include "math.h"

enum DIRECTION
{
	FWD = 0,
	REV = 1,
};

class PositionCtrl
{
public:
	PositionCtrl(StepDriver* _driver, Brake* _brake, ADC_Param* _adc, TIM_HandleTypeDef* _htim)
	{
		driver = _driver;
		brake = _brake;
		adc = _adc;
		htim = _htim;
	}

	PositionCtrl(StepDriver* _driver, ADC_Param* _adc, TIM_HandleTypeDef* _htim)
	{
		driver = _driver;
		brake = 0;
		adc = _adc;
		htim = _htim;
	}

	void enable()
	{
		PID_Enabled = true;
		driver->enable();
		if (brake)
			brake->Release();

		__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));	//or use V??
	}
	void disable()
	{
		PID_Enabled = false;
		driver->disable();
		if (brake)
			brake->Lock();
	}

	void onTimer()
	{
		if (!PID_Enabled)
			return;

		if (V < Vtoset)
		{
			if (abs(V) >= Vmin)
				V += A / abs(V);
//			else V += Vmin;
			else V = Vmin;
		}
		else if (V > Vtoset)
		{
			if (abs(V) >= Vmin)
				V -= A / abs(V);
//			else V -= Vmin;
			else V = -Vmin;
		}

		if (V > 0)
			driver->setDirection(FWD);
		else
			driver->setDirection(REV);

		e = PosToGo - adc->GetValue();

		float deltaV = abs(V) - Vmin;// - shorter path, but has overpositioning issues
		float brakeTime = deltaV / A; //seconds -> to timer interrupts -> to path
		brakePath = brakeTime * deltaV / (2 * stepsADCconvertCoef);
		if (brakePath < brakePathMin)
			(brakePath = brakePathMin);

		if (abs(e) > brakePathMin )
		{
			//enable();
			Vtoset = (Vcmd-Vmin) * e / (brakePath) + Vmin; // e is positive or negative
			if (abs(Vtoset) > Vcmd)
				Vtoset = Vcmd * e / abs(e);
//			else if (abs(Vtoset) < Vmin)
//				Vtoset = Vmin * e / abs(e);
		}
		else
		{
			Vtoset = Vmin;
			if (V > 0)
				V = Vmin;
			else
				V = -Vmin;
		}

		if (abs(e) < 3 && abs(V) == Vmin)  // (abs(V) <=  (Vmin * 1.5)) // ( -border) > e > + border and low speed
		{
			V = 0; Vtoset = 0; Vcmd = Vmin;

			if (PID_Enabled)
				disable();
		}

		if (abs(V) >= Vmin)
		{
			int buf = (int) (1000000 / abs(V));
			__HAL_TIM_SET_AUTORELOAD(htim, buf);
			driver->halfStep();
		}
		else
		{
			int buf = (int) (1000000 / Vmin);
			__HAL_TIM_SET_AUTORELOAD(htim, buf);
		}
	}// end of OnTimer();

	float PosToGo = 0.0f;  			// position to reach in ADC counts
	float e = 0.0f; 				// current position error in ADC counts (e = PosToGo - adc->GetValue();)
	float V = 0.0f;					// Current Velocity in steps/second
	float Vcmd = 0.0f;				// V from cmd
	float Vtoset = 0.0f;			// Velocity to aim

	//axis settings:
	float A = 6000.0f;				// Acceleration in steps/second^2
	float Vmin = 300.0f;			// Minimum speed in steps/second
	float Vmax = 10000.0f;			// Maximum speed in steps/second
	float stepsADCconvertCoef = 46.0f; // (~24 for CV )coefficient to convert mms to ADC counts (empirically calculated)
	uint16_t adcMin = 1500;		//low limit in ADC counts
	uint16_t adcMax = 2500;		//high limit in ADC counts
	uint16_t brakePathMin = 20;		//position error less than these borders is ignored..

	float brakePath = brakePathMin; // path to brake @A accel for current Commanded Speed Vcmd. (not for current speed)!

	bool PID_Enabled = false;


	uint8_t cmdManualMove (DIRECTION dir)
	{
		if (dir == FWD)
			PosToGo = adcMax;
		else
			PosToGo = adcMin;

		Vcmd = 3000;

		enable();
		__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));
		return 1;
	}

	uint8_t stop ()
	{
		if (V > Vmin)
			PosToGo = adc->GetValue() + brakePath;
		else if (V < -Vmin)
			PosToGo = adc->GetValue() - brakePath;
		return 0;
	}

	uint8_t cmdMove (uint16_t posToGo, uint16_t speed)
	{
		if (posToGo > adcMax || posToGo < adcMin)
		{
			//out of range  TODO: process error
			return 0;
		}

		PosToGo = posToGo;

		Vcmd = speed;
		if (Vcmd > Vmax)
			Vcmd = Vmax;

		if (abs(V) <= Vmin)
		{
			if (PosToGo - adc->GetValue() > 0)
				V = Vtoset = Vmin;
			else
				V = Vtoset = -Vmin;

			enable();
			__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));
		}

		return 1;
	}

//private:
public:
	Brake* brake = 0;
	ADC_Param* adc = 0;
	StepDriver* driver = 0;
	TIM_HandleTypeDef* htim = 0;

};




#endif /* APP_POSITION_CONTROL_HPP_ */
