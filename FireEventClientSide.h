#pragma once
#include "Hooks.h"
#include "Hitmarker.h"
#include "AutoPeek.h"
#include "hitsounds.h"
#include "Dormant.h"
#pragma comment(lib,"Winmm.lib")

extern std::vector<IGameEventListener2*> g_pGameEventManager;
class CGameEventListener : public IGameEventListener2
{
private:
	std::string eventName;
	Fucntion Call;
	bool server;
public:
	CGameEventListener() : eventName(str("")), Call(nullptr), server(false)
	{
	}

	CGameEventListener(const std::string& gameEventName, const Fucntion Func, bool Server) : eventName(gameEventName), Call(Func), server(Server)
	{
		this->RegListener();
	}

	virtual void FireGameEvent(IGameEvent* event)
	{
		this->Call(event);
	}

	virtual int IndicateEventHandling(void)
	{
		return 0x2A;
	}

	int GetEventDebugID() override
	{
		return 0x2A;
	}

	void RegListener()
	{
		static bool first = true;
		if (!first)
			return;
		if (interfaces.event_manager->AddListener(this, this->eventName.c_str(), server))
			g_pGameEventManager.push_back(this);
	}
};

#define REG_EVENT_LISTENER(p, e, n, s) p = new CGameEventListener(n, e, s)


player_info_t GetInfo(int Index)
{
	player_info_t Info;
	interfaces.engine->GetPlayerInfo(Index, &Info);
	return Info;
}

std::string HitgroupToName(int hitgroup)
{
	switch (hitgroup)
	{
	case HITGROUP_GENERIC: return hs::h_generic.s();
	case HITGROUP_HEAD: return hs::head.s();
	case HITGROUP_CHEST: return hs::chest.s();
	case HITGROUP_STOMACH: return hs::stomach.s();
	case HITGROUP_LEFTARM: return hs::left_arm.s();
	case HITGROUP_RIGHTARM: return hs::right_arm.s();
	case HITGROUP_LEFTLEG: return hs::left_leg.s();
	case HITGROUP_RIGHTLEG: return hs::right_leg.s();
	case HITGROUP_NECK: return str("neck");
	case HITGROUP_GEAR: return hs::gear.s();
	default: return str("hitgroup_") + std::to_string(hitgroup);
	}
}

void LogHits(IGameEvent* Event)
{
	int attackerid = Event->GetInt(str("attacker"));
	int entityid = interfaces.engine->GetPlayerForUserID(attackerid);
	if (entityid == interfaces.engine->GetLocalPlayer())
	{
		int nUserID = Event->GetInt(str("attacker"));
		int nDead = Event->GetInt(str("userid"));

		if (nUserID || nDead)
		{
			player_info_t killed_info = GetInfo(interfaces.engine->GetPlayerForUserID(nDead));

			if (vars.visuals.hitmarker & 1)
				csgo->last_hit_time = interfaces.global_vars->realtime;
			else
				csgo->last_hit_time = 0.f;

			int hitgroup = Event->GetInt(str("hitgroup"));

			if (!shot_snapshots.empty())
			{
				auto snapshot = shot_snapshots.front();
				if (snapshot.entity == interfaces.ent_list->GetClientEntity(interfaces.engine->GetPlayerForUserID(nDead)))
				{
					const auto angle = Math::CalculateAngle(snapshot.start, snapshot.impact);
					Vector forward;
					Math::AngleVectors(angle, forward);
					const auto end = snapshot.impact + forward * 2000.f;

					int idx = snapshot.record.player->EntIndex();

					const bool& hittable_in_head = CanHitHitbox(snapshot.start, end, &snapshot.record, 0, snapshot.record.bones);

					if (snapshot.hitbox == 0
						&& hitgroup != HITGROUP_HEAD
						&& hittable_in_head) {
						csgo->imaginary_misses[snapshot.entity->GetIndex()]++;
					}
					else if (
						snapshot.hitbox == 0
						&& hitgroup == HITGROUP_HEAD
						&& hittable_in_head) {
						g_Resolver->AddHitInfo(snapshot.entity, snapshot.resolver_info);
					}
					if (vars.visuals.hitmarker & 2) {
						g_Hitmarker->Add(snapshot.intended_position, hitgroup == HITGROUP_HEAD, Event->GetString(str("dmg_health")));
					}
					shot_snapshots.erase(shot_snapshots.begin());
				}
				auto& new_hitlog = csgo->custom_hitlog.emplace_back();
				new_hitlog.name = snapshot.entity->GetName();
				new_hitlog.backtrack = snapshot.backtrack;
				new_hitlog.hitbox = snapshot.hitbox;
				new_hitlog.resolver = snapshot.resolver_mode;
				new_hitlog.spread = false;
			}

			int health = Event->GetInt(str("health"));

			string Message;
			Message += health > 0 ? str("Hit ") : str("Killed ");
			Message += killed_info.name;

			if (hitgroup != HITGROUP_GENERIC && hitgroup != HITGROUP_GEAR) {
				Message += str(" at ");
				Message += HitgroupToName(hitgroup);
			}

			Message += health > 0 ? str(" for ") : str(" with damage at ");
			Message += Event->GetString(str("dmg_health"));
			Message += str(" hp ");
			if (health > 0) {
				Message += str("(");
				Message += std::to_string(health);
				Message += str("hp left) ");
			}
			if (vars.visuals.eventlog & 1)
				Msg(Message, vars.visuals.eventlog_color);
		}
	}
}

