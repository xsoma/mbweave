#include "Hooks.h"
#include "Features.h"

bool animation::is_valid(float range = .2f, float max_unlag = .2f)
{
	if (!interfaces.engine->GetNetChannelInfo() || !valid)
		return false;

	const auto correct = std::clamp(interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)
		+ interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING)
		+ g_Ragebot->LerpTime(), 0.f, max_unlag);

	float curtime = csgo->local->isAlive() ? TICKS_TO_TIME(csgo->fixed_tickbase) : interfaces.global_vars->curtime;

	return fabsf(correct - (curtime - sim_time)) < range && correct < 1.f;
}

animation::animation(IBasePlayer* player)
{
	const auto weapon = player->GetWeapon();
	safepoints = false;

	this->player = player;

	index = player->GetIndex();
	dormant = player->IsDormant();
	velocity = player->GetVelocity();
	origin = player->GetOrigin();
	abs_origin = player->GetAbsOrigin();
	obb_mins = player->GetMins();
	obb_maxs = player->GetMaxs();
	std::memcpy(layers, player->GetAnimOverlays(), sizeof(CAnimationLayer) * 13);
	poses = player->m_flPoseParameter();
	anim_state = player->GetPlayerAnimState();
	sim_time = player->GetSimulationTime();
	interp_time = 0.f;
	priority = -1;
	came_from_dormant = -1;
	last_shot_time = weapon ? weapon->GetLastShotTime() : 0.f;
	duck = player->GetDuckAmount();
	lby = player->GetLBY();
	eye_angles = player->GetEyeAngles();
	abs_ang = player->GetAbsAngles();
	flags = player->GetFlags();
	eflags = player->GetEFlags();
	effects = player->GetEffects();

	land_time = 0.0f;

	is_landed = false;
	land_in_cycle = false;

	didshot = false;
	valid = true;
}

animation::animation(IBasePlayer* player, Vector last_reliable_angle) : animation(player)
{
	this->last_reliable_angle = last_reliable_angle;
}

void animation::restore(IBasePlayer* player) const
{
	player->GetVelocity() = velocity;
	player->GetFlagsPtr() = flags;
	player->GetEFlags() = eflags;
	player->GetDuckAmount() = duck;
	std::memcpy(player->GetAnimOverlays(), layers, sizeof(CAnimationLayer) * 13);
	player->GetLBY() = lby;
	player->GetOrigin() = origin;
	player->SetAbsOrigin(abs_origin);
}

void animation::apply(IBasePlayer* player) const
{
	player->SetPoseParameter(poses);
	player->GetVelocity() = velocity;
	player->GetFlagsPtr() = flags;
	player->GetEFlags() = eflags;
	player->GetDuckAmount() = duck;
	std::memcpy(player->GetAnimOverlays(), layers, sizeof(CAnimationLayer) * 13);
	player->GetLBY() = lby;
	player->GetOrigin() = origin;
	player->SetAbsOrigin(abs_origin);
	if (player->GetPlayerAnimState())
		player->SetAnimState(anim_state);
}

