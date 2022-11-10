#ifndef PROTOS_RAW_MSG_H_
#define PROTOS_RAW_MSG_H_

#include "ican_device.h"

namespace Protos
{

	/*
		��������� ����������� � RAW-���������.
	*/
	class RawMsg
	{
	public:
		enum
		{// �������� ���������
			ATTR_REQUEST = 0,
			ATTR_ANSWER  = 1,
			ATTR_SET     = 2,
			ATTR_ERROR   = 3,
		};

		RawMsg()
		{
			Ext.Bytes = 0;
			Dlc = 0;
		}
		RawMsg(const CAN::Packet& packet)
		{
			Dlc = packet.Dlc;
			Ext.Bytes = packet.Id.Ext;
			for (int i = 0; i < Dlc; i++)
				Data.Bytes[i] = packet.Data[i];
		}
		
		RawMsg(const Protos::Msg& msg)
		{
			Dlc = msg.Dlc;
			Ext.Bytes = msg.Id.Ext;
			for (int i = 0; i < Dlc; i++)
			Data.Bytes[i] = msg.Data[i];
		}

		char GetId()	 const { return Data.Cmd.bit.Id; }
		bool IsRequest() const { return (Data.Cmd.bit.Attr == ATTR_REQUEST); }
		bool IsAnswer()  const { return (Data.Cmd.bit.Attr == ATTR_ANSWER);  }
		bool IsSet()	 const { return (Data.Cmd.bit.Attr == ATTR_SET);	 }
		bool IsError()   const { return (Data.Cmd.bit.Attr == ATTR_ERROR);	 }

		union
		{
			struct
			{
				unsigned long UidByte2  : 8;  ///< 24-bit unique identifier
				unsigned long UidByte1	: 8;
				unsigned long UidByte0	: 8;
				unsigned long UidType	: 3;  ///< type of unique identifier
				unsigned long Reserved	: 1;
				unsigned long Protocol	: 1;  ///< Protos or RAW-protocol
			} bit;
			unsigned long Bytes;
		}Ext;

		unsigned char Dlc;
		
		union
		{
			union
			{
				struct
				{
					unsigned char Attr  : 2;   ///< command attribute
					unsigned char Id    : 6;   ///< command id
				}bit;
			}
			Cmd;
			unsigned char Bytes[8];
		}
		Data;
	};

}//namespace Protos


#endif /* PROTOS_RAW_MSG_H_ */
