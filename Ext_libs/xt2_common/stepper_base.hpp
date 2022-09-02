#ifndef APP_BASE_STEPPER_HPP_
#define APP_BASE_STEPPER_HPP_

#include "adc_param.hpp"
#include "step_driver.hpp"
#include "stm32g4xx_hal.h"
#include "system_def.hpp"

enum Event { EVENT_NULL = 0,
	EVENT_STOP, // 	move stopped
	EVENT_CSS,  //	constant speed reached
	EVENT_CSE   //  constant speed end
};

class IStepper
{
public:
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual int halfStep() = 0;
	virtual void setDirection(int dir) = 0;
	virtual int Control(int& event) = 0;
	virtual bool Move(int pos, uint16_t vset, bool tomo = false) {};
	virtual bool Move2 (int cmdDir, int vt) {};
	virtual	void Stop() = 0;
	virtual bool IsMoving() const = 0;
	virtual int GetVelocity() const = 0;
	virtual void SetVelocity(int speed) = 0;
	virtual int GetBrakeSteps() const {return 0;};
	virtual int GetCurDir() const {return 0;};
};

class BaseStepper : public IStepper
{
public:
	BaseStepper(const StepDriverCfg& driver, TIM_HandleTypeDef* _htim)
		: Driver(driver)
		, htim(_htim)
	{
	}
	void enable() override
	{
		Driver.enable();
	}
	void disable() override
	{
		Driver.disable();
	}
	int halfStep() override
	{
		Driver.halfStep();
		if(Dir == FWD)
			return 1;
		else
			return -1;
	}
	void setDirection(int dir) override
	{
		if (dir == FWD)
			Driver.setDirection(0);
		else
			Driver.setDirection(1);
		Dir = dir;
	}

protected:
	StepDriver Driver;
	TIM_HandleTypeDef* htim;
	int Dir = -1;
};

#endif
