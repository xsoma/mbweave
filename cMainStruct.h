#pragma once
#include "Utils.h"
#include "DLL_MAIN.h"
#include "interfaces.h"
#include "i_base_player.h"
#include "c_usercmd.h"
#include "render.h"
#include <deque>
#include <array>
#include <mutex>

#define TICK_INTERVAL            ( interfaces.global_vars->interval_per_tick )
#define TIME_TO_TICKS( dt )        ( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )        ( TICK_INTERVAL *( t ) )

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4    
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_NECK		 8
#define HITGROUP_GEAR 10

class INetChannel;
class CInput;
class ConVar;
class VMTHook;
class CClientState;

using namespace std;
////////////////////////////////////////////////////////////////////////////////////////////////////

struct cINIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct MatrixRecorded;
extern MatrixRecorded g_Records[64];
struct animation;

struct command
{
	int previous_command_number = 0;
	bool is_used = false;
	int cmd_number = 0;
	bool is_outgoing = false;
};
struct correction_data
{
	int tickcount = 0;
	int choked_commands = 0;
	int command_number = 0;
};
struct hitlog {
	std::string name;
	/*
	hitbox
		resolver
		backtrack
		safepoint
		spread
	*/
	int hitbox;
	std::string resolver;
	int backtrack;
	bool safepoint;
	bool spread;
};

class CCSGO_HudDeathNotice;


class CCheatVars
{
public:
	void* subs_info;
	std::string username;
	std::string password;
	std::string sub_time;
	std::wstring log_location;
	cINIT Init; 
	bool DoUnload;
	IDirect3DDevice9* render_device;
	bool ShowConfig;
	CUtils Utils;
	int unpdred_tick;
	int m_shot_command_number;
	Vector unpred_eyepos;
	std::vector<int> animQueue;
	bool should_draw_taser_range;
	std::pair<Vector, Vector> taser_ranges[106];
	std::vector<hitlog> custom_hitlog;
	IBasePlayer* local;
	float impact_time;
	std::mutex mtx;
	std::deque<correction_data> c_data;
	std::deque<command> packets;
	Vector impact_origin;
	Vector origin;
	bool mapChanged;
	float unpred_inaccuracy;
	int fixed_tickbase;
	float last_gfy;
	float dormant_time[64];

	int CameFromDormant[65];
	
	// meme
	Vector last_shoot_pos;
	int shift_amount;
	bool need_recharge_rn;
	bool is_local_alive;
	int w;
	int h;
	float last_hit_time;
	bool is_connected;
	float tick_rate;
	bool bInSendMove;
	bool charge;
	int32_t nSinceUse;
	bool TickShifted;
	bool bShootedFirstBullet;
	bool bFirstSendMovePack;
	Vector eyepos;
	int last_forced_tickcount;
	matrix fakematrix[128];
	matrix realmatrix[128];
	matrix fakelag_matrix[128];
	bool hitchance;
	Vector CurAngle;
	Vector last_sended_origin;
	float weapon_range;
	float lc_factor;
	bool round_end;
	bool SwitchAA, EnableSwitchAA;
	int SwitchAA2;
	float next_lby_update;
	void updatelocalplayer();
	bool last_sendpacket;
	bool force_next_sendpacket;
	bool send_packet;
	bool should_update_animations;
	bool fake_duck;
	int scroll_amount;
	bool didShot[65];
	float delta;
	float duck_amount;
	CCSGOPlayerAnimState* animstate;
	bool EnableBones, Rebuild, ShouldUpdate, UpdateMatrix, UpdateMatrixLocal;
	bool should_sidemove;
	float original_forwardmove;
	float original_sidemove;
	CUserCmd* cmd;
	bool choke_meme;
	alignas(16) matrix BoneMatrix[128];
	float old_simtime[64];
	bool should_stop;
	bool should_stop_slide;
	bool should_stop_fast;
	IBaseCombatWeapon* weapon;
	CCSWeaponInfo* weapon_data;
	int weapon_id;
	Vector viewangles;
//	std::unique_ptr< VMTHook > g_pNetChannelHook;
	std::unique_ptr< VMTHook > g_pPlayerHook;
	vector<int> choked_history;
	int eventtimer;
	float zero_pitch;
	float last_clamped_yaw;
	Vector VisualAngle;
	Vector UnChokedAng;
	int last_choked;
	float NextPredictedLBYUpdate;
	Vector vecUnpredictedVel;
	Vector original;
	Vector FakeAngle;
	bool forcing_shot;
	std::vector<Vector> shot_angles;
	float feetYaw;
	bool in_cm;
	float velocity_modifier;
	std::vector<int> choked_number;
	CClientState* client_state;
	CCSGameRules* game_rules;
	INetChannel* net_channel;
	int max_fakelag_choke;
	bool aimwareShouldChoke;
	int aimwareTicks;
	//int skip_ticks;
	bool can_charge_on_shot;
	int skip_ticks;
	bool need_to_recharge;
	bool disable_dt;
	bool scoped;
	int zoom_level;
	bool exploits;
	CAnimationLayer layers[13];
	std::array<float, 24> poses;
	int revolver_cock;
	int revolver_query;
	bool revolver_fire;

	bool round_start;

	float ping;
	float desync_angle;
	int damagedshots;
	int not_damagedshots;
	int chockedticks, timer;
	float viewMatrix[4][4] = { 0 };
	bool ForceOffAA;
	float EntityAlpha[65];
	bool PlayerReseted[65];
	float LastSeenTime[65];
	bool StrafeModifiedAngles;
	Vector StrafeAngles;
	float resolved_yaw[65];
	int imaginary_misses[65];
	int actual_misses[65];
	float weaponspread;
	float curtime;
	int tickcount;
	float spread, innacuracy;
	uint32_t shot_cmd = -1;
	int shot_tick = -1;
	uintptr_t uRandomSeed;
	bool ispenetrable;
	bool key_pressed[256];
	int key_pressedticks[256];
	bool key_down[256];
	bool InThirdperson;
	bool InAutowall;
	bool canR8shoot;

	std::map<std::string, LPDIRECT3DTEXTURE9> outdated_textures;

	string build_date = __DATE__;
	float get_absolute_time() {
		return (float)(clock() / (float)1000.f);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////