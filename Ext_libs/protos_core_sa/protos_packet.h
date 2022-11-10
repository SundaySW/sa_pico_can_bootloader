#ifndef PROTOS_PACKET_H_
#define PROTOS_PACKET_H_


#include "ican_device.h"

namespace Protos
{

	/*!
		Protos �����. �������� ��c�� ��� �����
		protos-���������.
	*/
	struct Packet
	{
		Packet()
		{
			Short();
		};

		Packet(const CAN::Packet& can)
		{
			TI.Byte  = can.Id.Tab[0];
			Src      = can.Id.Tab[1];
			Dst      = can.Id.Tab[2];
			ID.Byte  = can.Id.Tab[3];
			SetData((const char*)can.Data, can.Dlc);
		}

		bool Is(SL_FLAG sl) const
		{
			return (ID.Bit.PS == 0 &&
			        ID.Bit.SL == sl);
		}

		Packet& Short()
		{//make short packet
			ID.Bit.PS = 0;
			ID.Bit.SL = 0;
			return *this;
		}

		Packet& LongFirst(char pri, char src, char dst, MSG_TYPE type, unsigned short len)
		{// make long first packet
			ID.Bit.PS = 0;
			ID.Bit.SL = 1;
			TI.Bit.Type = (unsigned char)type;
			ID.Bit.Pri = pri;
			Src = src;
			Dst = dst;
			const char* p = (const char*)(&len);
			SetData(p, sizeof(len));
			return *this;
		};

		Packet& LongNext(unsigned char index, char pri, char src, char dst, const char* data, int len)
		{// make long next
			ID.Bit.Pri = pri;
			ID.Bit.PS  = 0;
			ID.Bit.SL  = 2;
			TI.Index   = index;
			Src = src;
			Dst = dst;
			SetData(data, len);
			return *this;
		};

		Packet& LongFC(FC_FLAG flag, unsigned char blockSize, unsigned char delay)
		{//make flow control
			ID.Bit.PS = 0;
			ID.Bit.Pri = 0;
			ID.Bit.SL = 3;
			char data[3] = { (char)flag, (char)blockSize, (char)delay };
			SetData(data, 3);
			return *this;
		};

		Packet& SetIndex(unsigned char index) { TI.Index = index; return *this; }
		Packet& SetPri(char pri) { ID.Bit.Pri = pri; return *this; }
		Packet& SetSrc(char src) { Src = src; return *this; }
		Packet& SetDst(char dst) { Dst = dst; return *this; }
		Packet& SetType(unsigned char type) { TI.Bit.Type = type; return *this; }
		Packet& SetData(const char* data, int len)
		{
			if (len > 8) return *this;
			Dlc = len;
			for (int i = 0; i < len; i++)
				Data[i] = data[i];
			return *this;
		}

		union {
			struct
			{
				unsigned char SL     : 2; // Short/Long sign
				unsigned char PS     : 1; // Protocol select: Protos/Raw
				unsigned char Pri    : 2; // Priority
				unsigned char Unused : 3; // Not used in CAN standard
			}
			Bit;
			unsigned char Byte = 0;
		} ID;

		unsigned char Src = 0;
		unsigned char Dst = 0;

		union {
			struct
			{
				unsigned char Reserved : 4;
				unsigned char Type     : 4;		// Message type
			}Bit;
			unsigned char Index = 0;			// Index in long message sequence
			unsigned char Byte;
		}TI;									

		unsigned char Dlc = 0;

		union
		{
			struct
			{
				//Buffer[0]...Buffer[7]
				union {
					unsigned char Byte0;
					FC_FLAG FCFlag;				///< FC flag in control flow packet
				};
				union {
					unsigned char Byte1;
					unsigned char BlockSize;	///< Block size in control flow packet
				};
				union {
					unsigned char Byte2;
					unsigned char Delay;		///< Delay in control flow packet
				};
				union {
					unsigned char Byte3;
				};
				union {
					unsigned char Byte4;
				};
				union {
					unsigned char Byte5;
				};
				union {
					unsigned char Byte6;
				};
				union {
					unsigned char Byte7;
				};
			}
			View;
			unsigned char Data[8];
		};
	};

	extern void Send(const Packet& packet);

}//namespace Protos

#endif /* PROTOS_PACKET_H_ */
