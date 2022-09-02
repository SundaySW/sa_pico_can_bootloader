#ifndef APP_BASE_DEVICE_HPP_
#define APP_BASE_DEVICE_HPP_

#include "can_port.hpp"
#include "protos_msg.h"
#include "protos_device.h"

class BaseDevice : public Protos::Device
{
public:
	BaseDevice(DeviceUID::TYPE type, uint8_t family, uint8_t addr, FDCAN_HandleTypeDef* can)
		: Protos::Device(type, family, addr)
		, Port(can)
	{}

	inline void Poll()
	{
		PollPort();
		OnPoll();
	}

	bool OnCanRX(FDCAN_RxHeaderTypeDef header, uint8_t* data)
	{
		return Port.OnRX(header, data);
	}

protected:
	virtual inline void OnPoll(){}

	inline void PollPort()
	{
		Protos::Msg msg;
		if (Port.Read(&msg))
			GrabMessage(msg);

		Port.Trans();
	}

	void SendProtosMsgImpl(const Protos::Msg& msg) override {
		Port.Send(msg);
	}
	void SendRawMsgImpl(const Protos::RawMsg& msg) override {
		Port.SendRaw(msg);
	}
    void SendBootMsgImpl(const Protos::BootMsg& msg) override {
        Port.SendBoot(msg);
    }

	CANPort Port;
};



#endif /* APP_BASE_DEVICE_HPP_ */