void CAnimationFix::UpdatePlayers()
{
	if (!interfaces.engine->IsInGame())
		return;

	const auto local = csgo->local;

	// erase outdated entries
	for (auto it = animation_infos.begin(); it != animation_infos.end();) {
		auto player = reinterpret_cast<IBasePlayer*>(interfaces.ent_list->GetClientEntityFromHandle(it->first));

		if (!player || player != it->second->player || !player->isAlive()
			|| !local)
		{
			if (player)
				player->GetClientSideAnims() = true;
			it = animation_infos.erase(it);
		}
		else
			it = next(it);
	}

	if (!local)
	{
		for (auto i = 1; i <= interfaces.engine->GetMaxClients(); ++i) {
			const auto entity = interfaces.ent_list->GetClientEntity(i);
			if (entity && entity->IsPlayer())
				entity->GetClientSideAnims() = true;
		}
	}

	for (auto i = 1; i <= interfaces.engine->GetMaxClients(); ++i) {
		const auto entity = interfaces.ent_list->GetClientEntity(i);
		if (!entity || !entity->IsPlayer())
			continue;

		if (!entity->isAlive())
			continue;

		if (entity->IsDormant()) {
			csgo->CameFromDormant[entity->EntIndex()] = -1;
			continue;
		}

		if (entity == local)
			continue;

		if (entity != local && entity->GetTeam() == local->GetTeam()) {
			csgo->EnableBones = entity->GetClientSideAnims() = true;
			continue;
		}

		if (animation_infos.find(entity->GetRefEHandle()) == animation_infos.end())
			animation_infos.insert_or_assign(entity->GetRefEHandle(), new animation_info(entity, {}));
	}

	// run post update
	for (auto& info : animation_infos)
	{
		auto& _animation = info.second;
		const auto player = _animation->player;

		for (auto it = _animation->frames.rbegin(); it != _animation->frames.rend();) {
			if (!it->is_valid(0.2f + TICKS_TO_TIME(17)))
				it = decltype(it) {
				info.second->frames.erase(next(it).base())
			};
			else
				it = next(it);
		}

		if (g_Resolver->Do(_animation->player)) {
			if (auto state = player->GetPlayerAnimState(); state != nullptr)
				state->m_abs_yaw = g_Resolver->ResolverInfo[player->EntIndex()].ResolvedAngle;
		}
		else {
			if (auto state = player->GetPlayerAnimState(); state != nullptr)
				g_Resolver->ResolverInfo[player->EntIndex()].ResolvedAngle = state->m_abs_yaw;
		}

		// have we already seen this update?
		if (player->GetSimulationTime() != player->CameFromDormantTime()) {
			if (player->GetSimulationTime() <= player->GetOldSimulationTime())
				continue;
		}

		// reset animstate
		if (_animation->last_spawn_time != player->GetSpawnTime())
		{
			const auto state = player->GetPlayerAnimState();
			if (state)
				player->ResetAnimationState(state);

			_animation->last_spawn_time = player->GetSpawnTime();
		}

		// grab previous
		animation* previous = nullptr;

		if (!_animation->frames.empty() && !_animation->frames.front().dormant
			&& TIME_TO_TICKS(player->GetSimulationTime() - _animation->frames.front().sim_time) <= 17)
			previous = &_animation->frames.front();

		// store server record
		auto& record = _animation->frames.emplace_front(player, info.second->last_reliable_angle);

		animation* backup = new animation(player);
		backup->apply(player);

		record.build_inversed_bones(player);
		record.build_unresolved_bones(player);

		_animation->UpdateAnims(&record, previous);

		record.build_server_bones(player);

		backup->restore(player);
		delete backup;
	}
}

CAnimationFix::animation_info* CAnimationFix::get_animation_info(IBasePlayer* player)
{
	auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end())
		return nullptr;

	return info->second;
}

bool animation::is_valid_extended()
{
	return is_valid();
}

animation* CAnimationFix::get_latest_animation(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second->frames.empty())
		return nullptr;

	for (auto it = info->second->frames.begin(); it != info->second->frames.end(); it = next(it)) {
		if ((it)->is_valid_extended()) {
			return &*it;
		}
	}

	return nullptr;
}

std::vector<animation*> CAnimationFix::get_valid_animations(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	std::vector<animation*> ret = {};

	if (info == animation_infos.end() || info->second->frames.empty())
		return ret;
	Vector last_origin = Vector(0, 0, 0);
	for (auto it = info->second->frames.begin(); it != info->second->frames.end(); it = next(it)) {
		if ((it)->is_valid_extended()) {
			float diff = 0.f;
			if (it != info->second->frames.begin() && last_origin != Vector(0, 0, 0)) {
				diff = it->origin.DistTo(last_origin);
			}
			if (diff > 25.f || it->eye_angles.x <= 25.f)
				ret.emplace_back(&*it);

			last_origin = it->origin;
		}
	}

	return ret;
}

animation* CAnimationFix::get_oldest_animation(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second->frames.empty())
		return nullptr;

	for (auto it = info->second->frames.rbegin(); it != info->second->frames.rend(); it = next(it)) {
		if ((it)->is_valid_extended()) {
			return &*it;
		}
	}

	return nullptr;
}

animation* CAnimationFix::get_latest_firing_animation(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second->frames.empty())
		return nullptr;

	for (auto it = info->second->frames.begin(); it != info->second->frames.end(); it = next(it))
		if ((it)->is_valid_extended() && (it)->didshot)
			return &*it;

	return nullptr;
}