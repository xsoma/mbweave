#include "Features.h"

void CMisc::PreverseKillFeed(bool roundStart)
{
	static auto hud_ptr = *(DWORD**)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element =
		reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));

	if (!find_hud_element || !hud_ptr)
		return;

	static auto nextUpdate = 0.0f;

	if (roundStart || !vars.visuals.preverse_killfeed) {
		nextUpdate = interfaces.global_vars->realtime + 10.0f;
		return;
	}

	if (nextUpdate > interfaces.global_vars->realtime)
		return;

	nextUpdate = interfaces.global_vars->realtime + 2.0f;

	const auto deathNotice = std::uintptr_t(find_hud_element(hud_ptr, str("CCSGO_HudDeathNotice")));
	if (!deathNotice)
		return;

	const auto deathNoticePanel = (*(UIPanel**)(*reinterpret_cast<std::uintptr_t*>(deathNotice - 20 + 88) + sizeof(std::uintptr_t)));

	const auto childPanelCount = deathNoticePanel->GetChildCount();

	for (int i = 0; i < childPanelCount; ++i) {
		const auto child = deathNoticePanel->GetChild(i);
		if (!child)
			continue;

		if (child->HasClass(str("DeathNotice_Killer")))
			child->SetAttributeFloat(str("SpawnTime"), interfaces.global_vars->curtime);
	}
}

void CMisc::FixMovement(CUserCmd* cmd, Vector& ang)
{
	if (!csgo->local)
		return;

	Vector  move, dir;
	float   delta, len;
	Vector   move_angle;

	if (!(csgo->local->GetFlags() & FL_ONGROUND) && cmd->viewangles.z != 0 && cmd->buttons & IN_ATTACK)
		cmd->sidemove = 0;

	move = { cmd->forwardmove, cmd->sidemove, 0 };

	len = move.NormalizeMovement();

	if (!len)
		return;

	Math::VectorAngles(move, move_angle);

	delta = (cmd->viewangles.y - ang.y);

	move_angle.y += delta;

	Math::AngleVectors(move_angle, &dir);

	dir *= len;

	if (csgo->local->GetMoveType() == MOVETYPE_LADDER) {
		if (cmd->viewangles.x >= 45 && ang.x < 45 && std::abs(delta) <= 65)
			dir.x = -dir.x;

		cmd->forwardmove = dir.x;
		cmd->sidemove = dir.y;

		if (cmd->forwardmove > 200)
			cmd->buttons |= IN_FORWARD;

		else if (cmd->forwardmove < -200)
			cmd->buttons |= IN_BACK;

		if (cmd->sidemove > 200)
			cmd->buttons |= IN_MOVERIGHT;

		else if (cmd->sidemove < -200)
			cmd->buttons |= IN_MOVELEFT;
	}
	else {
		if (cmd->viewangles.x < -90 || cmd->viewangles.x > 90)
			dir.x = -dir.x;

		cmd->forwardmove = dir.x;
		cmd->sidemove = dir.y;
	}

	cmd->forwardmove = clamp(cmd->forwardmove, -450.f, 450.f);
	cmd->sidemove = clamp(cmd->sidemove, -450.f, 450.f);
	cmd->upmove = clamp(cmd->upmove, -320.f, 320.f);
}

void CMisc::FixMouseInput() 
{
	static Vector delta_viewangles{ };
	Vector delta = csgo->cmd->viewangles - delta_viewangles;
	delta.Normalize();

	static ConVar* sensitivity = interfaces.cvars->FindVar(str("sensitivity"));

	if (!sensitivity)
		return;

	if (delta.x != 0.f) {
		static ConVar* m_pitch = interfaces.cvars->FindVar(str("m_pitch"));

		if (!m_pitch)
			return;

		int final_dy = static_cast<int>((delta.x / m_pitch->GetFloat()) / sensitivity->GetFloat());
		if (final_dy <= 32767) {
			if (final_dy >= -32768) {
				if (final_dy >= 1 || final_dy < 0) {
					if (final_dy <= -1 || final_dy > 0)
						final_dy = final_dy;
					else
						final_dy = -1;
				}
				else {
					final_dy = 1;
				}
			}
			else {
				final_dy = 32768;
			}
		}
		else {
			final_dy = 32767;
		}

		csgo->cmd->mousedy = static_cast<short>(final_dy);
	}

	if (delta.y != 0.f) {
		static ConVar* m_yaw = interfaces.cvars->FindVar(str("m_yaw"));

		if (!m_yaw)
			return;

		int final_dx = static_cast<int>((delta.y / m_yaw->GetFloat()) / sensitivity->GetFloat());
		if (final_dx <= 32767) {
			if (final_dx >= -32768) {
				if (final_dx >= 1 || final_dx < 0) {
					if (final_dx <= -1 || final_dx > 0)
						final_dx = final_dx;
					else
						final_dx = -1;
				}
				else {
					final_dx = 1;
				}
			}
			else {
				final_dx = 32768;
			}
		}
		else {
			final_dx = 32767;
		}

		csgo->cmd->mousedx = static_cast<short>(final_dx);
	}

	delta_viewangles = csgo->cmd->viewangles;
}

