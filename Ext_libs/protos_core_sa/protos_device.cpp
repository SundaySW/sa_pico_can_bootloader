
#include "protos_misc_cmd.h"
#include "protos_device.h"

#include <string.h>

namespace
{

	#define VERSION_MAJOR 1
	#define VERSION_MINOR 0

	enum
	{// ���������� ������� � RAW ���������
		MSG_DEVICE_ADDRESS		= 0x10, ///< ����������\��������� ����� ����������
		MSG_ADDRESS_DUPLICATE	= 0x13  ///< �� ����� ���������� � ����������� ��������
	};

	enum
	{// ��������� � ������ ���������
		MSG_DEVICE_ADDRESS_ANS		 = (MSG_DEVICE_ADDRESS    << 2) | Protos::RawMsg::ATTR_ANSWER,
		MSG_DEVICE_ADDRESS_ERR		 = (MSG_DEVICE_ADDRESS    << 2) | Protos::RawMsg::ATTR_ERROR,
		MSG_ADDRESS_DUPLICATE_ERROR  = (MSG_ADDRESS_DUPLICATE << 2) | Protos::RawMsg::ATTR_ERROR
	};

} //namesapce

namespace Protos
{

	Device::Device(DeviceUID::TYPE type, uint8_t family, uint8_t addr)
	: Family(family)
	, AddressAssignTimout(0)
	, AddressAssignTag(false)
	, Uid(type)
	, Address(0)
	//, Can(0)
	{
		/* Configure non-volatile memory */
		//NVMemory::Start(NVM_WAITSTATE3);
		/* Load protos device address */
//		Address = LoadAddress();
		Address = addr;
	}

	void Device::AssignAddress(char address)
	{
		Address = address;
		//EEPROM eeprom;
		//eeprom.Write(0, &address, sizeof(address));
	}

	void Device::GrabMessage(const Protos::Msg& msg)
	{
		if (msg.IsRaw())
		{
			Protos::RawMsg rawMsg(msg);
			ProcessRAWMessage(rawMsg);
		}
		else if(msg.IsBoot()){
            Protos::BootMsg bootMsg(msg);
            ProcessBootMessage(bootMsg);
		}
		else
		{
			char dst = msg.GetDestID();
			if (dst != Address && dst != Protos::Msg::BROADCAST)
			return;
			
			/* resolve address collisions */
			if ((msg.GetSenderID() == Address) && (Address != 0))
			return;// ProcessSameAddressOnBus();
			
			/* process common(misc) commands */
			if (msg.GetMSGType() == MSGTYPE_CMDMISC)
			{
				switch (msg.GetCMDId())
				{
					case CMDMISC_GET_FIRMWARE_VERSION:
					{
						char data[] = { CMDMISC_GET_FIRMWARE_VERSION, VERSION_MAJOR, VERSION_MINOR };
						SendProtosMsg(msg.GetSenderID(), MSGTYPE_CMDMISC_ANSWER, data, sizeof(data));
					}
					break;
					//case CMDMISC_GET_ALL_PARAMS:
					//{
						//SendAllParametrID(msg.GetDestID());
						////char data[] = { CMDMISC_GET_FIRMWARE_VERSION, VERSION_MAJOR, VERSION_MINOR };
						////SendProtosMsg(msg.GetSenderID(), MSGTYPE_CMDMISC_ANSWER, data, sizeof(data));
					//}
					//break;
				}
			}

			else
			{// process other messages
				ProcessMessage(msg);
			}
		}
	}

	uint8_t Device::LoadAddress()
	{
		char addr = 0;
		//EEPROM eeprom;
	//	eeprom.Read(0, &addr, 1);
	//	if (addr == 0xFF) addr = 0;
		addr = 0xBF;
		return addr;
	}

	inline bool Device::ProcessRAWMessage(const Protos::RawMsg& msg)
	{
		switch (msg.GetId())
		{
			case MSG_DEVICE_ADDRESS:
			{
				if (msg.IsAnswer())
				{
					const char otherDeviceAddr = msg.Data.Bytes[1];
					if (Address == otherDeviceAddr && otherDeviceAddr != 0)
					{
						ProcessSameAddressOnBus();
					}
				}
				else if (msg.IsRequest())
				{
					bool answer = false;
					if (msg.Data.Bytes[1] == 0xFF)
					answer = (msg.Data.Bytes[2] == 0 /*any family*/ || msg.Data.Bytes[2] == Family);
					else
					answer = (msg.Data.Bytes[1] == Address);
					
					if (!answer) break;

					char data[] = { MSG_DEVICE_ADDRESS_ANS, Address, Family, Uid.Type, 0, 0, 0, 0 };
					memcpy(&data[4], Uid.Data.I1, sizeof(uint32_t));
					SendRawMsg(data, sizeof(data));
				}
				else if (msg.IsSet())
				{
					uint32_t uid;
					memcpy(&uid, msg.Data.Bytes + 3, sizeof(uint32_t));
					
					//������� ���� UID, �������������� ������ ����.
					if (Uid.MatchType(msg.Data.Bytes[2]) &&  Uid.MatchData(uid)) // ��� ������� ������???
					{
						AssignAddress(msg.Data.Bytes[1]);
						AddressAssignTimout = 0;
						
						if (Address)
						{
							// wait (1sec) confirmation that I can(address not busy) accept new address
							AddressAssignTimout = 1000;
							AddressAssignTag    = false;
						}

						// notify other devices about my new address
						char data[] = { MSG_DEVICE_ADDRESS_ANS, Address, AddressAssignTag };
						SendRawMsg(data, sizeof(data));
					}
				}
			}
			break;
			case MSG_ADDRESS_DUPLICATE:
			{
				const char otherDeviceAddr = msg.Data.Bytes[1];

				if ((Address == otherDeviceAddr) && (otherDeviceAddr != 0))
				{
					if (AddressAssignTag == false)
					{
						// ��������� � ���, ��� � �� ���� "������������"
						char data[] = { MSG_DEVICE_ADDRESS_ERR, Address, AddressAssignTag,
						Uid.Data.I1[0], Uid.Data.I1[1], Uid.Data.I1[2], Uid.Data.I1[3] };
						
						SendRawMsg(data, sizeof(data));
						
						Address = 0;
						AddressAssignTimout = 0;
					}
					else
					{// �� ����� ��������� ��� (� �����) ���������� � ����������� ��������
						// � AddressAssignTag == true
						const char otherDeviceAssignTag = msg.Data.Bytes[2];
						if (otherDeviceAssignTag)
						{
							Address = 0;
							char data[] =
							{
								MSG_ADDRESS_DUPLICATE_ERROR,
								0, /* ����� �� ���� ������������ ����� ��������� */
								AddressAssignTag,
								Uid.Data.I1[0], Uid.Data.I1[1], Uid.Data.I1[2], Uid.Data.I1[3]
							};
							SendRawMsg(data, sizeof(data));
						}
					}
				}
			}
			break;
		}
		return true;
	}

