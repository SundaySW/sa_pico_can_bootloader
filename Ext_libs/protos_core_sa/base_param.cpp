#include "base_param.h"
#include "protos_misc_cmd.h"

using namespace Protos;
extern bool SendMsg(char dest, char msgType, const char *data, char dlc);
//extern bool SendMsg(char dest, char msgType, char data0, char data1);

#define MIN_UPDATERATE 5 //ms
#define MIN_SENDRATE  19 //ms

enum PGF_returns
{
	Ret_PGF_error,
	Ret_PGF_char,
	Ret_PGF_int,
	Ret_PGF_long,
	Ret_PGF_float,
};

void BaseParam::Close()
{
	TypeAttr = 0;
	SendRate = SendTime = 0;
	Id = 0;
}

bool BaseParam::GetField(char field, Variant& value)
{
	switch (field)
	{
	case Protos::FIELD_TYPE:
		value = Variant((char)GetValueType());
	break;
	case Protos::FIELD_VALUE:
	{
		if (!IsValid) return false;
		GetValue(value);
	}
	break;
	case Protos::FIELD_SENDRATE:
		value = Variant(GetSendRate());
	break;
	case Protos::FIELD_READONLY:
		value = Variant(IsReadOnly() ? (char)1 : (char)0);
	break;
	case Protos::FIELD_UPDATERATE:
	{
		UpdateParam* p = 0;
		if (QueryInterface(IID_UPDATABLE, (void*&)p))
		{
			value = Variant(p->GetUpdateRate());
		}
	}
	break;
	case Protos::FILED_OFFSET:
	case Protos::FIELD_MULT:
	{
		CalibrParam* p = 0;
		if (QueryInterface(IID_CALIBRATEABLE, (void*&)p))
		{
			if (field == Protos::FIELD_MULT)
				value = Variant(p->GetMult());
			else
				value = Variant(p->GetOffset());
		}
	}
	break;
	//case Protos::FIELD_ID_EXT:
	//	value = Variant(GetIdExt());
	//break;
	};
	return true;
}

void BaseParam::OnTimer(int ms)
{
	if (SendTime > -SendRate)
	{
		SendTime -= ms;
	}

	UpdateParam* pu = 0;
	if (QueryInterface(IID_UPDATABLE, (void*&)pu))
	{
		int updateTime = pu->GetUpdateTime();
		if (updateTime > -pu->GetUpdateRate())
		{
			pu->SetUpdateTime(updateTime - ms);
		}
	}
}

/*!
	��������� ���������� � �������� ���������.
	��� ������, ���������� � �������� ����� ���������. 
*/
void BaseParam::Poll()
{
	if ((SendTime < 0) && (SendRate > MIN_SENDRATE))
	{
		SendTime += SendRate;
		SendValue();
	}

	UpdateParam* pu = 0;
	if (QueryInterface(IID_UPDATABLE, (void*&)pu))
	{
		const int updateTime = pu->GetUpdateTime();
		if ((updateTime < 0) && (pu->GetUpdateRate() > MIN_UPDATERATE))
		{
			pu->SetUpdateTime(updateTime + pu->GetUpdateRate());
			UpdateValue();
		}
	}
}

//bool BaseParam::ProcessMessage(const Protos::Msg2& msg)
//{
//	if (msg.Type != MSGTYPE_PARAM_REQUEST && msg.Type != MSGTYPE_PARAM_SET)
//		return false;
//
//	if (!MatchId(msg.View.ParamID))
//		return false;
//
//	switch (msg.Type)
//	{
//		case MSGTYPE_PARAM_REQUEST:
//		{
//			Variant v;
//			if (!GetField(msg.View.Bits.FieldID, v))
//			return true;
//
//			char buffer[8];
//			buffer[0] = msg.View.ParamID;
//
//			if (v)
//			{
//				buffer[1] = msg.View.FieldAndType;
//				int size  = v.Get(buffer + 2, 6);
//				if (size)
//					SendMsg(msg.Src, MSGTYPE_PARAM_ANSWER, buffer, 2 + size);
//			}
//			else
//			{
//				buffer[1] = msg.View.FieldAndType;
//				buffer[2] = v.GetType();
//				SendMsg(msg.Src, MSGTYPE_PARAM_ERROR, buffer, 3);
//			}
//		}
//		return true;
//		case MSGTYPE_PARAM_SET:
//		{
//			char fieldAndType = msg.View.FieldAndType;
//			char err = SetField(fieldAndType, (const char*)(msg.Data + 2)); //TODO: type cast
//			if (err != RET_PSF_OK)
//			{
//				char buffer[3]={ msg.View.ParamID, msg.View.FieldAndType, err };
//				SendMsg(msg.Src, MSGTYPE_PARAM_ERROR, buffer, 3);
//			}
//		}
//		return true;
//	}
//	return false;
//}

