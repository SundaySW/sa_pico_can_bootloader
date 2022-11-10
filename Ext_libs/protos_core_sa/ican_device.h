#pragma once

namespace CAN
{
	struct Packet
	{
		enum { DATA_SIZE = 8 };
		
		Packet()
		: Dlc(0)
		{
			Id.Ext = 0;
		}
		
		union UID
		{
			unsigned long  Ext;
			unsigned short Std;
			unsigned char  Tab[4];
		}Id;
		unsigned char Dlc;
		unsigned char Data[DATA_SIZE];
	};
	
//    class IDevice
//    {
//    public:
//        virtual bool EnqueuePacket(const Packet& packet) { return false; }
//        virtual bool PollRXQueue(Packet& packet)         { return false; }
//    };
}
