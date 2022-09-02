//SA 20052021
#ifndef ARRAY_H_
#define ARRAY_H_

template <class T, int size>

struct Array
{
	T data[size];
	int len = 0;

	bool Append(T byte)
	{
		if (len < size)
		{
			data[len++] = byte;
			return true;
		}
		return false;
	}
	void Reset()
	{
		len = 0;
	}
};

using CArray = Array<char, 100>;

#endif