IBasePlayer* GetPlayer(int ID)
{
	int index = interfaces.engine->GetPlayerForUserID(ID);
	return interfaces.ent_list->GetClientEntity(index);
}

void ClearMissedShots(IGameEvent* game_event)
{
	memset(csgo->actual_misses, 0, sizeof(csgo->actual_misses));
	memset(csgo->imaginary_misses, 0, sizeof(csgo->actual_misses));
}

void player_hurt(IGameEvent* game_event) {

	current_event_push(game_event);
	run_callback("player_hurt");
	current_event_clear();

	LogHits(game_event);
	int attackerid = game_event->GetInt(str("attacker"));
	int entityid = interfaces.engine->GetPlayerForUserID(attackerid);
	if (entityid == interfaces.engine->GetLocalPlayer())
	{
		if (vars.visuals.hitmarker_sound) {
			switch (vars.visuals.hitmarker_sound_type) {
			case 0:
				interfaces.engine->ClientCmd_Unrestricted(str("play buttons/arena_switch_press_02.wav"), 0);
				break;
			case 1:
				interfaces.engine->ClientCmd_Unrestricted(str("play resource/warning.wav"), 0);
				break;
			case 2:
				PlaySoundA(default_sound, NULL, SND_ASYNC | SND_MEMORY);
				break;
			case 3:
				PlaySoundA(cod_sound, NULL, SND_ASYNC | SND_MEMORY);
				break;
			}
		}
	}

	if (!csgo->local || !csgo->local->isAlive() || !game_event)
		return;

	if (shot_snapshots.empty())
		return;

	auto& snapshot = shot_snapshots.front();

	if (interfaces.engine->GetPlayerForUserID(game_event->GetInt(str("attacker"))) != interfaces.engine->GetLocalPlayer())
		return;

	if (!snapshot.bullet_impact)
	{
		snapshot.bullet_impact = true;
		snapshot.first_processed_time = TICKS_TO_TIME(csgo->fixed_tickbase);
	}
	snapshot.damage = game_event->GetInt(str("dmg_health"));
	snapshot.hitgroup_hit = game_event->GetInt(str("hitgroup"));
}

void bullet_impact(IGameEvent* game_event) {

	current_event_push(game_event);
	run_callback("bullet_impact");
	current_event_clear();

	g_BulletTracer->Log(game_event);

	if (!csgo->local || !csgo->local->isAlive() || !game_event)
		return;

	if (shot_snapshots.empty())
		return;

	auto& snapshot = shot_snapshots.front();

	if (interfaces.engine->GetPlayerForUserID(game_event->GetInt(str("userid"))) != interfaces.engine->GetLocalPlayer())
		return;

	if (!snapshot.bullet_impact)
	{
		snapshot.bullet_impact = true;
		snapshot.first_processed_time = TICKS_TO_TIME(csgo->fixed_tickbase);
		snapshot.impact = Vector(game_event->GetFloat(str("x")), game_event->GetFloat(str("y")), game_event->GetFloat(str("z")));
	}
}

void weapon_fire(IGameEvent* game_event) {

	current_event_push(game_event);
	run_callback("weapon_fire");
	current_event_clear();

	if (!csgo->local || !csgo->local->isAlive() || !game_event)
		return;
	csgo->last_shoot_pos = csgo->local->GetEyePosition();

	if (shot_snapshots.empty())
		return;

	auto& snapshot = shot_snapshots.front();

	if (!snapshot.weapon_fire)
		snapshot.weapon_fire = true;
	csgo->not_damagedshots++;
}