void CMisc::SlideWalk()
{
	if (!csgo->local->isAlive())
		return;

	if (csgo->local->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (!(csgo->local->GetFlags() & FL_ONGROUND))
		return;

	if (g_AntiAim->ShouldAA() && !vars.misc.slidewalk)
		csgo->cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
}

bool CMisc::IsChatOpened() {
	if (!csgo->local)
		return false;

	static auto hud_ptr = *(DWORD**)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), 
		str("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element = 
		reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), 
			str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));

	if (!find_hud_element || !hud_ptr)
		return false;

	auto chat = find_hud_element(hud_ptr, str("CCSGO_HudChat"));
	if (!chat)
		return false;

	bool is_in_chat = *(bool*)((DWORD)chat + 0x58);

	if (is_in_chat)
		return true;

	return false;
}

void CMisc::CopyCommand(CUserCmd* cmd, int tickbase_shift)
{
	static auto cl_forwardspeed = interfaces.cvars->FindVar(str("cl_forwardspeed"));
	static auto cl_sidespeed = interfaces.cvars->FindVar(str("cl_sidespeed"));

	if (vars.ragebot.dt_teleport)
	{
		Vector vMove(cmd->forwardmove, cmd->sidemove, cmd->upmove);
		float flSpeed = sqrt(vMove.x * vMove.x + vMove.y * vMove.y), flYaw;
		Vector vMove2;
		Math::VectorAngles(vMove, vMove2);
		vMove2.Normalize();
		flYaw = DEG2RAD(cmd->viewangles.y - csgo->original.y + vMove2.y);
		cmd->forwardmove = cos(flYaw) * flSpeed;
		cmd->sidemove = sin(flYaw) * flSpeed;
	}
	else {
		cmd->forwardmove = 0.0f;
		cmd->sidemove = 0.0f;
	}

	if (g_Binds[bind_peek_assist].active)
		g_Ragebot->FastStop();

	auto commands_to_add = 0;

	do
	{
		auto sequence_number = commands_to_add + cmd->command_number;

		auto command = interfaces.input->GetUserCmd(sequence_number);
		auto verified_command = interfaces.input->GetVerifiedUserCmd(sequence_number);

		memcpy(command, cmd, sizeof(CUserCmd));

		if (command->tick_count != INT_MAX && csgo->client_state->iDeltaTick > 0)
			interfaces.prediction->Update(
				csgo->client_state->iDeltaTick, true,
				csgo->client_state->nLastCommandAck,
				csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands
			);

		command->command_number = sequence_number;
		command->hasbeenpredicted = command->tick_count != INT_MAX;

		++csgo->client_state->iChokedCommands;

		if (csgo->client_state->pNetChannel)
		{
			++csgo->client_state->pNetChannel->iChokedPackets;
			++csgo->client_state->pNetChannel->iOutSequenceNr;
		}

		command->viewangles = Math::normalize(command->viewangles);

		memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd));
		verified_command->m_crc = command->GetChecksum();

		++commands_to_add;
	} while (commands_to_add != tickbase_shift);

	interfaces.prediction->PreviousAckHadErrors = true;
	interfaces.prediction->CommandsPredicted = 0;
}

// to-do: все таки понять принцип работы эксплоитов 
// а то так мы никуда и не продвинемся ;)
// @opai

// за себя говори, я всё понимаю, просто лень чет думать
// @ekzi

__forceinline void ResetValue()
{
	csgo->skip_ticks = 0;
	csgo->shift_amount = 0;
}

