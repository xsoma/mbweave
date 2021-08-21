#pragma once

#include <d3dx9.h>

#include <map>
#include <string>

class script_cache_data
{
public:
	std::map<std::string, LPDIRECT3DTEXTURE9> images;
};

extern std::map<std::string, script_cache_data> scripts_cache;