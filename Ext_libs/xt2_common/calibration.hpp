//SA 14.03.21
#ifndef APP_CALIBRATION_HPP_
#define APP_CALIBRATION_HPP_

#include "25aa02uid.hpp"

enum CALIB_TYPE
{
	UNDEFINED = 0,
	LINEAR = 1,
	LUT = 2, // 5 points + 1 user point setup
};

enum
{
	MAX_CALIB_POINTS = 5,
};

struct Point
	{
		int16_t pos = 0;
		uint16_t adc = 0;
	};

struct CalibConfig
{
	C_25AA02UID* eeprom;
	uint8_t addr;
	CALIB_TYPE type;
};

class Calibration
{
public:


	Calibration (const CalibConfig& cfg)
	{
		eeprom = cfg.eeprom;
		startaddr = cfg.addr;
		type = cfg.type;
	}

	bool Init()
	{
		if (type == LINEAR)
		{
			if(!readPoint(0x00, &P[0].pos, &P[0].adc))
				return false;
			if(!readPoint(0x01, &P[1].pos, &P[1].adc))
				return false;

			dataExist = true;

			adcMin = P[0].adc;
			adcMax = P[1].adc;

			if (P[1].adc != P[0].adc)
			{
				mult = (float)(P[1].pos - P[0].pos) / (P[1].adc - P[0].adc);
				offset = (float)P[0].pos - mult * P[0].adc;
			}
			return true;
		}
		else if (type == LUT)
		{
			int i;
			for (i = 0; i < MAX_CALIB_POINTS; i++)
			{
				if(!readPoint(i, &P[i].pos, &P[i].adc))
				return false;
			}

			if(!readPoint(i, &userpoint.pos, &userpoint.adc))
				return false;

			dataExist = true;

			adcMin = P[0].adc;
			adcMax = P[4].adc;
			return true;
		}
		return false;
	}

	bool readPoint(uint8_t pointNumber, int16_t* pos, uint16_t* adc)
	{
		if (pointNumber > MAX_CALIB_POINTS)
			return false;
		uint8_t buf[4];
		if (eeprom->Read(startaddr + pointNumber * 4, 4, buf))
		{
			uint16_t _pos = 0, _adc = 0;
			_pos = ((uint16_t)buf[0] << 8) + buf[1];
			_adc = ((uint16_t)buf[2] << 8) + buf[3];
			if (_pos != 0xFFFF && _adc != 0xFFFF)
			{
				*pos = _pos;
				*adc = _adc;
				return true;
			}
		}
		return false;
	}

	bool writePoint(uint8_t pointNumber, int16_t pos, uint16_t adc)
	{
		if (pointNumber > MAX_CALIB_POINTS)
			return false;

		uint8_t buf[4];
		buf[0] = (pos >> 8);
		buf[1] = pos;
		buf[2] = (adc >> 8);
		buf[3] = adc;

		return eeprom->Write(startaddr + pointNumber * 4, 4, buf);
	}

	bool CheckAdcInRange(short adc)
	{
		if (adc > adcMax || adc < adcMin)
			return false;
		return true;
	}

	bool dataExist = false;

	C_25AA02UID* eeprom = 0;
	uint8_t startaddr = 0;

	Point P[MAX_CALIB_POINTS];
	Point userpoint;
	CALIB_TYPE type = UNDEFINED;

	float offset = 0.0f, mult = 1.0f;
	float stepsToADC = 50.0f;

	short adcMax = 3000;
	short adcMin = 800;
};


#endif /* APP_CALIBRATION_HPP_ */
