#ifndef APP_STEPPER_MAN_HPP_
#define APP_STEPPER_MAN_HPP_

#include "stepper_base.hpp"
#include "system_def.hpp"

struct MANStepperCfg
{
	bool progressiveAcceleration = false;
	float A;					// Acceleration in steps/second^2
	float Vmin;					// Minimum speed in steps/second
	float Vmax;					// Maximum speed in steps/second
	float stepsADCconvertCoef;  // (~24 for CV )coefficient to convert mms to ADC counts (empirically calculated)
};

class MANStepper : public BaseStepper
{
public:
	MANStepper(const StepDriverCfg& driver, TIM_HandleTypeDef* _htim, ADC_Param* adc)
		: BaseStepper(driver, _htim)
		, Adc(adc)
	{
	}

	void Init(const MANStepperCfg& cfg)
	{
		progressiveAcceleration = cfg.progressiveAcceleration;
		A = cfg.A;
		Vmin = cfg.Vmin;
		Vmax= cfg.Vmax;
		stepsADCconvertCoef = cfg.stepsADCconvertCoef;
	}

	int Control(int& event) override
	{
		event = EVENT_NULL;

		if (Vt == 0)
		{
			if (!IsMoving())
				return 0;
		}

		if (CurDir != CmdDir)
		{
			if (DecV() <= Vmin)
			{
				setDirection(CurDir = CmdDir);
				IncV();
			}
		}
		else
		{
			if (V <= Vt)
				IncV();
			else
				DecV();
		}

		if (HasStopCondition())
		{
			DoStop();
			event = EVENT_STOP;
		}
		else
			halfStep();

		return 0;
	}

	bool IsMoving() const {
		return V != 0.0;
	}

	void Stop()
	{
		Vt = 0;
		if (!IsMoving())
			disable();
	}

	bool Move2 (int cmdDir, int vt)
	{
		Vt = vt;
		if (Vt > Vmax)
			Vt = Vmax;

		CmdDir = (DIRECTION)cmdDir;

		if (V == 0.0)
		{
			V = Vmin;
			setDirection(CurDir = CmdDir);

			enable();
			halfStep();

			__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));
		}

		return true;
	}

	int GetVelocity() const override
	{
		return V;
	}

	void SetVelocity(int vt) override
	{
		Move2(CurDir, vt);
	}

	int GetBrakeSteps() const override
	{
		//if progressiveAcceleration
		// ...
		//else linear acceleration:
		int steps = ((V - Vmin) / 2 + Vmin) * ((V - Vmin)/A);
		return steps;
	}

	int GetCurDir() const
	{
		return CurDir;
	}

private:
	bool HasStopCondition() const {
		return Vt == 0.0 && V <= Vmin;
	}

	void DoStop(){
		disable();
		V = 0.0;
	}

	float IncV() {
		if (V >= Vmax) return V;
		V += A / V;
		__HAL_TIM_SET_AUTORELOAD(htim, (1000000/V));
		return V;
	}

	float DecV() {
		if (V <= Vmin) return V;
		V -= A / V;
		if (V < Vmin)
			V = Vmin;
		__HAL_TIM_SET_AUTORELOAD(htim, (1000000/V));
		return V;
	}

	float A = 1000.0f;			// step^2 / second
	float V = 0.0f;		 		// step / second
	float Vt = 0.0f;			// target Speed

	float Vmin = 100.0f;		// step / second
	float Vmax = 7000.0f;		// step / second

	DIRECTION CurDir = DIRECTION::FWD,
			  CmdDir = DIRECTION::UNDEF;

	ADC_Param* Adc = 0;

	bool progressiveAcceleration = false;
	float stepsADCconvertCoef = 1;
};



#endif /* APP_STEPPER_MAN_HPP_ */
