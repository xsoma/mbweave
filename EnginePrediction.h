#pragma once
#include "hooks.h"

class CEnginePrediction
{
public:
	struct
	{
		float m_flFrametime;
		float m_flCurtime;
		int m_fFlags;
		int m_iTickCount;
		float m_flVelocityModifier;

		Vector m_vecVelocity;
		Vector m_vecOrigin;
	} m_store;

	struct
	{
		int m_iRunCommandTickbase;
		bool m_bOverrideModifier;
	} m_other;


	float predicted_curtime;


	int* m_nPredictionRandomSeed = nullptr;
	int* m_pPredictionPlayer = nullptr;
	//uint32_t* m_nPredictionRandomSeed;
	//c_entity** m_pPredictionPlayer;
	CMoveData				m_move_data;
private:
	bool old_in_prediction;
	bool old_first_prediction;


	using unknown_function_think = char(__thiscall*)(IBasePlayer*, char);
	using PostThinkVPhysics_t = bool(__thiscall*)(IBasePlayer*);
	using SimulatePlayerSimulatedEntities_t = void(__thiscall*)(IBasePlayer*);

	unknown_function_think sub_1019B010;
	PostThinkVPhysics_t PostThinkVPhysics;
	SimulatePlayerSimulatedEntities_t SimulatePlayerSimulatedEntities;

	CUserCmd* save_m_pCurrentCommand;
	int save_m_nPredictionRandomSeed;
	int save_m_flInnacuracy;
	int save_m_flRecoilIndex;

public:
	int m_hConstraintEntity;
	CEnginePrediction()
	{
		ZeroMemory(this, sizeof CEnginePrediction);
	}