bool CMisc::Doubletap()
{
	static int last_doubletap = 0;
	double_tap_enabled = true;
	static auto recharge_double_tap = false;

	if (recharge_double_tap)
	{
		recharge_double_tap = false;
		recharging_double_tap = true;
		ResetValue();
		return false;
	}

	if (recharging_double_tap)
	{
		if ((g_Ragebot->IsAbleToShoot(vars.ragebot.recharge_time) || fabs(csgo->fixed_tickbase - last_doubletap) > TIME_TO_TICKS(0.5f)) && !g_Ragebot->ShouldWork)
		{
			last_doubletap = 0;

			recharging_double_tap = false;
			double_tap_key = true;
			dt_bullets = 0;
		}
		else if (csgo->cmd->buttons & IN_ATTACK) {
			last_doubletap = csgo->fixed_tickbase;
			dt_bullets++;
		}
	}

	if (!vars.ragebot.enable)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		ResetValue();
		return false;
	}

	if (!g_Binds[bind_double_tap].active)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		//if (!g_Binds[bind_hide_shots].active)
		//	ResetValue();
		return false;
	}

	//if (double_tap_key && g_Binds[bind_double_tap].key != g_Binds[bind_hide_shots].key)
	//	hide_shots_key = false;

	if (!double_tap_key)
	{
		double_tap_enabled = false;
		ResetValue();
		return false;
	}

	if (csgo->game_rules->IsFreezeTime() || csgo->local->HasGunGameImmunity() || csgo->local->GetFlags() & FL_FROZEN)
	{
		double_tap_enabled = false;
		ResetValue();
		return false;
	}

	if (csgo->game_rules->IsValveDS())
	{
		double_tap_enabled = false;
		ResetValue();
		return false;
	}

	if (csgo->fake_duck)
	{
		double_tap_enabled = false;
		ResetValue();
		return false;
	}

	if (csgo->skip_ticks <= 8) {
		ResetValue();
		csgo->need_to_recharge = true;
		return false;
	}

	auto max_tickbase_shift = csgo->weapon->GetMaxTickbaseShift();
	bool can_dt =
		!csgo->weapon->IsMiscWeapon()
		&& csgo->weapon->IsGun()
		&& csgo->weapon->GetItemDefinitionIndex() != WEAPON_ZEUSX27
		&& csgo->weapon->GetItemDefinitionIndex() != WEAPON_REVOLVER;

	bool is_firing = csgo->cmd->buttons & IN_ATTACK;

	if (can_dt && is_firing)
	{
		auto next_command_number = csgo->cmd->command_number + 1;
		auto user_cmd = interfaces.input->GetUserCmd(next_command_number);

		memcpy(user_cmd, csgo->cmd, sizeof(CUserCmd));
		user_cmd->command_number = next_command_number;

		CopyCommand(user_cmd, max_tickbase_shift);

		recharge_double_tap = true;
		double_tap_enabled = false;
		double_tap_key = false;
		csgo->send_packet = true;
		last_doubletap = csgo->fixed_tickbase;
		dt_bullets++;
	}
	else if (can_dt)
		csgo->shift_amount = max_tickbase_shift;

	return true;
}

void CMisc::Hideshots()
{
	hs_shot = false;
	hs_works = false;
	hide_shots_enabled = false;
	if (g_Binds[bind_double_tap].active)
		return;

	hide_shots_enabled = true;

	if (!vars.ragebot.enable)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		//ResetValue();

		return;
	}

	if (!g_Binds[bind_hide_shots].active)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		ResetValue();

		return;
	}

	//if (csgo->skip_ticks == 0) {
	//	csgo->need_to_recharge = true;
	//	return;
	//}

	if (!g_Binds[bind_double_tap].active) {
		if (csgo->fake_duck || csgo->game_rules->IsFreezeTime() || csgo->local->HasGunGameImmunity() || csgo->local->GetFlags() & FL_FROZEN)
		{
			hide_shots_enabled = false;
			ResetValue();
			return;
		}
	}
	
	hs_works = true;

	if (!csgo->weapon->IsMiscWeapon() && csgo->weapon->GetItemDefinitionIndex() != weapon_revolver) {
		if (csgo->send_packet
			&& (g_Ragebot->IsAbleToShoot() && csgo->cmd->buttons & IN_ATTACK)) {
			csgo->shift_amount = 6;
			hs_shot = true;
		}
		else {
			if (!g_Ragebot->shot)
				csgo->cmd->buttons &= ~IN_ATTACK;
		}
	}
}

