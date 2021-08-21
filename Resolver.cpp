#include "Hooks.h"
#include "Features.h"

enum EResolverStages {
	antifreestand,
	inverse_antifreestand,
	unresolved,
};

std::string ResolverMode[65];

float CResolver::GetAngle(IBasePlayer* player) {
	return Math::NormalizeYaw(player->GetEyeAngles().y);
}

float CResolver::GetForwardYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(GetBackwardYaw(player) - 180.f);
}

float CResolver::GetAwayAngle(IBasePlayer* player) {
	return Math::CalculateAngle(csgo->local->GetEyePosition(), player->GetEyePosition()).y;
}

float CResolver::GetBackwardYaw(IBasePlayer* player) {
	return Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
}

float CResolver::GetLeftYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y - 90.f);
}

float CResolver::GetRightYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y + 90.f);
}

void CResolver::StoreAntifreestand()
{
	if (!csgo->local->isAlive()) {
		memset(this->ResolverInfo, 0, sizeof(this->ResolverInfo));
		return;
	}
	static int delay_ticks[65] = { 0 };
	for (int i = 0; i < interfaces.engine->GetMaxClients(); ++i)
	{
		auto player = interfaces.ent_list->GetClientEntity(i);

		if (!player 
			|| !player->isAlive() 
			|| player->IsDormant()
			|| !player->IsPlayer()
			|| player->GetTeam() == csgo->local->GetTeam()
			|| player->EntIndex() == interfaces.engine->GetLocalPlayer()) {
			continue;
		}

		auto idx = player->GetIndex();

		if (vars.ragebot.resolver) {
			if (csgo->actual_misses[player->GetIndex()] <= 0) {
				auto latest_animation = g_Animfix->get_latest_animation(player);
				if (!latest_animation)
					continue;

				auto point = g_Ragebot->GetAdvancedHeadPoints(player, latest_animation->unresolved_bones);

				g_Ragebot->BackupPlayer(latest_animation);
				g_Ragebot->SetAnims(latest_animation, latest_animation->unresolved_bones);

				float left_damage = g_AutoWall->Think(point[0], player, HITGROUP_HEAD).m_damage;
				float right_damage = g_AutoWall->Think(point[1], player, HITGROUP_HEAD).m_damage;

				g_Ragebot->RestorePlayer(latest_animation);

				if (left_damage > right_damage)
					FreestandSide[idx] = 1;
				else if (left_damage < right_damage)
					FreestandSide[idx] = -1;
			}

			memset(&ResolverInfo[idx], 0, sizeof(&ResolverInfo[idx]));
		}
	}
}

bool DoesHaveFakeAngles(IBasePlayer* player) {
	static int skip_ticks[65] = { 0 };
	int idx = player->GetIndex();
	if (player->GetSimulationTime() == player->GetOldSimulationTime())
		skip_ticks[idx]++;
	else
		skip_ticks[idx] = 0;
	return skip_ticks[idx] >= 16;
}

