#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "protos.h"

namespace Protos
{

	class Msg
	{
	public:
		enum
		{
			BROADCAST = 0xFF
		};
	
		Msg();
		Msg(unsigned char senderId, unsigned char destId);
		Msg(unsigned long id, const char* data, int dlc);
		
		bool From2x(char* packet2x, int len);	
		bool FromFixed(char* packet, int len);
		unsigned char GetDestID() const;
		unsigned char GetMSGId() const;
		unsigned char GetMSGType() const;
		unsigned char GetSenderID() const;
		unsigned char GetLongIndex() const;
		bool IsLong() const;
		bool IsBoot() const;
		bool IsLongFirst() const;
		bool IsLongNext() const;
		bool IsLongFlow() const;
		bool IsRaw()  const;
		int Make2x(char* result) const;
		int MakeFixed(char* result) const;
		void NextLongIndex();
		void SetMSGId(unsigned char msgId);
		void SetDataBuffer(const unsigned char* buffer, int size);
		void SetLongFirst();
		void SetLongNext();
		void SetMSGType(unsigned char msgType);
		void StartLongIndex();
		char GetParamID() const;
		char GetFieldID() const;
		char GetFieldAndType() const;
		
		unsigned char GetCMDId() const;
	
		enum { DATA_SIZE = 8 };
				
		union UID
		{
			unsigned long  Ext;
			unsigned short Std;
			unsigned char  Tab[4];
		}Id;
		unsigned char Dlc;
		unsigned char Data[DATA_SIZE];
	};
		
}//namespace Protos



