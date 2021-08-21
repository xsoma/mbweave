#pragma once
#include "DLL_MAIN.h"
#include "json/json.h"

using namespace std;

enum removals
{
	vis_recoil = 0,
	vis_smoke,
	flash,
	scope,
	zoom,
	post_processing,
	fog,
	shadow,
};

enum weap_type {
	def,
	scar,
	scout,
	_awp,
	rifles,
	pistols,
	heavy_pistols
};

struct chams_t
{
	bool enable;
	int material;

	color_t material_color;

	unsigned int overlay;

	color_t glow_color[2];
	color_t glass_color;
	color_t metallic_color;
	color_t wireframe_color;

	int phong_amount;
	int rim_amount;
};

enum chams_type
{
	enemy_visible,
	enemy_xqz,
	enemy_history,
	enemy_ragebot_shot,

	local_default,
	local_desync,
	local_arms,
	local_weapon,

	chams_max,
};

struct CWeaponConfig {
	bool enable;
	int mindamage;
	int mindamage_override;
	int hitchance;
	int doubletap_hc;
	bool multipoint;
	bool quickstop;
	int quickstop_options;
	int max_misses;
	bool static_scale;
	int scale_head;
	int scale_body;
	//unsigned x3
	unsigned int hitscan;
	unsigned int prefer_safepoint;
	unsigned int force_safepoint;
};

struct CGlobalVariables
{
	struct {
		bool enable;
		bool backtrack;
	} legitbot;
	struct
	{
		bool enable;
		bool silent;
		int fov;
		bool autoshoot;
		int autoscope;
		bool resolver;

		bool dt_teleport;
		int recharge_time;
		bool zeusbot;
		int zeuschance;

		CWeaponConfig weapon[7];

		int active_index;
	} ragebot;
	struct
	{
		bool enable;
		int pitch;
		int yaw;
		bool attarget;
		bool attarget_off_when_offsreen;
		int desync_direction;
		int desync_amount;
		bool avoid_overlap;

		bool aa_on_use;
		bool desync;

		bool manual_antiaim;
		bool ignore_attarget;

		int fakelag;
		bool fakelag_when_standing;
		bool fakelag_when_exploits;
		bool fakelag_on_peek;
		int fakelagfactor;
		int min, max;

		int jitter_angle;

	} antiaim;
	struct
	{
		bool enable;
		bool teammates;
		bool dormant;
		bool box;
		color_t box_color;
		bool healthbar;
		bool name;
		color_t name_color;
		unsigned int flags;
		color_t flags_color;
		bool weapon;
		color_t weapon_color;

		bool skeleton;
		color_t skeleton_color;

		bool ammo;
		color_t ammo_color;

		bool zeus_warning;

		unsigned int hitmarker;
		bool visualize_damage;
		color_t hitmarker_color;
		bool hitmarker_sound;
		int hitmarker_sound_type;

		unsigned int remove;

		bool glow;
		color_t glow_color;
		int glowtype;

		bool preverse_killfeed;

		bool local_glow;
		color_t local_glow_clr;

		bool indicators;
		bool indicators_rage;
		bool hitlog;

		bool bullet_tracer;
		color_t bullet_tracer_color;
		bool bullet_impact;
		bool bullet_tracer_local;
		color_t bullet_tracer_local_color;
		color_t bullet_impact_color;
		color_t client_impact_color;
		int impacts_size;
		int bullet_tracer_type;
		int aspect_ratio;
		bool shot_multipoint;
		bool snipercrosshair;
		bool clantagspammer;
		int transparentprops;

		bool out_of_fov;
		color_t out_of_fov_color;
		int out_of_fov_distance;
		int out_of_fov_size;

		int active_chams_index;

		// local player esp
		bool nightmode;
		bool customize_color;
		int nightmode_amount;
		int prop_alpha_amount;
		//bool removepostprocessing;
		int nightmode_val;
		color_t nightmode_color;
		color_t nightmode_prop_color;
		color_t nightmode_skybox_color;
		int thirdperson_dist;
		bool chamsragdoll;

		int points_size;

		bool override_hp;
		color_t hp_color;

		int prop, wall;