	void Device::ProcessSameAddressOnBus()
	{
		if (AddressAssignTag == false)
		{
			char otherDeviceAddr = Address;
			AddressAssignTimout = 0;
			Address = 0;
			// ��������� � ���, ��� � �� ���� "������������"
			char data[] = { MSG_DEVICE_ADDRESS_ERR, otherDeviceAddr, AddressAssignTag,
			Uid.Data.I1[0], Uid.Data.I1[1], Uid.Data.I1[2], Uid.Data.I1[3] };
			SendRawMsg(data, sizeof(data));
		}
		else
		{
			// ���������, ��� � ��� "�����" ����� �����
			char data[] = { MSG_ADDRESS_DUPLICATE_ERROR, Address, AddressAssignTag,
			Uid.Data.I1[0], Uid.Data.I1[1], Uid.Data.I1[2], Uid.Data.I1[3] };
			SendRawMsg(data, sizeof(data));
		}
	}

//	bool Device::ReadUID(I2CMaster& i2c, uint8_t i2cSlaveAddress)
//	{
//		uint8_t writeData[] = { 0x7F, 0xFC };
//		uint8_t r[4];
//		bool ok;
//		if ((ok = i2c.WriteReadSync(i2cSlaveAddress, writeData, 2, r, 4)))
//		{
//			memcpy(&Uid.Data.I4, r, sizeof(uint32_t));
//		}
//		return ok;
//	}

	void Device::OnTimerINT(int ms)
	{
		if (AddressAssignTimout > 0)
		{// wait confirmation(not busy by other devices) for new address
			AddressAssignTimout -= ms;
			if (AddressAssignTimout <= 0)
			{
				AddressAssignTag = true;
			}
		}
		OnTimer(ms);
	}

	void Device::SendProtosMsg(char dest, char msgType, const char* data, char dlc)
	{
		Protos::Msg msg;
		msg.Id.Tab[3] = 0x00;
		msg.Id.Tab[2] = dest;
		msg.Id.Tab[1] = Address;
		msg.Id.Tab[0] = (msgType << 4);
		msg.Dlc = dlc;
		for (unsigned char k = 0; k < dlc; k++)
		{
			msg.Data[k] = data[k];
		}
		SendProtosMsgImpl(msg);
		//CanTXQueue.Push(msg);
	}

	void Device::SendBootMsg(char msgType, const char* data, uint32_t uid, char dlc)
	{
		Protos::BootMsg msg;
		msg.ExtId.bit.MsgType = msgType;
		msg.ExtId.Tab[2] = (uid >> 16);
		msg.ExtId.Tab[1] = (uid >> 8);
		msg.ExtId.Tab[0] = uid;
		msg.Dlc = dlc;
		for (unsigned char k = 0; k < dlc; k++)
		{
			msg.Data[k] = data[k];
		}
        SendBootMsgImpl(msg);
		//CanTXQueue.Push(msg);
	}

	void Device::SendRawMsg(const char* data, int dlc)
	{
		Protos::RawMsg rmsg;
		rmsg.Ext.bit.UidType  = Uid.Type;
		rmsg.Ext.bit.Protocol = 1;
		rmsg.Ext.bit.UidByte0 = Uid.Data.I1[1];
		rmsg.Ext.bit.UidByte1 = Uid.Data.I1[2];
		rmsg.Ext.bit.UidByte2 = Uid.Data.I1[3];
		rmsg.Dlc = dlc;
		for (int i = 0; i < dlc; i++)
		{
			rmsg.Data.Bytes[i] = data[i];
		}
		SendRawMsgImpl(rmsg);
		//CanTXQueue.Push(rmsg);
	}

	void Device::StartAddressValidation()
	{
		AddressAssignTag	= false;
		AddressAssignTimout = 1000;
		const char data[] = { MSG_DEVICE_ADDRESS_ANS, Address, AddressAssignTag };
		SendRawMsg(data, sizeof(data));
	}

}//namespace Protos