bool CResolver::Do(IBasePlayer* player) {
	
	static int offresolver_ticks[65] = { 0 };
	static float last_velocity[65] = { 0.f };
	static int ticks_with_zero_pitch[65] = { 0.f };

	int idx = player->GetIndex();
	auto animstate = player->GetPlayerAnimState();
	if (!animstate || !vars.ragebot.enable || player->HasGunGameImmunity() || !vars.ragebot.resolver || !csgo->local->isAlive()) {
		ResolverMode[idx] = str("OFF");
		ResolverInfo[idx].Index = 0;
		return false;
	}

	if (player->GetEyeAngles().x < 45.f)
		ticks_with_zero_pitch[idx]++;
	else
		ticks_with_zero_pitch[idx] = 0;

	int add = 0;
	if (!shot_snapshots.empty()) {
		const auto& snapshot = shot_snapshots.front();
		const bool& dt_ready = !csgo->need_to_recharge && g_Misc->dt_bullets <= 1 && csgo->weapon->isAutoSniper();
		if (dt_ready && snapshot.hitbox == 0 && snapshot.intended_damage > player->GetHealth()) {
			add = g_Misc->dt_bullets;
		}
	}

	int missed_shots = (csgo->actual_misses[idx] + csgo->imaginary_misses[idx] + add) % 4;

#ifndef _DEBUG
	if (player->GetPlayerInfo().fakeplayer) {
		ResolverMode[idx] = str("OFF");
		ResolverInfo[idx].Index = 0;
		return false;
	}
#endif

	const float delta = 60.f;
	const float angle = GetAngle(player);

	const float low_delta = player->GetDSYDelta() * 0.5f;

	float desync_angle = delta;

	const bool& sideways =
		fabsf(Math::NormalizeYaw(angle - GetLeftYaw(player))) < 45.f
		||
		fabsf(Math::NormalizeYaw(angle - GetRightYaw(player))) < 45.f;

	const bool& forward = fabsf(Math::NormalizeYaw(angle - GetForwardYaw(player))) < 90.f && !sideways;

	if (!HitInfo[idx].empty()) {
		const auto& info = HitInfo[idx].front();
		if ((csgo->actual_misses[idx] + csgo->imaginary_misses[idx]) <= 0) {
			static std::string AA = str("AA");
			static std::string AB = str("AB");
			static std::string AC = str("AC");

			int side = FreestandSide[idx];
			if (side != 0)
			{
				if (info.Stage == inverse_antifreestand) {
					side *= -1;
					ResolverMode[idx] = AA;
					ResolverInfo[idx].Stage = EResolverStages::inverse_antifreestand;
				}
				else {
					ResolverInfo[idx].Stage = EResolverStages::antifreestand;
					ResolverMode[idx] = AB;
				}

				ResolverInfo[idx].Index = forward ? -side : side;
			}
			else {
				ResolverInfo[idx].Index = 0;
				ResolverMode[idx] = AC;
			}
			if (info.LowDelta) {
				desync_angle = low_delta;
				ResolverMode[idx].insert(ResolverMode[idx].begin(), 'l');
				ResolverInfo[idx].LowDelta = true;
			}
			else {
				ResolverInfo[idx].LowDelta = false;
				ResolverMode[idx].insert(ResolverMode[idx].begin(), 'd');
			}
			return false;
		}
	}

	if (missed_shots >= 2) {
		const float& velocity = player->GetVelocity().Length2D();
		const bool& additional_check = player->GetDuckAmount() >= 0.8f || player->GetEyeAngles().x <= 60.f;
		if (Math::IsNearEqual(velocity, last_velocity[idx], 10.f)) {
			if (velocity <= 120.f) {
				if (!additional_check)
					desync_angle = low_delta;
				else {
					if (missed_shots >= 3 && additional_check)
						desync_angle = low_delta;
				}
			}
		}
	}

	if (missed_shots == 0) {
		if (FreestandSide[idx] != 0) {
			ResolverMode[idx] = hs::C.s();

			if (forward) {
				ResolverMode[idx] += hs::B.s();
			}
			else
				ResolverMode[idx] += hs::A.s();
			ResolverInfo[idx].Index = forward ? -FreestandSide[idx] : FreestandSide[idx];
			ResolverInfo[idx].Stage = EResolverStages::antifreestand;
		}
		else
		{
			static std::string CC = str("CC");
			ResolverMode[idx] = CC;
			ResolverInfo[idx].Index = 0;
			ResolverInfo[idx].Stage = EResolverStages::unresolved;
		}
	}
	else {
		switch (missed_shots % 2)
		{
		case 0:
			if (FreestandSide[idx] == 0)
				FreestandSide[idx] = 1;
			break;
		case 1:
			if (FreestandSide[idx] == 0)
				FreestandSide[idx] = -1;
		}
		
		if (forward) {
			switch (missed_shots % 2) {
			case 0:
				ResolverMode[idx] = hs::DB.s();
				ResolverInfo[idx].Index = -FreestandSide[idx];
				break;
			case 1:
				ResolverMode[idx] = hs::DA.s();
				ResolverInfo[idx].Index = FreestandSide[idx];
				break;
			}
		}
		else {
			switch (missed_shots % 2) {
			case 0:
				ResolverMode[idx] = hs::DB.s();
				ResolverInfo[idx].Index = FreestandSide[idx];
				break;
			case 1:
				ResolverMode[idx] = hs::DA.s();
				ResolverInfo[idx].Index = -FreestandSide[idx];
				break;
			}
		}
		if (desync_angle != low_delta)
			ResolverInfo[idx].Stage = EResolverStages::inverse_antifreestand;
	}

	if (desync_angle == low_delta) {
		ResolverMode[idx].insert(ResolverMode[idx].begin(), 'l');
		ResolverInfo[idx].LowDelta = true;
	}
	else {
		ResolverInfo[idx].LowDelta = false;
		ResolverMode[idx].insert(ResolverMode[idx].begin(), 'd');
	}
	last_velocity[idx] = player->GetVelocity().Length2D();
	
	if (ResolverInfo[idx].Index != 0)
		ResolverInfo[idx].ResolvedAngle = Math::NormalizeYaw(angle + (desync_angle * ResolverInfo[idx].Index));
	return true;
}
