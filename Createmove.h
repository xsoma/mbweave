#include "Hooks.h"
#include "checksum_md5.h"
#include "NetChannelhook.h"
#include "WriteUsercmdDeltaToBuffer.h"
#include "RunCommand.h"

#define shift_ticks 16

bool __stdcall Hooked_CreateMove(float a, CUserCmd* cmd) {
	static auto CreateMove = g_pClientModeHook->GetOriginal< CreateMoveFn >(g_HookIndices[fnva1(hs::Hooked_CreateMove.s().c_str())]);
	csgo->updatelocalplayer();

	if (!cmd || !cmd->command_number || !csgo->local || csgo->DoUnload)
		return CreateMove(interfaces.client_mode, a, cmd);

	csgo->in_cm = true;
	csgo->cmd = cmd;
	csgo->original = cmd->viewangles;

	uintptr_t* pebp;
	__asm mov pebp, ebp;

	csgo->send_packet = true;
	csgo->shift_amount = 0;
	g_Misc->Clantag();
	g_Misc->ProcessMissedShots();
	g_Misc->FixMouseInput();
	INetChannel* pNetChannel = (INetChannel*)csgo->client_state->pNetChannel;

	if (csgo->local->isAlive()) {

		if (csgo->need_to_recharge) {
			cmd->tick_count = INT_MAX;
			const int ticks = /*g_Binds[bind_hide_shots].active ? shift_ticks / 2 :*/ shift_ticks;
			if (++csgo->skip_ticks >= ticks)
			{
				csgo->need_to_recharge = false;
				*(bool*)(*pebp - 0x1C) = true;
			}
			else
				*(bool*)(*pebp - 0x1C) = false;
			return false;
		}

		static bool toggle_hs = false;
		static bool toggle_dt = false;

		if (g_Binds[bind_hide_shots].active) {
			if (!toggle_hs) {
				g_Misc->hide_shots_key = true;
				toggle_hs = true;
			}
		}
		else
			toggle_hs = false;

		if (g_Binds[bind_double_tap].active) {
			if (!toggle_dt) {
				g_Misc->double_tap_key = true;
				toggle_dt = true;
			}
		}
		else
			toggle_dt = false;

		if (g_Misc->double_tap_key && g_Binds[bind_hide_shots].key != g_Binds[bind_double_tap].key)
			g_Misc->hide_shots_key = false;

		if (g_Misc->hide_shots_key && g_Binds[bind_double_tap].key != g_Binds[bind_hide_shots].key)
			g_Misc->double_tap_key = false;


		bool shit = g_Misc->double_tap_enabled && g_Misc->double_tap_key || g_Misc->hide_shots_enabled && g_Misc->hide_shots_key;

		if (csgo->skip_ticks < shift_ticks &&
			shit) {
			csgo->need_to_recharge = true;
		}

		if (g_Misc->hs_works)
			csgo->fixed_tickbase = csgo->local->GetTickBase() - 6;
		else
			csgo->fixed_tickbase = csgo->local->GetTickBase();

		csgo->tick_rate = 1.f / interfaces.global_vars->interval_per_tick;
		csgo->vecUnpredictedVel = csgo->local->GetVelocity();
		csgo->unpdred_tick = interfaces.global_vars->tickcount;
		csgo->weapon = csgo->local->GetWeapon();
		csgo->origin = csgo->local->GetOrigin();
		csgo->unpred_eyepos = csgo->local->GetEyePosition();
		csgo->duck_amount = csgo->local->GetDuckAmount();

		if (csgo->weapon)
		{
			csgo->unpred_inaccuracy = csgo->weapon->GetInaccuracy();
			csgo->weapon_range = []() {
				if (const auto& info = csgo->weapon->GetCSWpnData(); info != nullptr)
					return info->m_flRange;
				return -1.f;
			}();

			csgo->exploits = g_Binds[bind_double_tap].active && g_Misc->double_tap_key
				|| g_Binds[bind_hide_shots].active && g_Misc->hide_shots_key;

			csgo->original_forwardmove = cmd->forwardmove;
			csgo->original_sidemove = cmd->sidemove;

			g_Misc->FakeDuck();
			//Ragebot::Get().DoQuickStop();

			g_Ragebot->DropTarget();
			g_Legit->AutoPistol();
			g_AntiAim->Fakelag();
			
			g_Bunnyhop->Run(csgo->original);

			g_Misc->SlideWalk();

			g_EnginePrediction->update();
			g_EnginePrediction->start(csgo->local, cmd);
			g_Resolver->StoreAntifreestand();

			g_AntiAim->Initialize();
			run_callback("createmove");

			g_NetData->RecordViewmodelValues();

			csgo->weapon_data = csgo->weapon->GetCSWpnData();
			if (csgo->weapon->IsGun())
			{
				csgo->spread = csgo->weapon->GetSpread();
				csgo->innacuracy = csgo->weapon->GetInaccuracy();
				csgo->weaponspread = (csgo->spread + csgo->innacuracy) * 1000.f;
			}
			else
				csgo->weaponspread = 0.f;

			if (vars.misc.knifebot)
				g_Knifebot->Run();

			if (vars.ragebot.enable) {
				g_Ragebot->in_ragebot = true;
				g_Ragebot->Run();
				g_Ragebot->in_ragebot = false;
			}

			csgo->eyepos = csgo->local->GetEyePosition();

			if (vars.legitbot.enable && vars.legitbot.backtrack) {
				g_Legit->Backtrack();
			}
			if (vars.antiaim.enable)
			{
				g_AntiAim->Run();
				if (!g_AutoPeek->has_shot)
					g_AntiAim->Sidemove();
			}

			g_Misc->Doubletap();
			g_Misc->Hideshots();

			g_AutoPeek->Run();

			if (F::Shooting()) {
				csgo->m_shot_command_number = cmd->command_number;
				if (!csgo->fake_duck)
					csgo->send_packet = true;
			}

			csgo->forcing_shot = false;

			if (cmd->command_number >= csgo->m_shot_command_number
				&& csgo->m_shot_command_number >= cmd->command_number - csgo->client_state->iChokedCommands) {
				csgo->forcing_shot = true;
				if (csgo->send_packet)
					csgo->FakeAngle = interfaces.input->m_pCommands[csgo->m_shot_command_number % 150].viewangles;
				else
					csgo->VisualAngle = interfaces.input->m_pCommands[csgo->m_shot_command_number % 150].viewangles;
				//if (!g_Misc->hs_shot)
					csgo->CurAngle = interfaces.input->m_pCommands[csgo->m_shot_command_number % 150].viewangles;
			}
			else {
				if (csgo->send_packet)
					csgo->FakeAngle = csgo->cmd->viewangles;
				else
					csgo->VisualAngle = csgo->cmd->viewangles;
				//if (!g_Misc->hs_shot)
					csgo->CurAngle = csgo->cmd->viewangles;
			}

			if (vars.misc.antiuntrusted)
				csgo->cmd->viewangles = Math::normalize(csgo->cmd->viewangles);

			g_Animfix->UpdateFakeState();
			if (!g_Misc->hs_shot)
				g_Animfix->UpdateRealState();

			g_EnginePrediction->end(csgo->local);

			g_Misc->FixMovement(cmd, csgo->original);

			auto& correct = csgo->c_data.emplace_front();

			correct.command_number = csgo->cmd->command_number;
			correct.choked_commands = csgo->client_state->iChokedCommands + 1;
			correct.tickcount = interfaces.global_vars->tickcount;

			if (csgo->send_packet)
				csgo->choked_number.clear();
			else
				csgo->choked_number.emplace_back(correct.command_number);

			while (csgo->c_data.size() > (int)(2.0f / interfaces.global_vars->interval_per_tick))
				csgo->c_data.pop_back();

			auto& out = csgo->packets.emplace_back();

			out.is_outgoing = csgo->send_packet;
			out.is_used = false;
			out.cmd_number = csgo->cmd->command_number;
			out.previous_command_number = 0;

			while (csgo->packets.size() > (int)(1.0f / interfaces.global_vars->interval_per_tick))
				csgo->packets.pop_front();

			if (!csgo->send_packet && !csgo->game_rules->IsValveDS())
			{
				auto net_channel = csgo->client_state->pNetChannel;

				if (net_channel->iChokedPackets > 0 && !(net_channel->iChokedPackets % 4))
				{
					auto backup_choke = net_channel->iChokedPackets;
					net_channel->iChokedPackets = 0;

					net_channel->SendDatagram();
					--net_channel->iOutSequenceNr;

					net_channel->iChokedPackets = backup_choke;
				}
			}

			g_GrenadePrediction->Tick(csgo->cmd->buttons);
		}
		
		g_Misc->UpdateDormantTime();
		g_Misc->StoreTaserRange();
		static INetChannel* old_net_chan = nullptr;

		if (csgo->local->isAlive()) {
			csgo->ping = interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
			csgo->ping *= 1000.f;
		}
	}
	else {
		csgo->ForceOffAA = false;
		csgo->should_draw_taser_range = false;
	}

	csgo->last_sendpacket = csgo->send_packet;
	csgo->in_cm = false;
	*(bool*)(*pebp - 0x1C) = csgo->send_packet;
	return false;
}
