/*
 * pack2x.hpp
 *
 *  Created on: Mar 7, 2021
 *      Author: SAAI
 */

#ifndef PACK2X_HPP_
#define PACK2X_HPP_

inline void Pack2x(char src, char** dst)
{
	const char* LUT2X = "0123456789ABCDEF";

	(*dst)[0] = (char)LUT2X[(src >> 0x4) & 0x0F];
	(*dst)[1] = (char)LUT2X[src & 0x0F];
	*dst += 2;
}

inline uint8_t Pack2x(const char* msg, uint8_t len, char* msg2x)
{
	for (int i = 0; i < len; i++)
		{
			Pack2x(msg[i], &msg2x);
		}

	return 2 * len;
}

inline uint8_t Unpack2x(const char* msg2x, int len2x, char* msg)
{
	if (len2x & 1) return 0;

	char c[2];
	int n = 0;

	for (int i = 0; i < len2x; i += 2)
	{
		c[0] = msg2x[i]; c[1] = msg2x[i + 1];

		for (int k = 0; k < 2; k++)
		{
			if (c[k] >= 'A' && c[k] <= 'F')
			c[k] -= '7';
			else if (c[k] >= '0' && c[k] <= '9')
			c[k] -= '0';
			else if (c[k] >= 'a' && c[k] <= 'f')
			c[k] -= 'W';
			else
			return 0;
		}
		msg[n++] = ((c[0] << 0x4) & 0xF0) | (c[1] & 0x0F);
	}
	return n;
}



#endif /* PACK2X_HPP_ */
