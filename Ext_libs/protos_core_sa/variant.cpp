 #include "variant.h"
 #include <sys/cdefs.h>
 #include <string.h>

 Variant::Variant()
	: Type(TYPE_UNKNOWN)
	, ArraySize(0)
 {
	 Float = 0.0f;
 }

 Variant::Variant(int size, const char* array)
	: Type(TYPE_BYTEARRAY)
	, ArraySize(0)
 {
	 if (size <= (int)(sizeof(Array) / sizeof(Array[0])))
	 {
		 for (int i = 0; i < size; i++)
		 {
			 Array[i] = array[i];
		 }
		 ArraySize = size;
	 }
 }

 Variant::Variant(const char* value, VALUE_TYPE type)
	: Type(type)
 {
	switch (type)
	{
		case TYPE_CHAR:
			Int1 = value[0];
		break;
		case TYPE_SHORT:
			memcpy(&Int2, value, sizeof(Int2));
		break;
		case TYPE_FLOAT:
			memcpy(&Float, value, sizeof(Float));
		break;
		default:
			Type = TYPE_UNKNOWN;
	}
 }

int GetTypeSize(VALUE_TYPE type)
{
	switch (type)
	{
	case TYPE_CHAR:  return sizeof(char);
	case TYPE_SHORT: return sizeof(short);
	case TYPE_LONG:  return sizeof(int);
	case TYPE_FLOAT: return sizeof(float);
	default:
		return 0;
	};
	return 0;
}

int Variant::Get(char array[8]) const
{
	if (Type != TYPE_BYTEARRAY) return 0;
	for (unsigned char i = 0; i < ArraySize; i++)
	{
		array[i] = Array[i];
	}
	return ArraySize;
}

Variant& Variant::operator=(const Variant& v)
{
	if (&v == this) return *this;
	Type  = v.Type;
	if (Type == TYPE_BYTEARRAY)
	{
		for (int i = 0; i < v.ArraySize; i++)
		Array[i] = v.Array[i];
	}
	else
	{
		Float = v.Float;
	}
	ArraySize = v.ArraySize;
	return *this;
}


bool Variant::Get(float& value) const
{
	switch (Type)
	{
	case TYPE_CHAR:  value = Int1;  return true;
	case TYPE_SHORT: value = Int2;  return true;
	case TYPE_LONG:	 value = Int4;  return true;
	case TYPE_FLOAT: value = Float; return true;
	default:
		return false;
	}
	return false;
}

bool Variant::Get(char& value) const
{
	switch (Type)
	{
	case TYPE_CHAR:  
		value = Int1; 
	return true;
	default:
		return false;
	}
	return false;
}

bool Variant::Get(short& value) const
{
	switch (Type)
	{
	case TYPE_VOID:
	case TYPE_FLOAT:
	case TYPE_BYTEARRAY:
	case TYPE_UNKNOWN:
		return false;
	case TYPE_CHAR:
		value = Int1;
		return true;
	case TYPE_LONG:
		value = (short)Int4;
		return true;
	case TYPE_SHORT:  
		value = Int2;
	return true;
	};
	return false;
}

bool Variant::Get(int& value) const
{
	switch (Type)
	{
	case TYPE_CHAR:  value = Int1;  return true;
	case TYPE_SHORT: value = Int2;  return true;
	case TYPE_LONG:  value = Int4;  return true;
	default: return false;
	}
	return false;
}

int Variant::Get(char* buffer, int size) const
{
	if (size < GetTypeSize(Type)) return 0;

	switch (Type)
	{
	case TYPE_CHAR:
		buffer[0] = Int1;
	return sizeof(char);
	case TYPE_LONG:
		memcpy(buffer, &Int4, sizeof(Int4));
	return sizeof(int);
	case TYPE_SHORT:
		memcpy(buffer, &Int2, sizeof(Int2));
	return sizeof(short);
	case TYPE_FLOAT:
		memcpy(buffer, &Float, sizeof(Float));
	return sizeof(float);
	default:
		return 0;
	}
	return 0;
}
