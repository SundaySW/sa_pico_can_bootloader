#ifndef APP_CAN_PORT_HPP_
#define APP_CAN_PORT_HPP_

#include <protos_boot_msg.h>
#include "stm32g4xx_hal.h"
#include "protos_msg.h"
#include "can_queue.hpp"

class CANPort
{
public:
	enum { RXQ_SIZE = 32, TXQ_SIZE = 16};

	CANPort (FDCAN_HandleTypeDef* can)
		: Can(can)
	{
		TxH.Identifier = 0x000;
		TxH.IdType = FDCAN_EXTENDED_ID;
		TxH.TxFrameType = FDCAN_DATA_FRAME;
		TxH.DataLength = FDCAN_DLC_BYTES_8;
		TxH.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
		TxH.BitRateSwitch = FDCAN_BRS_OFF;
		TxH.FDFormat = FDCAN_CLASSIC_CAN;
		TxH.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
		TxH.MessageMarker = 0;
	}

	inline int Read(Protos::Msg* msgs, int n = 1)
	{
		int i = 0;
		for (; i < n; i++)
		{
			if (!RXQ.Pop(msgs[i]))
				break;
		}
		return i;
	}

	inline int Trans()
	{
		auto msg = TXQ.Peek();
		if (!msg) return 0;

		uint8_t TxD[8] = {0};

		TxH.Identifier = msg->Id.Ext;
		TxH.DataLength = msg->Dlc <<16;
		for (uint8_t i = 0; i < msg->Dlc; i++)
			TxD[i] = msg->Data[i];

		if (HAL_FDCAN_GetTxFifoFreeLevel(Can) > 0)
		{
			HAL_FDCAN_AddMessageToTxFifoQ(Can, &TxH, TxD);
			TXQ.Pop();
			return 1;
		}
		return 0;
	}

	bool OnRX(FDCAN_RxHeaderTypeDef header, uint8_t* data){
		return RXQ.Push(header.Identifier, header.DataLength >> 16, data);
	}

	void Send(const Protos::Msg& msg){
		TXQ.Push(msg);
	}

	void SendRaw(const Protos::RawMsg& msg){
		TXQ.Push(msg);
	}

    void SendBoot(const Protos::BootMsg& msg){
        TXQ.Push(msg);
    }
private:
	FDCAN_TxHeaderTypeDef TxH;
	CANQueue<RXQ_SIZE> RXQ;
	CANQueue<TXQ_SIZE> TXQ;
	FDCAN_HandleTypeDef* Can;
};




#endif /* APP_CAN_PORT_HPP_ */
