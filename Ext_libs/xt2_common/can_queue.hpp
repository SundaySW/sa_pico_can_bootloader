//SA 30.08.2020
#ifndef SRC_CAN_QUEUES_H_
#define SRC_CAN_QUEUES_H_

#include "protos_msg.h"
#include "protos_raw_msg.h"
//#include "stm32g4xx_hal.h"

struct CANPack
{
	enum { DATA_Size = 8 };
	CANPack() : Dlc(0) { Id.Ext = 0; }

	union UID
	{
		unsigned long  Ext;
		unsigned short Std;
		unsigned char  Tab[4];
	}Id;
	unsigned char Dlc;
	unsigned char Data[DATA_Size];
};

template <int Capacity>
class CANQueue
{
public:
	inline bool IsEmpty() const
	{
		return Count == 0;
	}

	bool IsFull() const
	{
		return Count == Capacity;
	}

//	bool Pop(FDCAN_RxHeaderTypeDef header, uint8_t* data)
//	{
//		if (IsEmpty())
//			return false;
//
//		header.Identifier = Array[getIndex].Id.Ext;
//		header.DataLength = Array[getIndex].Dlc << 16;
//		for (uint8_t i = 0; i < Array[getIndex].Dlc; i++)
//			{
//				data[i] = Array[getIndex].Data[i];
//			}
//
//		OnPop();
//		return true;
//	}

	inline const CANPack* Peek() const
	{
		if (IsEmpty())
			return 0;

		return &Array[getIndex];
	}

	bool Pop()
	{
		if (IsEmpty())
			return false;

		OnPop();
		return true;

	}

	inline bool Pop(Protos::Msg& msg)
	{
		if (IsEmpty())
			return false;

		msg.Id.Ext = Array[getIndex].Id.Ext;
		msg.Dlc = Array[getIndex].Dlc;
		for (uint8_t i = 0; i < Array[getIndex].Dlc; i++)
			{
				msg.Data[i] = Array[getIndex].Data[i];
			}

		OnPop();
		return true;
	}



//	bool Push(FDCAN_RxHeaderTypeDef header, uint8_t* data)
//	{
//		if (IsFull())
//			return false;
//
//		Array[putIndex].Id.Ext = header.Identifier;
//		Array[putIndex].Dlc = header.DataLength >> 16;
//		for (uint8_t i = 0; i < Array[putIndex].Dlc; i++)
//			Array[putIndex].Data[i] = data[i];
//
//		OnPush();
//		return true;
//	}

	bool Push(uint32_t id, uint8_t dlc, uint8_t* data)
	{
		if (IsFull())
			return false;
		Array[putIndex].Id.Ext = id;
		Array[putIndex].Dlc = dlc;
		for (uint8_t i = 0; i < dlc; i++)
			Array[putIndex].Data[i] = data[i];

		OnPush();
		return true;
	}

	bool Push(Protos::Msg msg)
	{
		if (IsFull())
			return false;
		Array[putIndex].Id.Ext = msg.Id.Ext;
		Array[putIndex].Dlc = msg.Dlc;
		for (uint8_t i = 0; i < msg.Dlc; i++)
			Array[putIndex].Data[i] = msg.Data[i];

		OnPush();
		return true;
	}

	bool Push(Protos::RawMsg rmsg)
	{
		if (IsFull())
			return false;
		Array[putIndex].Id.Ext = rmsg.Ext.Bytes;
		Array[putIndex].Dlc = rmsg.Dlc;
			for (uint8_t i = 0; i < Array[putIndex].Dlc; i++)
				Array[putIndex].Data[i] = rmsg.Data.Bytes[i];
		OnPush();
		return true;
	}

    bool Push(Protos::BootMsg bootMsg)
    {
        if (IsFull())
            return false;
        Array[putIndex].Id.Ext = bootMsg.ExtId.Bytes;
        Array[putIndex].Dlc = bootMsg.Dlc;
        for (uint8_t i = 0; i < Array[putIndex].Dlc; i++)
            Array[putIndex].Data[i] = bootMsg.Data[i];
        OnPush();
        return true;
    }

private:
	void OnPush()
	{
		if (++putIndex == Capacity)
			putIndex = 0;
		Count++;
	}
	inline void OnPop()
	{
		if (++getIndex == Capacity)
			getIndex = 0;
		Count--;
	}

	CANPack Array[Capacity];
	uint8_t putIndex = 0;
	uint8_t getIndex = 0;
	uint8_t Count = 0;
};

#endif /* SRC_CAN_QUEUES_H_ */
