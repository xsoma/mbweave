#pragma once
/* Other */
#define _CRT_SECURE_NO_WARNINGS
//#include "protect/VMProtectSDK.h"
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <psapi.h>
#include <time.h>
#include <process.h>
#include <vector>
#include <map>
#include <ostream>
#include <Shlobj.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <cmath>
#include <float.h>
#include <codecvt>
#include <sapi.h>
#include <algorithm>
#include <iterator>
#include <d3d9.h>
#include <rpcndr.h>
#include "protect/protect.hpp"

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(size) BYTE MACRO_CONCAT(_pad, __COUNTER__)[size];

/* Cheat */
#include "cMainStruct.h"
#include "Vector.h"
#include "RecvProps.h"
#include "bspflags.h"

template<typename T>
class Singleton
{
protected:
	Singleton() {}
	~Singleton() {}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

public:
	__forceinline static T& Get()
	{
		static T inst{};
		return inst;
	}
};

#pragma pack(push, 1)
struct c_bind {
	c_bind() {
		static_assert(sizeof(c_bind) <= 4, "sizeof c_bind > 4");

		active = false;
		type = 0;
		key = 0;
	}
	uint8_t type = 0;
	uint16_t key = 0;
	bool active = false;
};
#pragma pack(pop)

namespace js_variables {
	extern std::map<std::string, int*> ints;
	extern std::map<std::string, uint32_t*> uints;
	extern std::map<std::string, bool*> bools;
	extern std::map<std::string, color_t*> colors;
	extern std::map<std::string, c_bind*> binds;
	extern std::string find_bool(bool* ptr);
	extern std::string find_int(int* ptr);
	extern std::string find_uint(uint32_t* ptr);
	extern void link();
}
