//SA 30.03.21
#ifndef APP_AXIS_MOTOR_HPP_
#define APP_AXIS_MOTOR_HPP_

#include "stm32g4xx_hal.h"
#include "axis_base.hpp"
#include "brake.hpp"
#include "stepper_dir.hpp"
#include "stepper_pid.hpp"
#include "stepper_man.hpp"
#include "system_def.hpp"
#include "variant.h"
#include "math.h"

struct AxisMotorCfg
{
	PIDStepperCfg PIDCfg;
	DIRStepperCfg DIRCfg;
	MANStepperCfg MANCfg;
};

struct DelayedMove
{
	int Delay = 0;
	bool SyncPending = false;
	int steps = 0;
	int speed = 0;
};

class MotorAxis : public BaseAxis
{
public:
	void (*SignalEvent)(MotorAxis*, int event);

	MotorAxis(const PortPin& brake, const CalibConfig& calib, ADC_Param* adc,
			  const StepDriverCfg& driver, TIM_HandleTypeDef* _htim, AxisDevice* host)
		: BaseAxis(brake, calib, adc)
		, StepperPID(driver, _htim, adc)
		, StepperDIR(driver, _htim)
		, StepperMAN(driver, _htim, adc)
		, Host(host)
	{
		Stepper = &StepperPID;
	}

	MotorAxis(const CalibConfig& calib, ADC_Param* adc,
			  const StepDriverCfg& driver, TIM_HandleTypeDef* _htim, AxisDevice* host)
		: BaseAxis(calib, adc)
		, StepperPID(driver, _htim, adc)
		, StepperDIR(driver, _htim)
		, StepperMAN(driver, _htim, adc)
		, Host(host)
	{
		Stepper = &StepperPID;
	}

	void Init(const AxisMotorCfg& cfg)
	{
		BaseAxis::Init();

		StepperPID.Init(cfg.PIDCfg);
		StepperDIR.Init(cfg.DIRCfg);
		StepperMAN.Init(cfg.MANCfg);

		LockBrake();
		if (BrakeType == BRAKE_TYPE::EM){
			Stepper->disable();
		}
	}

	void Control()
	{
		if (Stepper)
		{
			controlLimits();

			int event = EVENT_NULL;

			StepCounter += Stepper->Control(event);
			if (event == EVENT_STOP)
			{
				LockBrake();
				Host->OnAxisEvent(this, EVENT_STOP);
			}
			else
			{
				if (event != EVENT_NULL)
					Host->OnAxisEvent(this, event);
			}
		}
	}

	int controlLimits()
	{
		if (!checkLimit(REV) || !checkLimit(FWD))
			Stepper->Stop();
	}

	bool MoveToPos(short pos, uint16_t speed)
	{
		Stopping = false;
		short adcToGo = ConvertPosToAdc(pos);

		if (!Calib.CheckAdcInRange(adcToGo))
			return false;

		Stepper = &StepperPID;
		Stepper->Move(adcToGo, speed);
		if (BrakeType != STEP_DRIVER)
		{
			ReleaseBrake(TIMEOUT_INFINITE);
		}

		return true;
	}

	bool MakeSteps(int steps, uint16_t speed, bool tomo = false)
	{
		Stopping = false;
		//TODO: check out of range!!!!!!!!!!!!
		Stepper = &StepperDIR;
		Stepper->Move(steps, speed, tomo);
		if (BrakeType != STEP_DRIVER)
		{
			ReleaseBrake(TIMEOUT_INFINITE);
		}
		return true;
	}

	bool ManualMove(int dir, int speed)
	{
		if (!checkLimit(dir))
			return false;

		Stopping = false;
		Stepper = &StepperMAN;
		Stepper->Move2(dir, speed);
		if (BrakeType != STEP_DRIVER)
		{
			ReleaseBrake(TIMEOUT_INFINITE);
		}

		return true;
	}

	void PrepareMove(int steps, int speed, int delay)
	{
		DM.Delay = delay;
		DM.SyncPending = true;
		DM.steps = steps;
		DM.speed = speed;
	}

	void OnTimer (int ms)
	{
		BaseAxis::OnTimer(ms);

		if (DM.Delay > 0 && !DM.SyncPending)
		{
			DM.Delay -= ms;
			if (DM.Delay <= 0)
			{
				MakeSteps(DM.steps, DM.speed, true);
			}
		}
	}

	void OnSyncCmd ()
	{
		DM.SyncPending = false;
		if (DM.Delay == 0){
			MakeSteps(DM.steps, DM.speed, true);
		}
	}

	void Stop()
	{
		if (!Stopping) {
			Stepper->Stop();
			Stopping = true;
		}
	}

	void ReleaseBrake(int timeout)
	{
		if (BrakeType == EM)
			Brake.Release(timeout);
		else if (BrakeType == STEP_DRIVER)
		{
			Stepper->disable();
		}
	}

	void LockBrake()
	{
		if (BrakeType == EM)
			Brake.Lock();
		else if (BrakeType == STEP_DRIVER)
		{
			Stepper->enable();
		}
	}

	bool IsMoving() const
	{
		return (Stepper->IsMoving());
	}

	int GetVelocity() const
	{
		return Stepper->GetVelocity();
	}

	void setVelocity(int speed)
	{
		Stepper->SetVelocity(speed);
	}

	int StepCounter = 0;

private:
	bool checkLimit(int dir) const
	{
		int adc = Adc->GetValue();
		int brakeAdc = Stepper->GetBrakeSteps() / Calib.stepsToADC;

		if (dir == REV){
			if ((adc < Calib.adcMin + brakeAdc) && (Stepper->GetCurDir() == REV))
				return false;
		}
		else  if (dir == FWD){
			if ((adc > Calib.adcMax - brakeAdc) && (Stepper->GetCurDir() == FWD))
				return false;
		}

		return true;
	}

	bool Stopping = false;
	IStepper* Stepper = 0;
	PIDStepper StepperPID;
	DIRStepper StepperDIR;
	MANStepper StepperMAN;
	DelayedMove DM;
	AxisDevice* Host = 0;
};



#endif /* APP_AXIS_MOTOR_HPP_ */
