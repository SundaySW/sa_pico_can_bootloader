#ifndef COMMANDS_H_
#define COMMANDS_H_

enum CMDMISC
{
	CMDMISC_NULL = 0,					  ///< Для общей ошибки возвращается CMDMISC_ERROR с кодом CMDMISC_NULL
	CMDMISC_GET_FIRMWARE_VERSION = 0x14,  ///< Получение версии прошивки
	CMDMISC_GET_ALL_PARAMS		 = 0x15,   ///< Получить ID всех параметров с типом параметра.
};

enum 
{ 
	CMDMISC_ERROR_READ_UID = 1,			  ///< Ошибка чтения уникального номера устройства
};

#endif /* COMMANDS_H_ */