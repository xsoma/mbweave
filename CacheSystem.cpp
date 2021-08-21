#include "Hooks.h"
#include "CacheSystem.h"

void CCacheSystem::FrameStageNotify() {
	valid_players.clear();
	for (auto i = 1; i <= interfaces.global_vars->maxClients; ++i) {
		auto pEntity = interfaces.ent_list->GetClientEntity(i);
		if (!pEntity)
			continue;
		if (!pEntity->isAlive() || pEntity->GetHealth() <= 0)
			continue;

		player_info[pEntity->GetIndex()].velocity = pEntity->GetVelocity();
		player_info[pEntity->GetIndex()].vec_origin = pEntity->GetOrigin();
		player_info[pEntity->GetIndex()].flags= pEntity->GetFlags();
		player_info[pEntity->GetIndex()].name = pEntity->GetName();

		valid_players.push_back(pEntity->GetIndex());
	}
}

void CCacheSystem::CreateMove() {

}