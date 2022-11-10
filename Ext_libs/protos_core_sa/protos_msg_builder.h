#ifndef PROTOS_MSG_BUILDER_H_
#define PROTOS_MSG_BUILDER_H_

#include "protos_msg2.h"

namespace Protos
{
	struct Packet;

	/*!
		Класс для сборки сообщения из пакетов.
	*/
	class MsgBuilder
	{
	public:
		MsgBuilder();
		bool Grab(const Packet& packet);
		bool IsReady() const;
	
	private:
		bool Long;				///< true/false: Собираем long/short сообщение
		unsigned char Index;	///< Ожидаемый индекс пакета long сообщения
		union
		{
			unsigned char  LenBytes[2];
			unsigned short Len;	///< Ожидаемая длина данных long сообщения
		};
		bool Ready;				///< Сборка закончена

	public:
		Msg2 Msg;				///< Целевое сообщение
	};

}//namespace Protos


#endif /* PROTOS_MSG_BUILDER_H_ */