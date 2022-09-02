#ifndef APP_AXIS_DEVICE_HPP_
#define APP_AXIS_DEVICE_HPP_

class MotorAxis;

class AxisDevice
{
public:
	virtual void OnAxisEvent(MotorAxis* axis, int event) = 0;
};


#endif /* APP_MOTOR_DEVICE_HPP_ */
