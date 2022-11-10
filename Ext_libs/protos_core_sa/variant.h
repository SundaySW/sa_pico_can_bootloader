#ifndef VARIANT_TR1_H_
#define VARIANT_TR1_H_

#include "value_type.h"

extern int GetTypeSize(VALUE_TYPE type);

class Variant
{
public:
	Variant();
	Variant(char c) : Int1(c) { Type = TYPE_CHAR; }
	Variant(short s) : Int2(s) { Type = TYPE_SHORT;  }
	Variant(int i) : Int4(i) { Type = TYPE_LONG;  }
	Variant(float f): Float(f) { Type = TYPE_FLOAT;}
	Variant(int size, const char* array);
	Variant(const char* value, VALUE_TYPE type);

	char GetI1() const { return Int1;  }
	short GetI2() const { return Int2;  }
	int GetI4() const { return Int4;  }
	float GetF4() const { return Float; }
	bool Get(float& value) const;
	bool Get(char& value) const;
	bool Get(short& value) const;
	bool Get(int& value) const;
	int Get(char* buffer, int size) const;
	int Get(char array[8]) const;

	VALUE_TYPE GetType() const
	{
		return Type;
	}

	void Close()
	{
		Type = TYPE_UNKNOWN;
		Float = 0.0f;
	}

	Variant& operator=(const Variant& v);

	operator bool() const
	{
		return Type != TYPE_UNKNOWN;
	}
	
private:
	union
	{
		char Int1;
		short Int2;
		long Int4;
		float Float;
		char Array[8];
	};
	VALUE_TYPE Type;
	unsigned char ArraySize;
};




#endif /* VARIANT_TR1_H_ */