		color_t points_color;
		int taser_range;
		color_t taser_range_color;
		bool nadepred;
		color_t nadepred_color;
		unsigned int eventlog;
		color_t eventlog_color;
		bool speclist;
		bool watermark;
		bool monitor;
		bool force_crosshair;

		bool kill_effect;

		int anim_id;

		bool blend_on_scope;
		int blend_value;

		chams_t chams[chams_max];

		bool ragdoll_force;
		struct {
			struct {
				bool enable;
				bool trajectories;
				unsigned int filter; // team, enemy, local
				color_t colors[3];
			} projectiles;
			struct {
				bool enabled;
				bool planted_bomb;
				color_t color;
			} weapons;
		} world;
	} visuals;

	struct
	{
		bool bunnyhop;
		bool autostrafe;
		bool knifebot;
		int restrict_type;
		bool antiuntrusted = true;
		int viewmodelfov;
		int viewmodel_x, viewmodel_y, viewmodel_z;
		int worldfov;
		int zoomfov;

		struct {
			bool enable;
			int main;
			int pistol;
			unsigned int misc;
		}autobuy;

		bool slidewalk;

		int agents_t, agents_ct;
	} misc;

	struct
	{
		std::string active_config_name;
		int active_config_index;
		color_t color;
		bool open;
		int y;
		bool guiinited;
		int current_tab;
	} menu;

	struct {
		std::string active_script_name;
		int active_script_index;
	} js;

};

extern CGlobalVariables vars;

enum binds_enum_t
{
	bind_override_dmg,
	bind_force_safepoint,
	bind_baim,
	bind_double_tap,
	bind_hide_shots,
	bind_aa_inverter,
	bind_manual_left,
	bind_manual_right,
	bind_manual_back,
	bind_manual_forward,
	bind_fake_duck,
	bind_slow_walk,
	bind_third_person,
	bind_peek_assist,

	bind_max
};

extern c_bind g_Binds[bind_max];

extern void CreateConfigFolder();

typedef Json::Value json_t;

class CConfig
{
private:

	string GetModuleFilePath(HMODULE hModule);
	string GetModuleBaseDir(HMODULE hModule);

public:
	struct preload_cfg {
		std::string created_by;
		std::string created_at;
		std::string last_modified_user;
		std::string last_modified_date;
		bool can_be_loaded;
	};
	vector<std::string> ConfigList;
	void ReadConfigs(LPCTSTR lpszFileName);
	void ResetToDefault();
	bool init = false;
	CConfig()
	{
		Setup();
	}
	void SaveColor(color_t color, const char* name, json_t* json) {
		auto& j = *json;
		j[name][str("red")] = color.get_red();
		j[name][str("green")] = color.get_green();
		j[name][str("blue")] = color.get_blue();
		j[name][str("alpha")] = color.get_alpha();
	}
	void SaveBind(c_bind* bind, const char* name, json_t* json) {
		auto& j = *json;
		j[name][str("key")] = bind->key;
		j[name][str("type")] = bind->type;
		j[name][str("active")] = bind->active;
	}
	void LoadBool(bool* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asBool();
	}
	void LoadInt(int* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asInt();
	}
	void LoadUInt(unsigned int* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asUInt();
	}
	void LoadFloat(float* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asFloat();
	}
	void LoadColor(color_t* pointer, const char* name, json_t json) {
		if (!json.isMember(name))
			return;
		const auto& location = json[name];
		if (location.isMember(str("red")) && location.isMember(str("green"))
			&& location.isMember(str("blue")) && location.isMember(str("alpha"))) {
			pointer->set_red(location[str("red")].asInt());
			pointer->set_green(location[str("green")].asInt());
			pointer->set_blue(location[str("blue")].asInt());
			pointer->set_alpha(location[str("alpha")].asInt());
		}
	}
	void LoadBind(c_bind* pointer, const char* name, json_t json) {
		if (!json.isMember(name))
			return;
		const auto& location = json[name];
		if (location.isMember(str("key")) && location.isMember(str("type"))) {
			pointer->key = location[str("key")].asUInt();
			pointer->type = location[str("type")].asUInt();
			pointer->active = location[str("active")].asBool();
		}
	}
	void Setup();
	void PreLoad(string cfg_name, preload_cfg* p);
	void Save(string cfg_name, bool create = false);
	void Load(string cfg_name);
	void Remove(string cfg_name);
};

extern CConfig Config;