void player_death(IGameEvent* game_event) {

	current_event_push(game_event);
	run_callback("player_death");
	current_event_clear();

	int attacker = interfaces.engine->GetPlayerForUserID(game_event->GetInt(str("attacker")));
	int iLocalPlayer = interfaces.engine->GetLocalPlayer();
	if (vars.visuals.kill_effect && csgo->local && csgo->local->isAlive() && attacker == iLocalPlayer)
		csgo->local->HealthShotBoostExpirationTime() = TICKS_TO_TIME(csgo->fixed_tickbase) + 1.f;
}

void round_end(IGameEvent* game_event) {
	current_event_push(game_event);
	run_callback("round_end");
	current_event_clear();
	csgo->ForceOffAA = true;
	g_Hitmarker->hitmarkers.clear();
	ClearMissedShots(game_event);
}

void DoBuyBot() {
	if (vars.misc.autobuy.enable) {
		std::string buy_str;

		switch (vars.misc.autobuy.main)
		{
		case 1:
			buy_str += (str("buy scar20; "));
			buy_str += (str("buy g3sg1; "));
			break;
		case 2:
			buy_str += (str("buy ssg08; "));
			break;
		case 3:
			buy_str += (str("buy awp; "));
			break;
		case 4:
			buy_str += (str("buy negev; "));
			break;
		case 5:
			buy_str += (str("buy m249; "));
			break;
		case 6:
			buy_str += (str("buy ak47; "));
			buy_str += (str("buy m4a1; "));
			buy_str += (str("buy m4a1_silencer; "));
			break;
		case 7:
			buy_str += (str("buy aug; "));
			buy_str += (str("buy sg556; "));
			break;
		}

		switch (vars.misc.autobuy.pistol)
		{
		case 1:
			buy_str += (str("buy elite; "));
			break;
		case 2:
			buy_str += (str("buy p250; "));
			break;
		case 3:
			buy_str += (str("buy tec9; "));
			buy_str += (str("buy fn57; "));
			break;
		case 4:
			buy_str += (str("buy deagle; "));
			buy_str += (str("buy revolver; "));
			break;
		}

		if (vars.misc.autobuy.misc & 1)
			buy_str += (str("buy vesthelm; "));
		if (vars.misc.autobuy.misc & 2)
			buy_str += (str("buy vest; "));

		if (vars.misc.autobuy.misc & 4)
			buy_str += (str("buy hegrenade; "));
		if (vars.misc.autobuy.misc & 8) {
			buy_str += (str("buy molotov; "));
			buy_str += (str("buy incgrenade; "));
		}

		if (vars.misc.autobuy.misc & 16)
			buy_str += (str("buy smokegrenade; "));
		if (vars.misc.autobuy.misc & 32)
			buy_str += (str("buy taser; "));
		if (vars.misc.autobuy.misc & 64)
			buy_str += (str("buy defuser; "));


		interfaces.engine->ExecuteClientCmd(buy_str.c_str());
	}
}

void on_restart(IGameEvent* game_event) {
	csgo->packets.clear();
	csgo->ForceOffAA = true;
	g_Hitmarker->hitmarkers.clear();

	g_Visuals->ResetInfo();
	ClearMissedShots(game_event);

	csgo->disable_dt = true;
	csgo->shift_amount = 0;

	for (int i = 0; i < 64; i++) {
		g_Visuals->player_info[i].Reset();
		g_DormantEsp->m_cSoundPlayers[i].reset();
	}

	g_DormantEsp->m_utlvecSoundBuffer.RemoveAll();
	g_DormantEsp->m_utlCurSoundList.RemoveAll();
	g_AutoPeek->Reset();

	csgo->round_start = true;

	DoBuyBot();
}

CGameEventListener* _player_hurt;
CGameEventListener* _bullet_impacts;
CGameEventListener* _weapon_fire;
CGameEventListener* _player_death;
CGameEventListener* _round_end;
CGameEventListener* _round_prestart;
CGameEventListener* _round_start;

void RegListeners()
{
	REG_EVENT_LISTENER(_player_hurt, &player_hurt, str("player_hurt"), false);
	REG_EVENT_LISTENER(_bullet_impacts, &bullet_impact, str("bullet_impact"), false);
	REG_EVENT_LISTENER(_weapon_fire, &weapon_fire, str("weapon_fire"), false);
	REG_EVENT_LISTENER(_player_death, &player_death, str("player_death"), false);
	REG_EVENT_LISTENER(_round_end, &round_end, str("round_end"), false);
	REG_EVENT_LISTENER(_round_prestart, &on_restart, str("round_prestart"), false);
	REG_EVENT_LISTENER(_round_start, &on_restart, str("round_start"), false);
}