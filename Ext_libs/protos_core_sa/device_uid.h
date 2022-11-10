#ifndef DEVICE_UID_H_
#define DEVICE_UID_H_

#include "stdint.h"
/*!
	���������� ����� ���������� �����.\n
	�� ����������� �� ���������� EEPROM ��� �� ���������� DS2401.\n
	������ �� ����� ���� �� 4 �� 8 ������.
*/
class DeviceUID
{
public:
	enum TYPE
	{
		TYPE_NULL       = 0,
		TYPE_MICROCHIP  = 1, //!< UID equal UID of (microchip) EEPROM chip UID,  4 bytes
		TYPE_DS2401		= 2, //!< UID equal to UID of 1-Wire chip DS2401,		6 bytes
	};

	DeviceUID(TYPE type = TYPE_NULL)
		: Type(type)
	{
		Data.I8 = 0;
	}

	bool MatchType(char type) const { return (Type == type); }
	bool MatchData(uint32_t uid) const
	{
		return (Type == TYPE_MICROCHIP && Data.I4 == uid);
	}

	union DATA
	{
		uint32_t I4;
		uint64_t I8;
		char I1[8];
	};
	DATA Data;
	TYPE Type;
};



#endif /* DEVICE_UID_H_ */
