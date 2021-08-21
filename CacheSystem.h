#pragma once
#include <vector>

class CCacheSystem {
public:
	struct CPlayerCache {
		int flags = 0;
		Vector vec_origin = Vector(0.f, 0.f, 0.f), velocity = Vector(0.f, 0.f, 0.f);
		std::string name;
	};
	std::map<int, CPlayerCache> player_info;
	std::vector<int> valid_players;
	void FrameStageNotify();
	void CreateMove();
};