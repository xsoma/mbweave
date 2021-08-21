#include "Hooks.h"
#include "Features.h"

animation* CLegit::GetAimAnimation() {
	if (!csgo->local->isAlive() || !csgo->cmd || !csgo->weapon->IsGun())
		return nullptr;
	Vector center_of_screen = Vector(csgo->w / 2, csgo->h / 2, 0);
	int closest_distance = INT_MAX;
	animation* closest_info = nullptr;
	for (auto i = 1; i <= interfaces.global_vars->maxClients; i++)
	{
		auto entity = interfaces.ent_list->GetClientEntity(i);
		if (!entity || entity == csgo->local)
			continue;
		if (!entity->isAlive() || entity->GetHealth() <= 0)
			continue;	
		if (entity->IsDormant() || entity->HasGunGameImmunity())
			continue;

		auto animation_info = g_Animfix->animation_infos.find(entity->GetRefEHandle());
		if (animation_info != g_Animfix->animation_infos.end()) {
			for (auto anims = animation_info->second->frames.begin(); anims != animation_info->second->frames.end(); anims++)
			{
				auto head_pos3d = anims->player->GetBonePos(anims->bones, 8);
				Vector head_pos2d;
				if (!Math::WorldToScreen(head_pos3d, head_pos2d))
					continue;
				auto distance = (center_of_screen - head_pos2d).Length2D();
				if (distance < closest_distance) {
					closest_distance = distance;
					closest_info = &*anims;
				}
			}
		}
	}
	
	return closest_info;
}

void CLegit::Backtrack()
{
	if (!vars.ragebot.enable)
		return;
	if (auto closest_animation = GetAimAnimation(); closest_animation != nullptr)
		csgo->cmd->tick_count = TIME_TO_TICKS(closest_animation->sim_time + g_Ragebot->LerpTime());
}

void CLegit::AutoPistol() {
	if (!csgo->local || !csgo->cmd || !csgo->weapon)
		return;

	if (csgo->local->GetFlags() & FL_FROZEN
		|| csgo->local->HasGunGameImmunity()
		|| csgo->game_rules->IsFreezeTime())
		return;

	if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER
		|| csgo->weapon->GetItemDefinitionIndex() == WEAPON_HEALTHSHOT
		|| csgo->weapon->IsNade()
		|| csgo->weapon->IsBomb()
		|| csgo->weapon->isAutoSniper())
		return;

	if (!g_Ragebot->IsAbleToShoot()) {
		if (csgo->cmd->buttons & IN_ATTACK)
			csgo->cmd->buttons &= ~IN_ATTACK;
		else if (csgo->weapon->IsKnife() && (csgo->cmd->buttons & IN_ATTACK || csgo->cmd->buttons & IN_ATTACK2))
		{
			if (csgo->cmd->buttons & IN_ATTACK)
				csgo->cmd->buttons &= ~IN_ATTACK;

			if (csgo->cmd->buttons & IN_ATTACK2)
				csgo->cmd->buttons &= ~IN_ATTACK2;
		}
	}

}
