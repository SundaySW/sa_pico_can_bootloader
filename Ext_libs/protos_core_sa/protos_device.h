#ifndef PROTOS_ADDR_MASTER_H_
#define PROTOS_ADDR_MASTER_H_

#include "device_uid.h"
#include "protos_msg.h"
#include "protos_raw_msg.h"
#include "protos_boot_msg.h"

namespace Protos
{

	class Device
	{
	public:
		static uint8_t LoadAddress();

		Device(DeviceUID::TYPE type, uint8_t family, uint8_t addr);
		//virtual ~Device(){}
		void GrabMessage(const Protos::Msg& msg);
		void OnTimerINT(int ms);
		void SendProtosMsg(char dest, char msgType, const char* data, char dlc);
		void SendBootMsg(char msgType, const char* data, uint32_t uid, char dlc);
	protected:
		//bool ReadUID(I2CMaster& i2c, uint8_t i2cSlaveAddress);
		void StartAddressValidation();
		virtual void OnTimer(int) {};
		virtual void ProcessMessage(const Protos::Msg&) {};
		virtual void SendProtosMsgImpl(const Msg& msg) {};
		virtual void SendRawMsgImpl(const RawMsg& msg) {};
        virtual void SendBootMsgImpl(const BootMsg& msg) {};
        virtual void ProcessBootMessage(const Protos::BootMsg& msg) {};
	
		inline bool ProcessRAWMessage(const Protos::RawMsg& msg);
        void ProcessSameAddressOnBus();
		void SendRawMsg(const char* data, int dlc);
		void AssignAddress(char address);

		uint8_t Family;
		int AddressAssignTimout;
		bool AddressAssignTag;

		DeviceUID Uid;
		uint8_t	Address;
	};

}//namespace Protos


#endif /* PROTOS_ADDR_MASTER_H_ */
