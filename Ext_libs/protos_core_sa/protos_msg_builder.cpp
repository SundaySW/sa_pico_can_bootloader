#include "protos_msg_builder.h"
#include "protos_packet.h"

namespace 
{
	bool AppendData(Protos::Msg2& msg, const unsigned char* data, unsigned char len)
	{
		if (msg.Dlc + len > Protos::Msg2::BUFFER_SIZE)
			return false;
		
		for (unsigned char i = 0; i < len; i++)
			msg.Data[msg.Dlc + i] = data[i];
		
		msg.Dlc += len;
		return true;
	}

}//namespace

namespace Protos
{

	MsgBuilder::MsgBuilder()
		: Long(false)
		, Ready(false)
	{
	}

	bool MsgBuilder::Grab(const Packet& packet)
	{
		if (packet.Is(SL_SHORT))
		{
			// Discard long message if started.
			if (Long)
				Long = false; 
				
			Msg.Dlc  = 0;
			Msg.Pri  = packet.ID.Bit.Pri;
			Msg.Src  = packet.Src;
			Msg.Dst  = packet.Dst;
			Msg.Type = (MSG_TYPE)packet.TI.Bit.Type;
		
			AppendData(Msg, packet.Data, packet.Dlc);
			Ready = true;
		}
		else
		{
			if (packet.Is(SL_LONG_FIRST))
			{
				Long = true;
				Msg.Pri     = packet.ID.Bit.Pri;
				Msg.Src     = packet.Src;
				Msg.Dst     = packet.Dst;
				Msg.Type    = (MSG_TYPE)packet.TI.Bit.Type;
				Msg.Dlc     = 0;
				Index       = 1;
				LenBytes[0] = packet.Data[0];
				LenBytes[1] = packet.Data[1];
				Ready       = false;
				if (Len == 0)
				{
					Ready = true;
					Long  = false;
				}
				if (Len > Msg2::BUFFER_SIZE)
				{
					Long  = false;
					Ready = false;
					return false;
				}
				// send flow control accept 
				Packet ans;
				ans.LongFC(FC_ACCEPT, 0xFF, 0).SetSrc(packet.Dst).SetDst(packet.Src);

				Send(ans);
			}
			else if (packet.Is(SL_LONG_NEXT))
			{
				if (!Long || Ready)
					return false;

				if (packet.TI.Index != Index || packet.Dlc > Len)
				{
					Long = false;
					return false;
				}

				AppendData(Msg, packet.Data, packet.Dlc);
				Len -= packet.Dlc;
				Index++;
					
				if (Len == 0)
				{
					Long = false;
					Ready = true;
				}
			}
		}
		return true;
	}

	bool MsgBuilder::IsReady() const
	{
		return Ready;
	}

}//namespace protos