bool BaseParam::ProcessMessage(const Protos::Msg& msg)
{
	unsigned char msgType = msg.GetMSGType();
	if (msgType != MSGTYPE_PARAM_REQUEST && msgType != MSGTYPE_PARAM_SET)
		return false;

	if (!MatchId(msg.GetParamID()))
		return false;

	switch (msgType)
	{
	case MSGTYPE_PARAM_REQUEST:
	{
		Variant v;
		if (!GetField(msg.GetFieldID(), v))
			return true;

		char buffer[8];
		buffer[0] = msg.GetParamID();
			
		if (v)
		{
			buffer[1] = ((msg.GetFieldID() << 3) | v.GetType());
			int size  = v.Get(buffer + 2, 6);
			if (size)
				SendMsg(msg.GetSenderID(), MSGTYPE_PARAM_ANSWER, buffer, 2 + size);
		}
		else
		{
			buffer[1] = msg.GetFieldAndType();
			buffer[2] = v.GetType();
			SendMsg(msg.GetSenderID(), MSGTYPE_PARAM_ERROR, buffer, 3);
		}
	}
	return true;
	case MSGTYPE_PARAM_SET:
	{
		char fieldAndType = msg.GetFieldAndType();
		char err = SetField(fieldAndType, (const char*)(msg.Data + 2)); //TODO: type cast
		if (err != RET_PSF_OK)
		{
			char buffer[3]={ msg.GetParamID(), msg.GetFieldAndType(), err };
			SendMsg(msg.GetSenderID(), MSGTYPE_PARAM_ERROR, buffer, 3);
		}
	}
	return true;
	}
	return false;
}

bool BaseParam::SendValue(char destId)
{
	if (!IsValid) return false;

	char buffer[6];
	buffer[0] = GetId();
	VALUE_TYPE type = GetValueType();
	buffer[1] = (FIELD_VALUE << 3) | type;
	
	Variant value;
	GetValue(value);
	
	int size = value.Get(buffer + 2, 6);

	bool ok = false;
	if (size)
	{
		ok = SendMsg(destId, MSGTYPE_PARAM_ANSWER, buffer, (size + 2));
	}
	return ok;
}

void BaseParam::SendIDandType(char destId)
{
	if (Id)
	{
		char answer[] = {CMDMISC_GET_ALL_PARAMS , Id, IdExt, GetTypeParam()};
		SendMsg(destId, Protos::MSGTYPE_CMDMISC_ANSWER, answer, 4);
	}
}

char BaseParam::SetField(char fieldAndType, const char* pValue)
{
	const char field = (fieldAndType >> 3);
	switch (field)
	{
	case Protos::FIELD_VALUE:
	{
		if (IsReadOnly())
			return RET_PSF_RO;

		VALUE_TYPE type = GetValueType();
		if (type != (VALUE_TYPE)(fieldAndType & 0x07))
			return RET_PSF_WRONGTYPE;
		
		Variant v(pValue, type);
		SetValue(v);
	}
	return RET_PSF_OK;
	case Protos::FIELD_SENDRATE:
	{
		Variant v(pValue, TYPE_SHORT);
		short sendRate = v.GetI2();
		SetSendRate(v.GetI2());
		if (sendRate == 0)
		{
			SetSendRate(v.GetI2());
		}
	}
	return RET_PSF_OK;
	case Protos::FIELD_UPDATERATE:
	{
		UpdateParam* p = 0;
		if (QueryInterface(IID_UPDATABLE, (void*&)p))
		{
			Variant v(pValue, TYPE_FLOAT);
			p->SetUpdateRate(v.GetI2());
			return RET_PSF_OK;
		}
		else
		{
			return RET_PSF_NOT_UPDATABLE;
		}
	}
	break;
	case Protos::FIELD_MULT:
	case Protos::FILED_OFFSET:
	{
		CalibrParam* p = 0;
		if (QueryInterface(IID_CALIBRATEABLE, (void*&)p))
		{
			Variant v(pValue, TYPE_FLOAT);
			if (field == Protos::FILED_OFFSET)
				p->SetOffset(v.GetF4());
			else
				p->SetMult(v.GetF4());
			
			return RET_PSF_OK;
		}
		else
		{
			return RET_PSF_NOT_CALIBRATEABLE;
		}
	}
	break;
	}
	return RET_PSF_NOFIELD;
}

void FloatParam::GetValue(Variant& v) const
{
	v = Variant(Value);
}

VALUE_TYPE FloatParam::GetValueType() const
{
	return TYPE_FLOAT;
}

void FloatParam::SetValue(const Variant& v)
{
	v.Get(Value);
	IsValid = true;
}

void ShortParam::GetValue(Variant& v) const
{
	v = Variant(Value);
}

VALUE_TYPE ShortParam::GetValueType() const
{
	return TYPE_SHORT;
}

void ShortParam::SetValue(const Variant& v)
{
	v.Get(Value);
	IsValid = true;
}


void IntParam::GetValue(Variant& v) const
{
	v = Variant(Value);
}

VALUE_TYPE IntParam::GetValueType() const
{
	return TYPE_LONG;
}

void IntParam::SetValue(const Variant& v)
{
	v.Get(Value);
	IsValid = true;
}

void CharParam::GetValue(Variant& v) const
{
	v = Variant(Value);
}

VALUE_TYPE CharParam::GetValueType() const
{
	return TYPE_CHAR;
}

void CharParam::SetValue(const Variant& v)
{
	v.Get(Value);
	IsValid = true;
}


