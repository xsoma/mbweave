#pragma once
#include "Hooks.h"
class CHitMarker
{
public:
	struct hitmarker_info
	{
		Vector position;
		int alpha;
		float time;
		std::string hp;
		bool hs;
		int seed;
	};
	std::vector<hitmarker_info> hitmarkers;
	void Paint();
	void Add(Vector hitpos, bool headshot, std::string damage);
};