	void initalize()
	{
		sub_1019B010 = (unknown_function_think)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), str("55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6 C1 E8")));

		PostThinkVPhysics = (PostThinkVPhysics_t)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), str("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB ? ? ? ? ? 0F 84")));

		SimulatePlayerSimulatedEntities = (SimulatePlayerSimulatedEntities_t)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), str("56 8B F1 57 8B BE ? ? ? ? 83 EF 01 78 74")));

		m_nPredictionRandomSeed = *reinterpret_cast <int**>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), str("A3 ? ? ? ? 66 0F 6E 86")) + 0x1);

		m_pPredictionPlayer = *reinterpret_cast <int**> (csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), str("89 35 ? ? ? ? F3 0F 10 48")) + 0x2);
	}

	void update() {
		interfaces.prediction->Update(csgo->client_state->iDeltaTick, csgo->client_state->iDeltaTick > 0,
			csgo->client_state->nLastCommandAck, csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands);
	}

	void post_think(IBasePlayer* player)
	{
		getvfunc<void(__thiscall*)(void*)>(interfaces.model_cache, 33)(interfaces.model_cache);

		if (player->isAlive()) {
			getvfunc< void(__thiscall*)(void*) >(player, 339)(player); //UpdateCollisionBounds
			if (player->GetFlags() & FL_ONGROUND)
				player->GetFallVelocity() = 0.f;
			if (player->GetSequence() == -1)
				getvfunc< void(__thiscall*)(void*, int) >(player, 218)(player, 0); //SetSequence
			getvfunc< void(__thiscall*)(void*) >(player, 219)(player); //StudioFrameAdvance 
			PostThinkVPhysics(player);
		};

		SimulatePlayerSimulatedEntities(player);

		getvfunc<void(__thiscall*)(void*)>(interfaces.model_cache, 34)(interfaces.model_cache);
	}

	void start(IBasePlayer* player, CUserCmd* m_cmd)
	{
		if (!interfaces.engine->IsInGame() || !player)
			return;

		//	CPrediction::StartCommand
		{
			save_m_nPredictionRandomSeed = *m_nPredictionRandomSeed;

			*m_nPredictionRandomSeed = m_cmd->random_seed;
			*m_pPredictionPlayer = reinterpret_cast <int> (player);

			save_m_pCurrentCommand = *reinterpret_cast<CUserCmd**>(uintptr_t(player) + m_hConstraintEntity - 12);
			*reinterpret_cast<CUserCmd**>(uintptr_t(player) + m_hConstraintEntity - 12) = m_cmd;
			*reinterpret_cast<CUserCmd*>(uintptr_t(player) + 0x3288) = *m_cmd;
		}

		auto weapon = player->GetWeapon();

		if (weapon && !weapon->IsNade() && !weapon->IsKnife())
		{
			save_m_flInnacuracy = weapon->GetInaccuracy();
			save_m_flRecoilIndex = weapon->GetRecoilIndex();
		}

		//	backup player variables
		m_store.m_fFlags = player->GetFlags();
		m_store.m_vecVelocity = player->GetVelocity();

		//	backup globals
		m_store.m_flCurtime = interfaces.global_vars->curtime;
		m_store.m_flFrametime = interfaces.global_vars->frametime;
		m_store.m_iTickCount = interfaces.global_vars->tickcount;


		//	backup
		const auto old_tickbase = player->GetTickBase();
		old_in_prediction = interfaces.prediction->InPrediction;
		old_first_prediction = interfaces.prediction->IsFirstTimePredicted;

		//	set globals correctly
		interfaces.global_vars->curtime = TICKS_TO_TIME(csgo->fixed_tickbase);
		predicted_curtime = interfaces.global_vars->curtime;

		interfaces.global_vars->frametime = interfaces.prediction->EnginePaused ? 0 : interfaces.global_vars->interval_per_tick;
		interfaces.global_vars->tickcount = player->GetTickBase();


		//	setup prediction
		interfaces.prediction->IsFirstTimePredicted = false;
		interfaces.prediction->InPrediction = true;

		if (m_cmd->impulse)
			*reinterpret_cast<uint32_t*>((uintptr_t)player + 0x31FC) = m_cmd->impulse;

		m_cmd->buttons |= *reinterpret_cast<int*>((uintptr_t)player + 0x3334);
		m_cmd->buttons &= ~(*reinterpret_cast<int*>((uintptr_t)player + 0x3330));

		// update button state
		const int buttons = m_cmd->buttons;
		int* player_buttons = player->GetButtons();
		const int buttons_changed = buttons ^ *player_buttons;

		// synchronize m_afButtonLast
		player->GetButtonLast() = *player_buttons;

		// synchronize m_nButtons
		*player->GetButtons() = buttons;

		// synchronize m_afButtonPressed
		player->GetButtonPressed() = buttons & buttons_changed;

		// synchronize m_afButtonReleased
		player->GetButtonReleased() = buttons_changed & ~buttons;


		interfaces.prediction->CheckMovingGround(player, interfaces.global_vars->frametime);

		interfaces.prediction->SetLocalViewAngles(m_cmd->viewangles);

		//	CPrediction::RunPreThink
		if (player->PhysicsRunThink(0))
		{
			getvfunc<void(__thiscall*)(void*)>(player, 317)(player);
		}

		//	CPrediction::RunThink
		{
			auto m_nNextThinkTick = *reinterpret_cast<int32_t*>((uintptr_t)player + 0xFC);
			if (m_nNextThinkTick != -1 &&
				m_nNextThinkTick > 0 &&
				m_nNextThinkTick <= player->GetTickBase())
			{
				*reinterpret_cast<int32_t*>((uintptr_t)player + 0xFC) = -1;

				sub_1019B010(player, 0);

				getvfunc<void(__thiscall*)(void*)>(player, 138)(player);
			}
		}


		//	set host
		interfaces.move_helper->SetHost(player);

		// start track prediction errors
		interfaces.game_movement->StartTrackPredictionErrors(player);

		//	setup input
		interfaces.prediction->SetupMove(player, m_cmd, interfaces.move_helper, &m_move_data);

		//	run movement
		interfaces.game_movement->ProcessMovement(player, &m_move_data);

		//	finish prediction
		interfaces.prediction->FinishMove(player, m_cmd, &m_move_data);

		//	invoke impact functions
		interfaces.move_helper->ProcessImpacts();

		//	CPrediction::RunPostThink
		{
			post_think(player);
		}

		player->GetTickBasePtr() = old_tickbase;

		interfaces.game_movement->FinishTrackPredictionErrors(player);
		interfaces.move_helper->SetHost(nullptr);

		if (weapon && !weapon->IsNade() && !weapon->IsKnife())
			weapon->UpdateAccuracyPenalty();
	}

	void end(IBasePlayer* player)
	{
		interfaces.prediction->IsFirstTimePredicted = old_first_prediction;
		interfaces.prediction->InPrediction = old_in_prediction;

		interfaces.global_vars->curtime = m_store.m_flCurtime;
		interfaces.global_vars->frametime = m_store.m_flFrametime;
		interfaces.global_vars->tickcount = m_store.m_iTickCount;

		//	CPrediction::FinishCommand
		{
			*reinterpret_cast<CUserCmd**>(uintptr_t(player) + m_hConstraintEntity - 12) = save_m_pCurrentCommand;
			*m_nPredictionRandomSeed = save_m_nPredictionRandomSeed;
			*m_pPredictionPlayer = 0;
		}

		interfaces.game_movement->Reset();

		/*if (!csgo::prediction->m_engine_paused && csgo::vars->frametime > 0)
			player->m_nTickBase()++;*/

		auto weapon = player->GetWeapon();
		if (weapon && !weapon->IsNade() && !weapon->IsKnife())
		{
			weapon->GetRecoilIndex() = save_m_flRecoilIndex;
			weapon->GetAccuracyPenalty() = save_m_flInnacuracy;
		}
	}
};

class CNetData {
private:
	class StoredData_t {
	public:
		int			m_tickbase;
		Vector		m_punch;
		Vector		m_punch_vel;
		Vector		m_view_offset;
		float		m_velocity_modifier;

	public:
		__forceinline StoredData_t() : m_tickbase{ }, m_punch{ }, m_punch_vel{ }, m_view_offset{ }, m_velocity_modifier{ } {};
	};
	std::array< StoredData_t, 150 > m_data;

public:
	struct ViewModelData_t {
		CBaseHandle m_hWeapon = 0;

		int m_nViewModelIndex = 0;
		int m_nAnimationParity = 0;
		int m_nSequence = 0;
		float networkedCycle = 0.0f;
		float animationTime = 0.0f;
	}viewModelData;

	void RecordViewmodelValues();
	void ApplyViewmodelValues();


	void Store();
	void Apply();
	void Reset();
};
