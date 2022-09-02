//SA 9.01.21
#ifndef APP_STEPPER_PID_HPP_
#define APP_STEPPER_PID_HPP_

#include "adc_param.hpp"
#include "brake.hpp"
#include "variant.h"
#include "math.h"



class StepperPID
{
public:

	enum DIRECTION
	{
		FWD = 0,
		REV = 1,
	};

	StepperPID()
	{

	}

	StepperPID(Brake* _brake, ADC_Param* _adc, GPIO_TypeDef* _step_port, uint16_t _step_pin, GPIO_TypeDef* _dir_port, uint16_t _dir_pin, GPIO_TypeDef* _en_port, uint16_t _en_pin, TIM_HandleTypeDef* _htim)
	{
		brake = _brake;
		adc = _adc;
		step_port = _step_port;
		step_pin = _step_pin;
		dir_port = _dir_port;
		dir_pin = _dir_pin;
		en_port = _en_port;
		en_pin = _en_pin;
		htim = _htim;
	}

	StepperPID(ADC_Param* _adc, GPIO_TypeDef* _step_port, uint16_t _step_pin, GPIO_TypeDef* _dir_port, uint16_t _dir_pin, GPIO_TypeDef* _en_port, uint16_t _en_pin, TIM_HandleTypeDef* _htim)
	{
		adc = _adc;
		step_port = _step_port;
		step_pin = _step_pin;
		dir_port = _dir_port;
		dir_pin = _dir_pin;
		en_port = _en_port;
		en_pin = _en_pin;
		htim = _htim;
	}

	void enable()
	{
		PID_Enabled = true;
		HAL_GPIO_WritePin(en_port, en_pin, GPIO_PIN_RESET);
		if (brake)
			brake->Release();
	}
	void disable()
	{
		PID_Enabled = false;
		HAL_GPIO_WritePin(en_port, en_pin, GPIO_PIN_SET);
		if (brake)
			brake->Lock();
	}

	void halfStep()
	{
		HAL_GPIO_TogglePin(step_port, step_pin);
	}
	void setDirection(uint8_t dir)
	{	//FWD = 0 (reset) , REV = 1 (set)
		HAL_GPIO_WritePin(dir_port, dir_pin, (GPIO_PinState)dir);
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
			setDirection(FWD);
		else
			setDirection(REV);

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
				V= Vmin;
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
			halfStep();
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

	float brakePath = brakePathMin; // path to brake @A accel for current Commanded Speed Vcmd. (not for current speed)!

	//axis settings:
	float A = 6000.0f;				// Acceleration in steps/second^2
	float Vmin = 300.0f;			// Minimum speed in steps/second
	float Vmax = 10000.0f;			// Maximum speed in steps/second
	float stepsADCconvertCoef = 46.0f; // (~24 for CV )coefficient to convert mms to ADC counts (empirically calculated)
	uint16_t VPosMin = 1500;		//low limit in ADC counts
	uint16_t VPosMax = 2500;		//high limit in ADC counts
	uint16_t brakePathMin = 20;		//position error less than these borders is ignored..

	bool PID_Enabled = false;

	uint8_t cmdManualMove (DIRECTION dir)
	{
		if (dir == FWD)
			PosToGo = VPosMax;
		else
			PosToGo = VPosMin;

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
		if (posToGo > VPosMax || posToGo < VPosMin)
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

	Brake* brake = 0;
	ADC_Param* adc = 0;
	GPIO_TypeDef* step_port = 0;
	uint16_t step_pin = 0;
	GPIO_TypeDef* dir_port = 0;
	uint16_t dir_pin = 0;
	GPIO_TypeDef* en_port = 0;
	uint16_t en_pin = 0;
	TIM_HandleTypeDef* htim = 0;

};



#endif /* APP_STEPPER_PID_HPP_ */
