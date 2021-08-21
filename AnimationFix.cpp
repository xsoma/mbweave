#include "Hooks.h"
#include "AnimationFix.h"
#include "Resolver.h"
#include "Features.h"

void UpdatePlayer(IBasePlayer* player)
{
	// make a backup of globals
	const auto backup_frametime = interfaces.global_vars->frametime;
	const auto backup_curtime = interfaces.global_vars->curtime;

	// get player anim state
	auto state = player->GetPlayerAnimState();

	// allow re-animate player in this tick
	if (state->m_last_update_frame == interfaces.global_vars->framecount)
		state->m_last_update_frame -= 1;

	// fixes for networked players
	interfaces.global_vars->frametime = interfaces.global_vars->interval_per_tick;
	interfaces.global_vars->curtime = player->GetSimulationTime();

	// notify the other hooks to instruct animations and pvs fix
	csgo->EnableBones = player->GetClientSideAnims() = true;
	player->UpdateClientSideAnimation();
	csgo->EnableBones = player->GetClientSideAnims() = false;

	player->InvalidatePhysicsRecursive(8);

	// restore globals
	interfaces.global_vars->curtime = backup_curtime;
	interfaces.global_vars->frametime = backup_frametime;
}

void BuildBones(IBasePlayer* player, float sim_time, matrix* mat) {
	auto accessor = player->GetBoneAccessor();
	if (!accessor)
		return;

	// keep track of old occlusion values
	const auto backup_occlusion_flags = player->GetOcclusionFlags();
	const auto backup_occlusion_framecount = player->GetOcclusionFramecount();

	// skip occlusion checks in c_cs_player::setup_bones
	player->GetOcclusionFlags() = 0;
	player->GetOcclusionFramecount() = 0;

	// clear bone accessor
	accessor->m_ReadableBones = accessor->m_WritableBones = 0;

	// invalidate bone cache
	player->InvalidateBoneCache();

	// stop interpolation
	player->GetEffects() |= 0x8;

	// change bone accessor
	const auto backup_bone_array = accessor->get_bone_array_for_write();
	accessor->set_bone_array_for_write(mat);

	// build bones
	player->SetupBones(nullptr, -1, 0x7FF00, interfaces.global_vars->curtime);

	// restore bone accessor
	accessor->set_bone_array_for_write(backup_bone_array);

	// restore original occlusion
	player->GetOcclusionFlags() = backup_occlusion_flags;
	player->GetOcclusionFramecount() = backup_occlusion_framecount;

	// start interpolation again
	player->GetEffects() &= ~0x8;
}

