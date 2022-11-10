#include "protos_msg.h"

namespace Protos
{
	Msg::Msg() : Dlc(0) { Id.Ext = 0;}
	
	Msg::Msg(unsigned char senderId, unsigned char destId)
		: Dlc(0)
	{
		Id.Tab[0] = 0;
		Id.Tab[1] = senderId;
		Id.Tab[2] = destId;
		Id.Tab[3] = 0;
	}
	
	Msg::Msg(unsigned long id, const char* data, int dlc)
	{
		Id.Ext = id;
		Dlc = dlc;
		for (int i = 0; i < Dlc; i++)
			Data[i] = data[i];
	}
	
	bool Msg::IsLong() const
	{
		return ((Id.Tab[3] & 0x03) != 0) ? 1 : 0;
	}
//TODO correct check
    bool Msg::IsBoot() const
    {
        return ((Id.Tab[3] & 0x18) == 0x18);
    }
//TODO correct check
	bool Msg::IsRaw() const
	{
		return ((Id.Tab[3] & 0x18) == 0x10);
	}
	
	bool Msg::IsLongFirst() const
	{
		return (Id.Tab[3] & 0x01);
	}
	
	bool Msg::IsLongNext() const
	{
		return (Id.Tab[3] & 0x02);
	}
	
	bool Msg::IsLongFlow() const
	{
		return (Id.Tab[3] & 0x03);
	}
	
	unsigned char Msg::GetDestID() const
	{
		return Id.Tab[2];
	}

	unsigned char Msg::GetMSGId() const
	{
		return Data[0];
	}
	unsigned char Msg::GetCMDId() const
	{
		return Data[0];
	}
	
	unsigned char Msg::GetMSGType() const
	{
		return ((Id.Tab[0] >> 4) & 0x0F);
	}
	
	unsigned char Msg::GetSenderID() const
	{
		return Id.Tab[1];
	}
	
	unsigned char Msg::GetLongIndex() const
	{
		return Id.Tab[0];
	}
	
	void Msg::NextLongIndex()
	{
		Id.Tab[0]++;
	}
	
	void Msg::SetDataBuffer(const unsigned char* buffer, int size)
	{
		for (int k = 0; k < size; k++)
		{
			Data[k] = buffer[k];
		}
		Dlc = size;
	}
	
	void Msg::SetLongFirst()
	{
		Id.Tab[3] = 0x01;
	}
	
	void Msg::SetLongNext()
	{
		Id.Tab[3] = 0x02;
	}
	
	void Msg::SetMSGId(unsigned char msgId)
	{
		Data[0] = msgId;
	}
	
	void Msg::SetMSGType(unsigned char msgType)
	{
		Id.Tab[0] = ((msgType << 4) & 0xF0);
	}
	
	void Msg::StartLongIndex()
	{
		Id.Tab[0] = 1;
	}
	
	char Msg::GetParamID() const
	{
		return Data[0];
	}

	char Msg::GetFieldID() const
	{
		return (Data[1]>>3);
	}

	char Msg::GetFieldAndType() const
	{
		return (Data[1]);
	}
		
	
}//namespace Protos
