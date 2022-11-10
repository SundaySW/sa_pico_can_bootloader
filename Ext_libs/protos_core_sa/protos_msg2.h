#ifndef PROTOS_MSG2_H_
#define PROTOS_MSG2_H_

#include "protos.h"

namespace Protos
{

	// View of Msg2::Data
	struct Data2View
	{
		union
		{
			unsigned char CmdID;
			unsigned char MsgID;
			unsigned char ParamID;
		};
		union
		{
			struct 
			{
				unsigned char FieldType  : 3;
				unsigned char FieldID    : 5;
			}Bits;
			unsigned char FieldAndType;
		};
	};

	// Long or short protos message
	// Должно заменить класс Protos::Msg
	struct Msg2
	{
		enum { BUFFER_SIZE = 64 };
		unsigned char Pri;					///< priority
		unsigned char Src;					///< sender
		unsigned char Dst;					///< receiver
		unsigned char Type = MSGTYPE_NULL;	///< message type
		int Dlc = 0;
		union
		{
			Data2View View;
			unsigned char Data[BUFFER_SIZE];
		};
	};

}//namespace Protos

#endif /* PROTOS_MSG2_H_ */