void animation::build_inversed_bones(IBasePlayer* player) {
	auto idx = player->GetIndex();
	static float spawntime[65] = { 0.f };
	static CBaseHandle* selfhandle[65] = { nullptr };

	if (spawntime[idx] == 0.f)
		spawntime[idx] = player->GetSpawnTime();


	auto alloc = g_Animfix->IS_Animstate[idx] == nullptr;
	auto change = !alloc && selfhandle[idx] != &player->GetRefEHandle();
	auto reset = !alloc && !change && player->GetSpawnTime() != spawntime[idx];

	if (change) {
		memset(&g_Animfix->IS_Animstate[idx], 0, sizeof(g_Animfix->IS_Animstate[idx]));
		selfhandle[idx] = (CBaseHandle*)&player->GetRefEHandle();
	}
	if (reset) {
		player->ResetAnimationState(g_Animfix->IS_Animstate[idx]);
		spawntime[idx] = player->GetSpawnTime();
	}

	if (alloc || change) {
		g_Animfix->IS_Animstate[idx] = reinterpret_cast<CCSGOPlayerAnimState*>(interfaces.memalloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (g_Animfix->IS_Animstate[idx])
			player->CreateAnimationState(g_Animfix->IS_Animstate[idx]);
	}

	if (!g_Animfix->IS_Animstate[idx])
		return;

	if (player->GetFlags() & FL_ONGROUND) {
		g_Animfix->IS_Animstate[idx]->m_on_ground = true;
		g_Animfix->IS_Animstate[idx]->m_landing = false;
	}
	g_Animfix->IS_Animstate[idx]->m_duration_in_air = 0.f;

	const float angle = g_Resolver->GetAngle(player);
	const float delta = player->GetDSYDelta();
	const float low_delta = delta * 0.5f;

	float desync_angle = delta;

	bool resolver_disabled = ResolverMode[idx] == str("OFF") || ResolverMode[idx].find('o') != -1 || g_Resolver->ResolverInfo[idx].Index == 0;

	if (ResolverMode[idx].find('l') != -1) // low delta
		desync_angle = low_delta; // half of max desync delta

	if (resolver_disabled)
		g_Animfix->IS_Animstate[idx]->m_abs_yaw = Math::NormalizeYaw(player->GetEyeAngles().y - desync_angle); // setup inversed side
	else
		g_Animfix->IS_Animstate[idx]->m_abs_yaw = Math::NormalizeYaw(Math::NormalizeYaw(angle + (desync_angle * -g_Resolver->ResolverInfo[idx].Index)));

	player->UpdateAnimationState(g_Animfix->IS_Animstate[idx], player->GetEyeAngles());
	BuildBones(player, sim_time, inversed_bones);
}

void animation::build_unresolved_bones(IBasePlayer* player)
{
	auto idx = player->GetIndex();

	static float spawntime[65] = { 0.f };
	static CBaseHandle* selfhandle[65] = { nullptr };

	if (spawntime[idx] == 0.f)
		spawntime[idx] = player->GetSpawnTime();


	auto alloc = g_Animfix->US_Animstate[idx] == nullptr;
	auto change = !alloc && selfhandle[idx] != &player->GetRefEHandle();
	auto reset = !alloc && !change && player->GetSpawnTime() != spawntime[idx];

	if (change) {
		memset(&g_Animfix->US_Animstate[idx], 0, sizeof(g_Animfix->US_Animstate[idx]));
		selfhandle[idx] = (CBaseHandle*)&player->GetRefEHandle();
	}
	if (reset) {
		player->ResetAnimationState(g_Animfix->US_Animstate[idx]);
		spawntime[idx] = player->GetSpawnTime();
	}

	if (alloc || change) {
		g_Animfix->US_Animstate[idx] = reinterpret_cast<CCSGOPlayerAnimState*>(interfaces.memalloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (g_Animfix->US_Animstate[idx])
			player->CreateAnimationState(g_Animfix->US_Animstate[idx]);
	}

	if (!g_Animfix->US_Animstate[idx])
		return;

	if (player->GetFlags() & FL_ONGROUND) {
		g_Animfix->US_Animstate[idx]->m_on_ground = true;
		g_Animfix->US_Animstate[idx]->m_landing = false;
	}
	g_Animfix->US_Animstate[idx]->m_duration_in_air = 0.f;

	const float delta = player->GetDSYDelta();

	const float low_delta = delta * 0.5f;

	float desync_angle = delta;

	bool resolver_disabled = ResolverMode[idx] == str("OFF") || ResolverMode[idx].find('o') != -1 || g_Resolver->ResolverInfo[idx].Index == 0;

	if (ResolverMode[idx].find('l') != -1) // low delta
		desync_angle = low_delta; // half of max desync delta

	if (resolver_disabled)
		g_Animfix->US_Animstate[idx]->m_abs_yaw = Math::NormalizeYaw(player->GetEyeAngles().y + desync_angle); // setup inversed side
	else
		g_Animfix->US_Animstate[idx]->m_abs_yaw = Math::NormalizeYaw(player->GetEyeAngles().y);

	player->UpdateAnimationState(g_Animfix->US_Animstate[idx], player->GetEyeAngles());
	BuildBones(player, sim_time, unresolved_bones);
}

void animation::build_server_bones(IBasePlayer* player)
{
	BuildBones(player, sim_time, bones);
}

void CAnimationFix::animation_info::UpdateAnims(animation* record, animation* from)
{
	auto animstate = player->GetPlayerAnimState();
	int idx = player->GetIndex();

	if (record->came_from_dormant <= 2)
		record->came_from_dormant++;

	if (!from)
	{
		record->velocity = player->GetVelocity();
		record->didshot = false;
		record->safepoints = false;
		record->apply(player);

		animstate->m_abs_yaw = g_Resolver->ResolverInfo[idx].ResolvedAngle;

		return UpdatePlayer(player);
	}
	else
	{
		int ticks_to_simulate = TIME_TO_TICKS(record->sim_time - from->sim_time);
		if (ticks_to_simulate > 31) ticks_to_simulate = 1;

		// did the player shoot?
		const float& came_from_dormant_time = record->player->CameFromDormantTime();

		if (!from->didshot)
			record->didshot = record->last_shot_time > from->sim_time && record->last_shot_time <= record->sim_time
			&& record->last_shot_time != came_from_dormant_time && from->last_shot_time != came_from_dormant_time;

		record->resolver_mode = ResolverMode[player->GetIndex()];

		const auto velocity_per_tick = (record->velocity - from->velocity) / ticks_to_simulate;
		const auto duck_amount_per_tick = (record->duck - from->duck) / ticks_to_simulate;

		if (ticks_to_simulate <= 1)
		{
			// set velocity and layers.
			record->velocity = player->GetVelocity();

			// apply record.
			record->apply(player);

			animstate->m_abs_yaw = g_Resolver->ResolverInfo[idx].ResolvedAngle;

			// run update.
			return UpdatePlayer(player);
		}
		else {

			float land_time = 0.0f;
			bool is_landed = false;
			bool land_in_cycle = false;

			// check if landed in choke cycle
			if (record->layers[4].m_flCycle < 0.5f && (!(record->flags & FL_ONGROUND) || !(from->flags & FL_ONGROUND)))
			{
				land_time = record->sim_time - (record->layers[4].m_flPlaybackRate * record->layers[4].m_flCycle);
				land_in_cycle = land_time >= from->sim_time;
			}

			bool on_ground = record->flags & FL_ONGROUND;

			for (auto i = 1; i <= ticks_to_simulate; i++)
			{
				const auto simulated_time = from->sim_time + TICKS_TO_TIME(i);

				player->GetVelocity() = (velocity_per_tick * i) + from->velocity;
				player->GetDuckAmount() = (duck_amount_per_tick * i) + from->duck;

				const auto ct = interfaces.global_vars->curtime;
				interfaces.global_vars->curtime = simulated_time;

				if (land_in_cycle && !is_landed)
				{
					if (land_time <= simulated_time)
					{
						is_landed = true;
						on_ground = true;
					}
					else
						on_ground = from->flags & FL_ONGROUND;
				}

				if (on_ground)
					player->GetFlagsPtr() |= FL_ONGROUND;
				else
					player->GetFlagsPtr() &= ~FL_ONGROUND;

				// backup simtime.
				const auto backup_simtime = player->GetSimulationTime();

				// set new simtime.
				player->GetSimulationTime() = simulated_time;

				animstate->m_abs_yaw = g_Resolver->ResolverInfo[idx].ResolvedAngle;

				// run update.
				UpdatePlayer(player);

				// restore old simtime.
				player->GetSimulationTime() = backup_simtime;

				interfaces.global_vars->curtime = ct;
			}
		}
	}
}

void CAnimationFix::FixPvs() {
	for (int i = 1; i <= interfaces.global_vars->maxClients; i++) {
		auto pCurEntity = interfaces.ent_list->GetClientEntity(i);
		if (!pCurEntity
			|| !pCurEntity->IsPlayer()
			|| pCurEntity->EntIndex() == interfaces.engine->GetLocalPlayer())
			continue;

		*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pCurEntity) + 0xA30) = interfaces.global_vars->framecount;
		*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pCurEntity) + 0xA28) = 0;
	}
}