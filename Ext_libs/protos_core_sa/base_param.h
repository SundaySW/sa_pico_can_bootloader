#ifndef BASE_PARAM_TR1_H_
#define BASE_PARAM_TR1_H_

#include "variant.h"
#include "protos_msg.h"
//#include "protos_msg2.h"

enum INTERFACE_ID
{
	IID_UNKNOWN       = 0,
	IID_UPDATABLE     = 1,
	IID_CONTROLABLE   = 2,
	IID_CALIBRATEABLE = 3
};

enum PARAM_TYPE
{
	TP_Unknown, //disable?
	TP_LinkParam,
	TP_ExtParam,
	TP_AnalogInpParam,
	TP_AnalogOutParam,
	TP_DiscreteInpParam,
	TP_DiscreteOutParam,
	TP_FreqParam
};


class BaseParam
{
public:
	BaseParam()
		: IsValid(true)
		, TypeAttr(0)
		, SendRate(0)
		, SendTime(0)
		, Id(0)
	{
	}
	BaseParam(char id)
		: IsValid(true)
		, TypeAttr(0)
		, SendRate(0)
		, SendTime(0)
		, Id(id)
	{
	}
	void Close();
	bool GetField(char field, Variant& value);
	short GetSendRate() const { return SendRate; }
	short GetSendTime() const { return SendTime; }
	virtual void GetValue(Variant&) const {};
	char GetValueAndTypeFlag() { return (Protos::FIELD_VALUE << 3) | GetValueType(); }
	virtual VALUE_TYPE GetValueType() const { return TYPE_UNKNOWN; }
	void InvalidateValue() { IsValid = false; }
	bool IsReadOnly() const { return (TypeAttr & 0x08); }
	virtual void OnTimer(int ms = 1);
	void SetValid(bool IsVal) {IsValid = IsVal;}; //sa added 23.06.20
	bool GetValid() {return IsValid;}	; //sa added 23.06.20
	char GetId() const  { return Id; }
	char GetIdExt() const { return IdExt; }
	virtual bool MatchId(char id) const { return (id == Id); }
	void SetId(char id) { Id = id; }
	virtual void Poll();
	
	virtual PARAM_TYPE GetTypeParam() {return TP_Unknown;};
	
		virtual uint8_t GetConfigBytes(char* buffer) {return 0;};   // �������� ��������� ��� ������������ ���������. Ÿ ����� �������� � EEPROM.
		virtual bool SetConfigBytes(char* buffer, uint8_t Size) { return false;}; // ���������������� �������� ��� ������ ��������� ������. �� ����� �������� �� EEPROM.
	
	bool ProcessMessage(const Protos::Msg& msg);
	//bool ProcessMessage(const Protos::Msg2& msg);
	virtual bool SendValue(char destId = Protos::BROADCAST);
	virtual void SendIDandType(char destId = Protos::Msg::BROADCAST); //sa added 23.06.20
	char SetField(char fieldAndType, const char* pValue);
	void SetSendRate(short sendRate) { SendRate = sendRate; }
	void SetSendTime(short sendTime) { SendTime = sendTime; }
	virtual bool QueryInterface(INTERFACE_ID /*iid*/, void*& /*p*/) { return false; };
	virtual void SetValue(const Variant&) {};
	virtual bool UpdateValue() { return false; }
	
protected:
	bool IsValid;	///< ������������� ��� ��� ��������  ���������, ���� �������� �� �������������
					///< �������� �� �������� �� ������
	char TypeAttr;
	short SendRate, SendTime;
	char Id, 
	IdExt; //sa added 23.06.20
};

class CalibrParam : public virtual BaseParam
{
public:
	CalibrParam()
		: Mult(1.0f)
		, Offset(0.0f)
	{}
	float GetOffset() const		 { return Offset;	}
	void SetOffset(float offset) { Offset = offset; }
	float GetMult() const		 { return Mult;		}
	void SetMult(float mult)	 { Mult = mult;		}
	float Calibrate(float value) const
	{
		return Mult * (value + Offset);
	}
protected:
	float Mult, Offset;
};

class FloatParam : public virtual BaseParam
{
public:
	FloatParam(uint8_t id)
		: BaseParam(id)
		, Value(0.0f)
	{
	}
	FloatParam()
		: BaseParam(0)
		, Value(0.0f)
	{
	}
	float GetValue() const { return Value; }
	void GetValue(Variant& v) const;
	VALUE_TYPE GetValueType() const;
	void SetFloat(float value) { Value = value; }
	void SetValue(const Variant& v);
protected:
	float Value;
};

class ShortParam : public virtual BaseParam
{
public:
	ShortParam()
		: BaseParam(0)
		, Value(0)
	{
	}
	void GetValue(Variant& v) const;
	VALUE_TYPE GetValueType() const;
	void SetValue(const Variant& v);
	void SetShort(short value) { Value = value; }
	short GetValue() const { return Value; }
protected:
	short Value;
};

class IntParam : public virtual BaseParam
{
public:
	IntParam()
		: BaseParam(0)
		, Value(0)
	{
	}
	void GetValue(Variant& v) const;
	VALUE_TYPE GetValueType() const;
	void SetValue(const Variant& v);
	void SetInt(int value) { Value = value; }
	int GetValue() const { return Value; }
protected:
	int Value;
};

class CharParam : public virtual BaseParam
{
public:
	CharParam(uint8_t id)
		: BaseParam(id)
		, Value(0)
	{
	}
	CharParam()
		: BaseParam(0)
		, Value(0)
	{
	}
	char GetValue() const { return Value; }
	void GetValue(Variant& v) const;
	VALUE_TYPE GetValueType() const;
	void SetChar(char value) { Value = value; }
	void SetValue(const Variant& v);
protected:
	char Value;
};

class UpdateParam : public virtual BaseParam
{
public:
	UpdateParam(int rate = 0)
		: UpdateRate(rate)
		, UpdateTime(0)
	{
	}
	bool QueryInterface(INTERFACE_ID iid, void*& p)
	{
		switch (iid)
		{
		case IID_UPDATABLE:
			p = (UpdateParam*)this;
		return true;
		default:
		return false;
		}
		return false;
	}
	short GetUpdateRate() const		{ return UpdateRate; }
	short GetUpdateTime() const		{ return UpdateTime; }
	void SetUpdateRate(short rate)  { UpdateRate = rate; }
	void SetUpdateTime(short time)  { UpdateTime = time; }
protected:
	short UpdateRate;
	short UpdateTime;
};

class CtrlParam : public virtual BaseParam
{
	public:
	CtrlParam(int rate = 0)
	: CtrlRate(rate)
	, CtrlTime(0)
	{
	}
	bool QueryInterface(INTERFACE_ID iid, void*& p)
	{
		switch (iid)
		{
			case IID_CONTROLABLE:
			p = (CtrlParam*)this;
			return true;
			default:
			return false;
		}
		return false;
	}
	short GetCtrlRate() const		{ return CtrlRate; }
	short GetCtrlTime() const		{ return CtrlTime; }
	void SetCtrlRate(short rate)  { CtrlRate = rate; }
	void SetCtrlTime(short time)  { CtrlTime = time; }
	protected:
	short CtrlRate;
	short CtrlTime;
};

#endif /* BASE_PARAM_TR1_H_ */