void CMisc::UpdateDormantTime() {
	for (int i = 1; i < 65; ++i)
	{
		auto ent = interfaces.ent_list->GetClientEntity(i);
		if (!ent)
			continue;
		if (ent->IsDormant() || !ent->isAlive() || !ent->IsPlayer()
			|| ent->EntIndex() == interfaces.engine->GetLocalPlayer()
			|| ent->GetTeam() == csgo->local->GetTeam()) {
			csgo->dormant_time[ent->GetIndex()] = -1.f;
			continue;
		}
		if (csgo->dormant_time[ent->GetIndex()] == -1.f)
			csgo->dormant_time[ent->GetIndex()] = ent->GetSimulationTime();
	}
}

void CMisc::Clantag()
{
	auto SetClanTag = [](const char* tag, const char* name)
	{
		static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>((DWORD)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
			hs::set_clantag.s().c_str())));
		pSetClanTag(tag, name);
	};

	auto Marquee = [](std::string& clantag)
	{
		std::string temp = clantag;
		clantag.erase(0, 1);
		clantag += temp[0];
	};

	static bool Reset = true;

	if (vars.visuals.clantagspammer)
	{
		Reset = false;
		static float oldTime;
		float flCurTime = TICKS_TO_TIME(csgo->fixed_tickbase);
		static float flNextTimeUpdate = 0;
		static int iLastTime;

		float latency = interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		if (int(interfaces.global_vars->curtime * 2 + latency) != iLastTime)
		{
			if (flNextTimeUpdate <= flCurTime || flNextTimeUpdate - flCurTime > 1.f)
			{
				switch (int(interfaces.global_vars->curtime * 2.4) % 10) {
				case 0: SetClanTag(hs::clantag1.s().c_str(), hs::weave_su.s().c_str()); break;
				case 1: SetClanTag(hs::clantag2.s().c_str(), hs::weave_su.s().c_str()); break;
				case 2: SetClanTag(hs::clantag3.s().c_str(), hs::weave_su.s().c_str()); break;
				case 3: SetClanTag(hs::clantag4.s().c_str(), hs::weave_su.s().c_str()); break;
				case 4: SetClanTag(hs::clantag5.s().c_str(), hs::weave_su.s().c_str()); break;
				case 5: SetClanTag(hs::clantag6.s().c_str(), hs::weave_su.s().c_str()); break;
				case 6: SetClanTag(hs::clantag7.s().c_str(), hs::weave_su.s().c_str()); break;
				case 7: SetClanTag(hs::clantag8.s().c_str(), hs::weave_su.s().c_str()); break;
				case 8: SetClanTag(hs::clantag9.s().c_str(), hs::weave_su.s().c_str()); break;
				case 9: SetClanTag(hs::clantag10.s().c_str(), hs::weave_su.s().c_str()); break;
				}
			}
			iLastTime = int(interfaces.global_vars->curtime * 2 + latency);
		}
	}
	else
	{
		if (!Reset)
		{
			static int iLastTime;

			float latency = interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
			if (int(interfaces.global_vars->curtime * 2 + latency) != iLastTime)
			{
				SetClanTag(str(""), str(""));
				iLastTime = int(interfaces.global_vars->curtime * 2 + latency);
			}
			Reset = true;
		}
	}
}

