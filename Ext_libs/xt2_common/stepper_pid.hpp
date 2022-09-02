#ifndef APP_STEPPER__PID_HPP_
#define APP_STEPPER__PID_HPP_

#include "stepper_base.hpp"
#include "stm32g4xx_hal.h"
#include <math.h>
#include "system_def.hpp"

struct PIDStepperCfg
{
	bool progressiveAcceleration = false;
	float A;					// Acceleration in steps/second^2
	float Vmin;					// Minimum speed in steps/second
	float Vmax;					// Maximum speed in steps/second
	float stepsADCconvertCoef;  // (~24 for CV )coefficient to convert mms to ADC counts (empirically calculated)
	uint16_t brakePathMin;		//position error less than these borders is ignored..
};

class PIDStepper : public BaseStepper
{
public:
	PIDStepper(const StepDriverCfg& driver, TIM_HandleTypeDef* _htim, ADC_Param* adc)
		: BaseStepper(driver, _htim)
		, Adc(adc)
	{
	}

	void Init(const PIDStepperCfg& cfg)
	{
		progressiveAcceleration = cfg.progressiveAcceleration;
		A = cfg.A;
		Vmin = cfg.Vmin;
		Vmax= cfg.Vmax;
		stepsADCconvertCoef = cfg.stepsADCconvertCoef;
		brakePathMin = cfg.brakePathMin;
	}

	int Control(int& event) override
	{
		if (!PID_Enabled)
			return 0;

		if (V < Vtoset)
		{
			if (abs(V) >= Vmin){
				if (progressiveAcceleration)
					V += A / 1000;
				else
					V += A / abs(V);
			}

//			else V += Vmin;
			else V = Vmin;
		}
		else if (V > Vtoset)
		{
			if (abs(V) >= Vmin){
				if (progressiveAcceleration)
					V -= A / 1000;
				else
					V -= A / abs(V);
			}
//			else V -= Vmin;
			else V = -Vmin;
		}

		if (V > 0)
			setDirection(FWD);
		else
			setDirection(REV);

		e = AdcToGo - Adc->GetValue();

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
			{
				disablePID();
				event = EVENT_STOP;
			}
		}

		if (abs(V) >= Vmin && PID_Enabled)
		{
			int buf = (int) (1000000 / abs(V));
			__HAL_TIM_SET_AUTORELOAD(htim, buf);
			return halfStep();
		}
		else
		{
			int buf = (int) (1000000 / Vmin);
			__HAL_TIM_SET_AUTORELOAD(htim, buf);
		}
		return 0;
	}// end of OnTimer();

	bool Move(int adcToGo, uint16_t speed, bool tomo) override
	{
		AdcToGo = adcToGo;

		Vcmd = speed;
		if (Vcmd > Vmax)
			Vcmd = Vmax;

		if (abs(V) <= Vmin)
		{
			if (AdcToGo - Adc->GetValue() > 0)
				V = Vtoset = Vmin;
			else
				V = Vtoset = -Vmin;

			enablePID();
			__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));
		}

		return 1;
	}

	void Stop() override
	{
		if (V > Vmin)
			AdcToGo = Adc->GetValue() + brakePath;
		else if (V < -Vmin)
			AdcToGo = Adc->GetValue() - brakePath;
	}

	void enablePID()
	{
		PID_Enabled = true;
		Driver.enable();
		__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));	//or use V??
	}

	void disablePID()
	{
		PID_Enabled = false;
		Driver.disable();
	}

	bool IsMoving() const override
	{
		return PID_Enabled;
	}

	int GetVelocity() const override
	{
		return V;
	}

	void SetVelocity(int speed)  override
	{
		Vtoset = speed;
	}

private:
	float AdcToGo = 0.0f;  			// position to reach in ADC counts
	float e = 0.0f; 				// current position error in ADC counts (e = AdcToGo - adc->GetValue();)
	float V = 0.0f;					// Current Velocity in steps/second
	float Vcmd = 0.0f;				// V from cmd
	float Vtoset = 0.0f;			// Velocity to aim

	bool PID_Enabled = false;

	bool progressiveAcceleration = false;
	float A;					// Acceleration in steps/second^2
	float Vmin;					// Minimum speed in steps/second
	float Vmax;					// Maximum speed in steps/second
	float stepsADCconvertCoef;  // (~24 for CV )coefficient to convert mms to ADC counts (empirically calculated)
	uint16_t brakePathMin;

	float brakePath = 0; // path to brake @A accel for current Commanded Speed Vcmd. (not for current speed)!
	ADC_Param* Adc;
};



#endif /* APP_STEPPER__PID_HPP_ */
