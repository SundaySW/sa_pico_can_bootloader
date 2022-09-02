
#ifndef APP_STEPPER_HPP_
#define APP_STEPPER_HPP_

#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "stepper_base.hpp"
#include "system_def.hpp"

struct DIRStepperCfg
{
	float A = 1000.0f;				// step^2 / second
	float Vmin = 100.0f;			// step / second
	float Vmax = 7000.0f;			// step / second
};

class DIRStepper : public BaseStepper
{
public:
	enum MODE
	{
		IDLE = 0,
		ACCEL = 1,
		CONST = 2,
		DECCEL = 3,
		EM_STOP = 4,
		START = 5,
		FINISH = 6,
		TOMO_HR = 7,
	};

	DIRStepper(const StepDriverCfg& driverCfg, TIM_HandleTypeDef* _htim)
		: BaseStepper(driverCfg, _htim)
	{
	}

	void Init(const DIRStepperCfg& cfg)
	{
		A = cfg.A;
		Vmin = cfg.Vmin;
		Vmax = cfg.Vmax;
	}

	int Control(int& event) override
	{
		event = EVENT_NULL;

		if (mode == IDLE)
			return 0;

		switch (mode)
		{
		case MODE::ACCEL:
		{
			if (V >= Vset)
			{
				V = Vset;
				if (tomoFlag)
					mode  = MODE::TOMO_HR;
				else
					mode  = MODE::CONST;
				event = EVENT_CSS;
				break;
			}

			if (accel_step >= stepsToGo / 2)
			{
				mode = MODE::DECCEL;
				break;
			}

			V += A / abs(V);
			accel_step++;

			int buf = (int) (1000000 / V);
			__HAL_TIM_SET_AUTORELOAD(htim, buf);
		}
		break;
		case MODE::CONST:
		{
			if (currentStep + accel_step >= stepsToGo)
			{
				mode  = MODE::DECCEL;
				event = EVENT_CSS;
				break;
			}

		}
		break;
		case MODE::TOMO_HR:
		{
			if (currentStep + accel_step >= stepsToGo)
			{
				tomoFlag = false;
				mode  = MODE::DECCEL;
				// ?? event = EVENT_CSS;
				break;
			}

			if (V < Vtomo)
			{
				V += A / abs(V);

			}
			else if (V > Vtomo)
			{
				V -= A / abs(V);
			}
		}
		break;
		case MODE::DECCEL:
		{
			if (accel_step <= 0)
			{
				event = EVENT_STOP;
				disable();
				mode = MODE::IDLE;

				break;
			}

			V -= A/abs(V);

			if (V < Vmin)
				V = Vmin;

			accel_step--;

			int buf = (int) (1000000 / V);
			__HAL_TIM_SET_AUTORELOAD(htim, buf);

		}
		break;
		case MODE::EM_STOP:
			//TODO: stepper em_stop
			//STOP TIMER
			//DISABLE DRIVER
		break;
		default:
			return 0;
		}

		if (mode == ACCEL || mode == CONST || mode == DECCEL || mode == TOMO_HR)
		{
			currentStep++;
			return halfStep();
		}
		return 0;
	}

	bool IsMoving() const override
	{
		return (mode != MODE::IDLE);
	}

	bool Move (int steps, uint16_t vset, bool tomo) override
	{
		if (mode != MODE::IDLE)
			return false;

		Vset = vset;
		if (Vset > Vmax)
			Vset = Vmax;

		if (steps > 0)
			setDirection(FWD);
		else
			setDirection(REV);

		stepsToGo = abs(steps);

		enable();
		V = Vmin;
		accel_step = 0;
		currentStep = 0;
		mode = MODE::ACCEL;

		halfStep();
		__HAL_TIM_SET_AUTORELOAD(htim, (1000000/Vmin));

		return true;
	}

	void Stop() override
	{
		//em stop
	}
	int GetVelocity() const override
	{
		return V;
	}
	void SetVelocity(int speed) override
	{
		Vtomo = speed;
	}

private:
	uint8_t mode = MODE::IDLE;
	int currentStep = 0;
	int stepsToGo = 100;
	int accel_step = 0;

	float A = 1000.0f;				// step^2 / second
	float V = 0.0f;		 			// step / second
	float Vset = 100.0f;
	float Vmin = 100.0f;			// step / second
	float Vmax = 7000.0f;			// step / second

	float Vtomo = 0.0f;
	bool tomoFlag = false;
};



#endif /* APP_STEPPER_HPP_ */