void CMisc::ProcessMissedShots()
{
	if (shot_snapshots.empty())
		return;

	auto& snapshot = shot_snapshots.front();
	const auto& time = TICKS_TO_TIME(csgo->fixed_tickbase);

	if (fabs(time - snapshot.time) > 1.f)
	{
		if (vars.visuals.eventlog & 8 && !snapshot.bullet_impact) 
			Msg(str("Unregistered shot"), vars.visuals.eventlog_color);
		
		shot_snapshots.erase(shot_snapshots.begin());
		return;
	}

	if (snapshot.first_processed_time != -1.f) {
		if (snapshot.damage == -1 && snapshot.weapon_fire && snapshot.bullet_impact && snapshot.record.player) {
			bool spread = false;
			bool sp_spread = false;
			if (snapshot.record.player) {
				const auto studio_model = interfaces.models.model_info->GetStudioModel(snapshot.record.player->GetModel());
				int idx = snapshot.record.player->EntIndex();

				if (studio_model)
				{
					const auto angle = Math::CalculateAngle(snapshot.start, snapshot.impact);
					Vector forward;
					Math::AngleVectors(angle, forward);
					const auto end = snapshot.impact + forward * 2000.f;
					if (!CanHitHitbox(snapshot.start, end, &snapshot.record, snapshot.hitbox, snapshot.record.bones))
						spread = true;
					if (snapshot.record.safepoints) {
						if (!CanHitHitbox(snapshot.start, end, &snapshot.record, snapshot.hitbox, snapshot.record.unresolved_bones)
							|| !CanHitHitbox(snapshot.start, end, &snapshot.record, snapshot.hitbox, snapshot.record.inversed_bones))
							sp_spread = true;
					}
				}
			}
			auto& new_hitlog = csgo->custom_hitlog.emplace_back();
			new_hitlog.name = snapshot.entity->GetName();
			new_hitlog.backtrack = snapshot.backtrack;
			new_hitlog.hitbox = snapshot.hitbox;
			new_hitlog.resolver = snapshot.resolver_mode;
			new_hitlog.spread = spread || sp_spread;

			if (!spread && !sp_spread)
				csgo->actual_misses[snapshot.entity->GetIndex()]++;

			if (vars.visuals.eventlog & 4) {
				if (!spread && !sp_spread) {
					if (snapshot.record.safepoints && !snapshot.resolver_info.LowDelta)
						Msg(str("Missed shot due to unknown reason"), vars.visuals.eventlog_color);
					else
					{
						g_Resolver->RemoveHitInfo(snapshot.entity);
						Msg(str("Missed shot due to animation desync"), vars.visuals.eventlog_color);
					}
				}
				else {
					if (snapshot.hitchance == 1.f)
						Msg(str("Missed shot due to prediction error"), vars.visuals.eventlog_color);
					else
						Msg(str("Missed shot due to spread"), vars.visuals.eventlog_color);
				}
			}

			shot_snapshots.erase(shot_snapshots.begin());
		}
	}
}

void CMisc::FakeDuck()
{
	if (vars.misc.restrict_type == 0) {
		csgo->fake_duck = false;
		return;
	}

	if (csgo->game_rules->IsFreezeTime()
		|| csgo->local->HasGunGameImmunity() 
		|| csgo->local->GetFlags() & FL_FROZEN
		|| csgo->cmd->buttons & IN_JUMP 
		|| !(csgo->local->GetFlags() & FL_ONGROUND) 
		|| !vars.antiaim.enable) {
		csgo->fake_duck = false;
		return;
	}

	csgo->cmd->buttons |= IN_BULLRUSH;

	if (g_Binds[bind_fake_duck].active)
	{
		csgo->fake_duck = true;

		if (csgo->client_state->iChokedCommands <= 6)
			csgo->cmd->buttons &= ~IN_DUCK;
		else
			csgo->cmd->buttons |= IN_DUCK;

		csgo->send_packet = csgo->client_state->iChokedCommands >= 14;
	}
	else
		csgo->fake_duck = false;
}

void CMisc::StoreTaserRange()
{
	Vector prev_scr_pos{ 0, 0, 0 };
	Vector scr_pos{ 0, 0, 0 };

	if (!csgo->local || !csgo->local->isAlive())
		return;

	auto local_weapon = csgo->weapon;
	if (!local_weapon || local_weapon->GetItemDefinitionIndex() != weapon_taser) {
		csgo->should_draw_taser_range = false;
		return;
	}

	csgo->should_draw_taser_range = true;

	float step = PI * 2.0f / 105.f;

	float rad = local_weapon->GetCSWpnData()->m_flRange - 15.f;

	Vector origin = csgo->local->GetRenderOrigin() + Vector(0, 0, 50);
	size_t i = 0;
	for (float rotation = 0; rotation <= (PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;
		filter.pSkip = csgo->local;
		ray.Init(origin, pos);

		interfaces.trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace);
		csgo->taser_ranges[i++] = std::pair(trace.endpos, trace.endpos + Vector(0.f, 0.f, -10.f));
	}
}

void CMisc::Ragdolls()
{
	if (!vars.visuals.ragdoll_force)
		return;

	for (auto i = 1; i <= interfaces.ent_list->GetHighestEntityIndex(); ++i)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e)
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != g_ClassID->CCSRagdoll)
			continue;

		e->GetVecForce() = Vector(800000.0f, 800000.0f, 800000.0f);
	}
}