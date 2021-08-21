#include "Variables.h"
#include "Hooks.h"
#include <sstream>
#include "GUI/element.h"
#include <filesystem>

c_bind g_Binds[bind_max];

//void xor_crypt(std::vector<uint8_t>& vec) {
//	for (auto& c : vec) {
//		for (const auto& k : xs64_extp_key)
//			c ^= k;
//	}
//}

void xor_crypt(std::vector<uint8_t>& vec) {
	for (size_t i = 0, size = vec.size(), key_size = xs64_extp_key.size(); i < size; ++i) {
		vec[i] ^= xs64_extp_key[i % key_size];
	}
}

void CConfig::ReadConfigs(LPCTSTR lpszFileName)
{
	ConfigList.push_back(lpszFileName);
}

void CConfig::ResetToDefault()
{
	vars.legitbot.enable = false;
	vars.legitbot.backtrack = false;

	vars.ragebot.enable = false;
	vars.ragebot.autoscope = 0;
	vars.ragebot.silent = true;
	vars.ragebot.fov = 180;
	vars.ragebot.autoshoot = false;
	vars.ragebot.resolver = false;
	vars.ragebot.dt_teleport = false;

	g_Binds[bind_override_dmg].key = 0;
	g_Binds[bind_override_dmg].active = false;
	g_Binds[bind_override_dmg].type = 1;

	g_Binds[bind_force_safepoint].key = 0;
	g_Binds[bind_force_safepoint].active = false;
	g_Binds[bind_force_safepoint].type = 1;

	g_Binds[bind_double_tap].active = false;
	g_Binds[bind_double_tap].key = 0;
	g_Binds[bind_double_tap].type = 1;

	g_Binds[bind_hide_shots].active = false;
	g_Binds[bind_hide_shots].key = 0;
	g_Binds[bind_hide_shots].type = 1;

	vars.ragebot.recharge_time = 7;

	for (auto& a : vars.ragebot.weapon) {
		a.enable = false;
		a.mindamage = 0;
		a.mindamage_override = 0;
		a.hitchance = 0;
		a.doubletap_hc = 0;
		a.multipoint = false;
		a.quickstop = false;
		a.static_scale = false;
		a.hitscan = 0;
		a.scale_head = 0;
		a.scale_body = 0;
		a.prefer_safepoint = 0;
		a.force_safepoint = 0;
	}
	
	vars.ragebot.active_index = 0;

	vars.antiaim.enable = false;
	vars.antiaim.aa_on_use = false;
	vars.antiaim.desync = false;
	vars.antiaim.pitch = 0;
	vars.antiaim.yaw = 0;
	
	vars.antiaim.fakelag = 0;
	vars.antiaim.fakelag_when_exploits = true;
	vars.antiaim.fakelag_when_standing = false;
	vars.antiaim.fakelag_on_peek = false;
	vars.antiaim.fakelagfactor = 1;

	g_Binds[bind_fake_duck].active = false;
	g_Binds[bind_fake_duck].key = 0;
	g_Binds[bind_fake_duck].type = 1;

	g_Binds[bind_slow_walk].active = false;
	g_Binds[bind_slow_walk].key = 0;
	g_Binds[bind_slow_walk].type = 1;

	vars.visuals.enable = false;
	vars.visuals.dormant = false;
	vars.visuals.skeleton = false;
	vars.visuals.skeleton_color = color_t(255, 255, 255, 255);
	vars.visuals.box = false;
	vars.visuals.box_color = color_t(255, 255, 255, 255);
	vars.visuals.healthbar = false;
	vars.visuals.override_hp = false;
	vars.visuals.hp_color = color_t(0, 255, 0, 255);
	vars.visuals.weapon = false;
	vars.visuals.weapon_color = color_t(255, 255, 255, 255);
	vars.visuals.name = false;
	vars.visuals.name_color = color_t(255, 255, 255, 255);
	vars.visuals.ammo = false;
	vars.visuals.ammo_color = color_t(238, 0, 142, 255);
	vars.visuals.flags = 0;
	vars.visuals.flags_color = color_t(255, 255, 255, 255);
	vars.visuals.out_of_fov = false;
	vars.visuals.out_of_fov_color = color_t(255, 255, 255, 255);

	vars.visuals.hitmarker = false;
	vars.visuals.hitmarker_sound = false;
	vars.visuals.hitmarker_sound_type = 0;
	vars.visuals.hitmarker_color = color_t(255, 255, 255, 255);
	vars.visuals.speclist = false;
	vars.visuals.monitor = false;
	vars.visuals.nadepred = false;
	vars.visuals.nadepred_color = color_t(255, 255, 255, 255);
	vars.visuals.taser_range = 0;
	vars.visuals.taser_range_color = color_t(255, 255, 255, 255);

	g_Binds[bind_peek_assist].active = false;
	g_Binds[bind_peek_assist].key = 0;
	g_Binds[bind_peek_assist].type = 0;

	g_Binds[bind_third_person].active = false;
	g_Binds[bind_third_person].key = 0;
	g_Binds[bind_third_person].type = 2;

	vars.visuals.thirdperson_dist = 150;
	vars.visuals.aspect_ratio = 0;
	vars.visuals.indicators = 0;
	vars.visuals.eventlog = 0;
	vars.visuals.eventlog_color = color_t(255, 255, 255, 255);
	vars.visuals.bullet_tracer = false;
	vars.visuals.bullet_tracer_type = 0;
	vars.visuals.bullet_tracer_color = color_t(255, 0, 0, 255);
	vars.visuals.bullet_impact = false;
	vars.visuals.bullet_impact_color = color_t(0, 0, 255, 255);
	vars.visuals.client_impact_color = color_t(255, 0, 0, 255);
	vars.visuals.bullet_tracer_local_color = color_t(0, 255, 0, 255);
	vars.visuals.remove = 0;
	vars.visuals.nightmode = false;
	vars.visuals.customize_color = false;
	vars.visuals.nightmode_amount = 50;
	vars.visuals.prop_alpha_amount = 0;
	vars.visuals.nightmode_color = color_t(43, 41, 46, 255);
	vars.visuals.nightmode_prop_color = color_t(255, 255, 255, 255);
	vars.visuals.nightmode_skybox_color = color_t(255, 255, 255, 255);
	vars.visuals.kill_effect = false;
	vars.misc.worldfov = 0;
	vars.misc.viewmodelfov = 68;
	vars.misc.zoomfov = 100.f;
	vars.visuals.zeus_warning = false;

	vars.visuals.watermark = true;

	vars.visuals.ragdoll_force = false;
	vars.visuals.glow = false;
	vars.visuals.glowtype = 0;
	vars.visuals.local_glow = false;
	vars.visuals.glow_color = color_t(214, 47, 182, 130);
	vars.visuals.local_glow_clr = color_t(214, 192, 47, 255);

	vars.visuals.out_of_fov_distance = 100;
	vars.visuals.out_of_fov_size = 25;

	// init enemy chams
	{
		// visible
		vars.visuals.chams[enemy_visible].enable = false;
		vars.visuals.chams[enemy_visible].material = 0;
		vars.visuals.chams[enemy_visible].material_color = color_t(240, 72, 10, 255);
		vars.visuals.chams[enemy_visible].glass_color = color_t(255, 255, 255, 200);
		vars.visuals.chams[enemy_visible].metallic_color = color_t(255, 255, 255, 200);
		vars.visuals.chams[enemy_visible].phong_amount = 50;
		vars.visuals.chams[enemy_visible].rim_amount = 0;
		vars.visuals.chams[enemy_visible].overlay = 0;

		for (int j = 0; j < 2; j++)
			vars.visuals.chams[enemy_visible].glow_color[j] = color_t(240, 72, 10);

		vars.visuals.chams[enemy_visible].wireframe_color = color_t(255, 255, 255, 200);

		// through walls
		vars.visuals.chams[enemy_xqz].enable = false;
		vars.visuals.chams[enemy_xqz].material = 0;
		vars.visuals.chams[enemy_xqz].material_color = color_t(255, 76, 0, 255);
		vars.visuals.chams[enemy_xqz].glass_color = color_t(255, 255, 255, 200);
		vars.visuals.chams[enemy_xqz].metallic_color = color_t(255, 255, 255, 200);
		vars.visuals.chams[enemy_xqz].phong_amount = 50;
		vars.visuals.chams[enemy_xqz].rim_amount = 0;
		vars.visuals.chams[enemy_xqz].overlay = 0;

		for (int x = 0; x < 2; x++)
			vars.visuals.chams[enemy_xqz].glow_color[x] = color_t(255, 76, 0, 150);

		vars.visuals.chams[enemy_xqz].wireframe_color = color_t(255, 255, 255, 200);
	}

	vars.visuals.blend_on_scope = false;
	vars.visuals.blend_value = 100;

	// other chams, don't need to do custom shit
	for (int i = 2; i < 8; i++)
	{
		vars.visuals.chams[i].enable = false;
		vars.visuals.chams[i].material = 0;
		vars.visuals.chams[i].material_color = color_t(255, 255, 255, 255);
		vars.visuals.chams[i].glass_color = color_t(255, 255, 255, 200);
		vars.visuals.chams[i].metallic_color = color_t(255, 255, 255, 200);
		vars.visuals.chams[i].phong_amount = 50;
		vars.visuals.chams[i].rim_amount = 0;
		vars.visuals.chams[i].overlay = 0;

		for (int j = 0; j < 2; j++)
			vars.visuals.chams[i].glow_color[j] = color_t(255, 255, 255, 150);
		vars.visuals.chams[i].wireframe_color = color_t(255, 255, 255, 200);
	}

	vars.visuals.world.weapons.enabled = false;
	vars.visuals.world.weapons.color = color_t(255, 255, 255, 255);

	vars.visuals.world.projectiles.enable = false;
	vars.visuals.world.projectiles.colors[0] = color_t(35, 255, 35, 255);
	vars.visuals.world.projectiles.colors[1] = color_t(255, 35, 35, 255);
	vars.visuals.world.projectiles.colors[2] = color_t(255, 255, 255, 255);
	vars.visuals.world.projectiles.filter = 0;
	vars.visuals.world.projectiles.trajectories = false;

	vars.misc.slidewalk = false;
	vars.misc.antiuntrusted = true;
	vars.misc.bunnyhop = false;
	vars.misc.knifebot = false;
	vars.visuals.clantagspammer = false;
	vars.misc.viewmodel_x = 0;
	vars.misc.viewmodel_y = 0;
	vars.misc.viewmodel_z = 0;

	vars.misc.autobuy.enable = false;
	vars.misc.autobuy.main = 0;
	vars.misc.autobuy.misc = 0;
	vars.misc.autobuy.pistol = 0;
}

void CConfig::Setup()
{
	ResetToDefault();
};

string CConfig::GetModuleFilePath(HMODULE hModule)
{
	string ModuleName = str("");
	char szFileName[MAX_PATH] = { 0 };

	if (GetModuleFileNameA(hModule, szFileName, MAX_PATH))
		ModuleName = szFileName;

	return ModuleName;
}

string CConfig::GetModuleBaseDir(HMODULE hModule)
{
	string ModulePath = GetModuleFilePath(hModule);
	return ModulePath.substr(0, ModulePath.find_last_of(str("\\/")));
}

void CConfig::Remove(string cfg_name) {

	static TCHAR path[256];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + str("\\weave\\configs\\");
		file = std::string(path) + str("\\weave\\configs\\") + cfg_name;
	}

	remove(file.c_str());
}

void CConfig::Save(string cfg_name, bool create)
{
	json_t configuration;

	auto& json = configuration[str("config")];
	json[str("name")] = cfg_name;


	time_t my_time = time(NULL);
	json[str("last_modified_date")] = std::string(ctime(&my_time));
	json[str("last_modified_user")] = csgo->username;

	if (create) {
		json[str("created_at")] = std::string(ctime(&my_time));
		json[str("created_by")] = csgo->username;
	}
	else
	{
		preload_cfg p;
		PreLoad(cfg_name, &p);
		json[str("created_at")] = p.created_at;
		json[str("created_by")] = p.created_by;
	}

	auto& legitbot = json[str("legitbot")]; {
		legitbot[str("enable")] = vars.legitbot.enable;
		legitbot[str("backtrack")] = vars.legitbot.backtrack;
	}

	for (uint16_t i = 0U; i < c_style::e_style_id_last; ++i)
		SaveColor(style.get_color((c_style::e_style_id)i), std::string(str("menu_colors_") + std::to_string(i)).c_str(), &json);
	
	auto& ragebot = json[str("ragebot")]; {
		ragebot[str("enable")] = vars.ragebot.enable;
		ragebot[str("auto_shoot")] = vars.ragebot.autoshoot;
		ragebot[str("silent")] = vars.ragebot.silent;
		ragebot[str("fov")] = vars.ragebot.fov;
		ragebot[str("auto_scope")] = vars.ragebot.autoscope;
		ragebot[str("zeusbot")] = vars.ragebot.zeusbot;
		ragebot[str("zeuschance")] = vars.ragebot.zeuschance;
		ragebot[str("resolver")] = vars.ragebot.resolver;
		ragebot[str("dt_teleport")] = vars.ragebot.dt_teleport;
		ragebot[str("recharge_time")] = vars.ragebot.recharge_time;

		SaveBind(&g_Binds[bind_override_dmg], str("override_dmg"), &ragebot);
		SaveBind(&g_Binds[bind_double_tap], str("double_tap"), &ragebot);
		SaveBind(&g_Binds[bind_hide_shots], str("hide_shots"), &ragebot);
		SaveBind(&g_Binds[bind_force_safepoint], str("force_safepoint"), &ragebot);

		SaveBind(&g_Binds[bind_baim], str("baim"), &ragebot);

		auto& weapon_cfg = ragebot[str("weapon_cfg")];
		for (int i = 0; i < 7; i++) {
			string category = str("weapon_") + std::to_string(i);
			weapon_cfg[category][str("enable")] = vars.ragebot.weapon[i].enable;
			weapon_cfg[category][str("mindamage")] = vars.ragebot.weapon[i].mindamage;
			weapon_cfg[category][str("mindamage_override")] = vars.ragebot.weapon[i].mindamage_override;
			weapon_cfg[category][str("hitchance")] = vars.ragebot.weapon[i].hitchance;
			weapon_cfg[category][str("doubletap_hc")] = vars.ragebot.weapon[i].doubletap_hc;
			weapon_cfg[category][str("multipoint")] = vars.ragebot.weapon[i].multipoint;
			weapon_cfg[category][str("quickstop")] = vars.ragebot.weapon[i].quickstop;
			weapon_cfg[category][str("quickstop_options")] = vars.ragebot.weapon[i].quickstop_options;
			weapon_cfg[category][str("hitboxes")] = vars.ragebot.weapon[i].hitscan;
			weapon_cfg[category][str("static_scale")] = vars.ragebot.weapon[i].static_scale;
			weapon_cfg[category][str("scale_body")] = vars.ragebot.weapon[i].scale_body;
			weapon_cfg[category][str("scale_head")] = vars.ragebot.weapon[i].scale_head;
			weapon_cfg[category][str("prefer_safepoint")] = vars.ragebot.weapon[i].prefer_safepoint;
			weapon_cfg[category][str("force_safepoint")] = vars.ragebot.weapon[i].force_safepoint;
			weapon_cfg[category][str("max_misses")] = vars.ragebot.weapon[i].max_misses;
		}
	}

	auto& antiaim = json[str("antiaim")]; {
		antiaim[str("enable")] = vars.antiaim.enable;
		antiaim[str("aa_on_use")] = vars.antiaim.aa_on_use;
		antiaim[str("pitch")] = vars.antiaim.pitch;
		antiaim[str("yaw")] = vars.antiaim.yaw;
		antiaim[str("desync")] = vars.antiaim.desync;
		antiaim[str("jitter_angle")] = vars.antiaim.jitter_angle;
		antiaim[str("at_target")] = vars.antiaim.attarget;
		antiaim[str("attarget_off_when_offsreen")] = vars.antiaim.attarget_off_when_offsreen;

		SaveBind(&g_Binds[bind_fake_duck], str("fakeduck"), &antiaim);
		SaveBind(&g_Binds[bind_slow_walk], str("slowwalk"), &antiaim);
		SaveBind(&g_Binds[bind_aa_inverter], str("inverter"), &antiaim);

		antiaim[str("desync_direction")] = vars.antiaim.desync_direction;
		antiaim[str("desync_amount")] = vars.antiaim.desync_amount;
		antiaim[str("manual_antiaim")] = vars.antiaim.manual_antiaim;
		antiaim[str("ignore_attarget")] = vars.antiaim.ignore_attarget;

		SaveBind(&g_Binds[bind_manual_left], str("manual_left"), &antiaim);
		SaveBind(&g_Binds[bind_manual_right], str("manual_right"), &antiaim);
		SaveBind(&g_Binds[bind_manual_back], str("manual_back"), &antiaim);
		SaveBind(&g_Binds[bind_manual_forward], str("manual_forward"), &antiaim);

		auto& fakelag = antiaim[str("fakelag")]; {
			fakelag[str("type")] = vars.antiaim.fakelag;
			fakelag[str("when_standing")] = vars.antiaim.fakelag_when_standing;
			fakelag[str("when_exploits")] = vars.antiaim.fakelag_when_exploits;
			fakelag[str("on_peek")] = vars.antiaim.fakelag_on_peek;
			fakelag[str("factor")] = vars.antiaim.fakelagfactor;
		}
	}

	auto& players = json[str("players")]; {
		auto& esp = players[str("esp")]; {
			esp[str("enable")] = vars.visuals.enable;
			esp[str("dormant")] = vars.visuals.dormant;
			esp[str("zeus_warning")] = vars.visuals.zeus_warning;

			esp[str("box")] = vars.visuals.box; {
				esp[str("box_color")][str("red")] = vars.visuals.box_color.get_red();
				esp[str("box_color")][str("green")] = vars.visuals.box_color.get_green();
				esp[str("box_color")][str("blue")] = vars.visuals.box_color.get_blue();
				esp[str("box_color")][str("alpha")] = vars.visuals.box_color.get_alpha();
			}

			esp[str("skeleton")] = vars.visuals.skeleton; {
				esp[str("skeleton_color")][str("red")] = vars.visuals.skeleton_color.get_red();
				esp[str("skeleton_color")][str("green")] = vars.visuals.skeleton_color.get_green();
				esp[str("skeleton_color")][str("blue")] = vars.visuals.skeleton_color.get_blue();
				esp[str("skeleton_color")][str("alpha")] = vars.visuals.skeleton_color.get_alpha();
			}

			esp[str("healthbar")][str("enable")] = vars.visuals.healthbar;
			esp[str("healthbar")][str("override_hp")] = vars.visuals.override_hp; {
				esp[str("healthbar")][str("hp_color")][str("red")] = vars.visuals.hp_color.get_red();
				esp[str("healthbar")][str("hp_color")][str("green")] = vars.visuals.hp_color.get_green();
				esp[str("healthbar")][str("hp_color")][str("blue")] = vars.visuals.hp_color.get_blue();
				esp[str("healthbar")][str("hp_color")][str("alpha")] = vars.visuals.hp_color.get_alpha();
			}

			esp[str("name")] = vars.visuals.name; {
				esp[str("name_color")][str("red")] = vars.visuals.name_color.get_red();
				esp[str("name_color")][str("green")] = vars.visuals.name_color.get_green();
				esp[str("name_color")][str("blue")] = vars.visuals.name_color.get_blue();
				esp[str("name_color")][str("alpha")] = vars.visuals.name_color.get_alpha();
			}

			esp[str("weapon")] = vars.visuals.weapon; {
				esp[str("weapon_color")][str("red")] = vars.visuals.weapon_color.get_red();
				esp[str("weapon_color")][str("green")] = vars.visuals.weapon_color.get_green();
				esp[str("weapon_color")][str("blue")] = vars.visuals.weapon_color.get_blue();
				esp[str("weapon_color")][str("alpha")] = vars.visuals.weapon_color.get_alpha();
			}

			esp[str("ammo")] = vars.visuals.ammo; {
				esp[str("ammo_color")][str("red")] = vars.visuals.ammo_color.get_red();
				esp[str("ammo_color")][str("green")] = vars.visuals.ammo_color.get_green();
				esp[str("ammo_color")][str("blue")] = vars.visuals.ammo_color.get_blue();
				esp[str("ammo_color")][str("alpha")] = vars.visuals.ammo_color.get_alpha();
			}

			esp[str("flags")] = vars.visuals.flags; {
				esp[str("flags_color")][str("red")] = vars.visuals.flags_color.get_red();
				esp[str("flags_color")][str("green")] = vars.visuals.flags_color.get_green();
				esp[str("flags_color")][str("blue")] = vars.visuals.flags_color.get_blue();
				esp[str("flags_color")][str("alpha")] = vars.visuals.flags_color.get_alpha();
			}
			esp[str("show_multipoint")] = vars.visuals.shot_multipoint;
			esp[str("out_of_fov")][str("enable")] = vars.visuals.out_of_fov; {
				esp[str("out_of_fov")][str("distance")] = vars.visuals.out_of_fov_distance;
				esp[str("out_of_fov")][str("size")] = vars.visuals.out_of_fov_size;
				esp[str("out_of_fov")][str("color")][str("red")] = vars.visuals.out_of_fov_color.get_red();
				esp[str("out_of_fov")][str("color")][str("green")] = vars.visuals.out_of_fov_color.get_green();
				esp[str("out_of_fov")][str("color")][str("blue")] = vars.visuals.out_of_fov_color.get_blue();
				esp[str("out_of_fov")][str("color")][str("alpha")] = vars.visuals.out_of_fov_color.get_alpha();
			}
		}
		auto& models = players[str("models")]; {  // ебал € в рот это все переносить пизда (@opai), пиздец еще и лоад делать ƒј ЅЋя“№
			auto& chams_cfg = models[str("chams")]; {
				chams_cfg[str("ragdoll_force")] = vars.visuals.ragdoll_force;
				chams_cfg[str("blend_on_scope")] = vars.visuals.blend_on_scope;
				chams_cfg[str("blend_value")] = vars.visuals.blend_value;

				for (int i = 0; i < chams_max; i++)
				{
					std::string category = str("model_") + std::to_string(i);

					chams_cfg[category][str("enable")] = vars.visuals.chams[i].enable;
					chams_cfg[category][str("overlay")] = vars.visuals.chams[i].overlay;

					chams_cfg[category][str("material")][str("type")] = vars.visuals.chams[i].material;
					chams_cfg[category][str("material")][str("color")][str("red")] = vars.visuals.chams[i].material_color.get_red();
					chams_cfg[category][str("material")][str("color")][str("green")] = vars.visuals.chams[i].material_color.get_green();
					chams_cfg[category][str("material")][str("color")][str("blue")] = vars.visuals.chams[i].material_color.get_blue();
					chams_cfg[category][str("material")][str("color")][str("alpha")] = vars.visuals.chams[i].material_color.get_alpha();

					chams_cfg[category][str("glass")][str("color")][str("red")] = vars.visuals.chams[i].glass_color.get_red();
					chams_cfg[category][str("glass")][str("color")][str("green")] = vars.visuals.chams[i].glass_color.get_green();
					chams_cfg[category][str("glass")][str("color")][str("blue")] = vars.visuals.chams[i].glass_color.get_blue();
					chams_cfg[category][str("glass")][str("color")][str("alpha")] = vars.visuals.chams[i].glass_color.get_alpha();

					chams_cfg[category][str("metallic")][str("color")][str("red")] = vars.visuals.chams[i].metallic_color.get_red();
					chams_cfg[category][str("metallic")][str("color")][str("green")] = vars.visuals.chams[i].metallic_color.get_green();
					chams_cfg[category][str("metallic")][str("color")][str("blue")] = vars.visuals.chams[i].metallic_color.get_blue();
					chams_cfg[category][str("metallic")][str("color")][str("alpha")] = vars.visuals.chams[i].metallic_color.get_alpha();

					chams_cfg[category][str("metallic")][str("phong_amount")] = vars.visuals.chams[i].phong_amount;
					chams_cfg[category][str("metallic")][str("rim_amount")] = vars.visuals.chams[i].rim_amount;

					for (int j = 0; j < 2; j++)
					{
						std::string second_category = str("glow_") + std::to_string(j);

						chams_cfg[category][second_category][str("color")][str("red")] = vars.visuals.chams[i].glow_color[j].get_red();
						chams_cfg[category][second_category][str("color")][str("green")] = vars.visuals.chams[i].glow_color[j].get_green();
						chams_cfg[category][second_category][str("color")][str("blue")] = vars.visuals.chams[i].glow_color[j].get_blue();
						chams_cfg[category][second_category][str("color")][str("alpha")] = vars.visuals.chams[i].glow_color[j].get_alpha();
					}

					chams_cfg[category][str("wireframe")][str("color")][str("red")] = vars.visuals.chams[i].wireframe_color.get_red();
					chams_cfg[category][str("wireframe")][str("color")][str("green")] = vars.visuals.chams[i].wireframe_color.get_green();
					chams_cfg[category][str("wireframe")][str("color")][str("blue")] = vars.visuals.chams[i].wireframe_color.get_blue();
					chams_cfg[category][str("wireframe")][str("color")][str("alpha")] = vars.visuals.chams[i].wireframe_color.get_alpha();
				}
			}
			auto& glow = models[str("glow")]; {
				glow[str("style")] = vars.visuals.glowtype;

				glow[str("enemy")][str("enable")] = vars.visuals.glow;
				glow[str("enemy")][str("color")][str("red")] = vars.visuals.glow_color.get_red();
				glow[str("enemy")][str("color")][str("green")] = vars.visuals.glow_color.get_green();
				glow[str("enemy")][str("color")][str("blue")] = vars.visuals.glow_color.get_blue();
				glow[str("enemy")][str("color")][str("alpha")] = vars.visuals.glow_color.get_alpha();

				glow[str("local")][str("enable")] = vars.visuals.local_glow;
				glow[str("local")][str("color")][str("red")] = vars.visuals.local_glow_clr.get_red();
				glow[str("local")][str("color")][str("green")] = vars.visuals.local_glow_clr.get_green();
				glow[str("local")][str("color")][str("blue")] = vars.visuals.local_glow_clr.get_blue();
				glow[str("local")][str("color")][str("alpha")] = vars.visuals.local_glow_clr.get_alpha();
			}
		}
		auto& misc = players[str("misc")]; {
			misc[str("preverse_killfeed")][str("enable")] = vars.visuals.preverse_killfeed;

			misc[str("nadepred")][str("enable")] = vars.visuals.nadepred;
			SaveColor(vars.visuals.nadepred_color, str("color"), &misc[str("nadepred")]);

			misc[str("taser_range")][str("enable")] = vars.visuals.taser_range;
			SaveColor(vars.visuals.taser_range_color, str("color"), &misc[str("taser_range")]);

			misc[str("indicators")] = vars.visuals.indicators;

			misc[str("aspect_ratio")] = vars.visuals.aspect_ratio;
			misc[str("thirdperson")][str("distance")] = vars.visuals.thirdperson_dist;
			SaveBind(&g_Binds[bind_third_person], str("bind"), &misc[str("thirdperson")]);

			SaveBind(&g_Binds[bind_peek_assist], str("peek_assist"), &misc);

			misc[str("watermark")] = vars.visuals.watermark;

			misc[str("eventlog")][str("enable")] = vars.visuals.eventlog;
			
			misc[str("eventlog")][str("color")][str("red")] = vars.visuals.eventlog_color.get_red();
			misc[str("eventlog")][str("color")][str("green")] = vars.visuals.eventlog_color.get_green();
			misc[str("eventlog")][str("color")][str("blue")] = vars.visuals.eventlog_color.get_blue();
			misc[str("eventlog")][str("color")][str("alpha")] = vars.visuals.eventlog_color.get_alpha();
			//visualize_damage
			misc[str("hitmarker")][str("type")] = vars.visuals.hitmarker;
			misc[str("hitmarker")][str("visualize_damage")] = vars.visuals.visualize_damage;
			misc[str("hitmarker")][str("enable_sound")] = vars.visuals.hitmarker_sound;
			misc[str("hitmarker")][str("hitmarker_sound_type")] = vars.visuals.hitmarker_sound_type;

			misc[str("hitmarker")][str("color")][str("red")] = vars.visuals.hitmarker_color.get_red();
			misc[str("hitmarker")][str("color")][str("green")] = vars.visuals.hitmarker_color.get_green();
			misc[str("hitmarker")][str("color")][str("blue")] = vars.visuals.hitmarker_color.get_blue();
			misc[str("hitmarker")][str("color")][str("alpha")] = vars.visuals.hitmarker_color.get_alpha();
		}
	}

	auto& world = json[str("world")]; {
		auto& weapons = world[str("weapons")]; {
			weapons[str("enable")] = vars.visuals.world.weapons.enabled;
			weapons[str("planted_bomb")] = vars.visuals.world.weapons.planted_bomb;
			SaveColor(vars.visuals.world.weapons.color, str("color"), &weapons);
		}
		auto& projectiles = world[str("projectiles")]; {
			projectiles[str("filter")] = vars.visuals.world.projectiles.filter;
			projectiles[str("enable")] = vars.visuals.world.projectiles.enable;
			projectiles[str("trajectories")] = vars.visuals.world.projectiles.trajectories;
			SaveColor(vars.visuals.world.projectiles.colors[0], str("trajectory_team"), &projectiles);
			SaveColor(vars.visuals.world.projectiles.colors[1], str("trajectory_enemy"), &projectiles);
			SaveColor(vars.visuals.world.projectiles.colors[2], str("trajectory_local"), &projectiles);
		}
		auto& tracers = world[str("tracers")]; {
			tracers[str("bullet_tracer")][str("size")] = vars.visuals.impacts_size;
			tracers[str("bullet_tracer")][str("enable")] = vars.visuals.bullet_tracer; {
				tracers[str("bullet_tracer")][str("sprite")] = vars.visuals.bullet_tracer_type;
				tracers[str("bullet_tracer")][str("color")][str("red")] = vars.visuals.bullet_tracer_color.get_red();
				tracers[str("bullet_tracer")][str("color")][str("green")] = vars.visuals.bullet_tracer_color.get_green();
				tracers[str("bullet_tracer")][str("color")][str("blue")] = vars.visuals.bullet_tracer_color.get_blue();
				tracers[str("bullet_tracer")][str("color")][str("alpha")] = vars.visuals.bullet_tracer_color.get_alpha();

				tracers[str("bullet_tracer")][str("local")] = vars.visuals.bullet_tracer_local;
				tracers[str("bullet_tracer")][str("local_color")][str("red")] = vars.visuals.bullet_tracer_local_color.get_red();
				tracers[str("bullet_tracer")][str("local_color")][str("green")] = vars.visuals.bullet_tracer_local_color.get_green();
				tracers[str("bullet_tracer")][str("local_color")][str("blue")] = vars.visuals.bullet_tracer_local_color.get_blue();
				tracers[str("bullet_tracer")][str("local_color")][str("alpha")] = vars.visuals.bullet_tracer_local_color.get_alpha();
			}
			
			tracers[str("bullet_impact")][str("enable")] = vars.visuals.bullet_impact; {
				tracers[str("bullet_impact")][str("color")][str("red")] = vars.visuals.bullet_impact_color.get_red();
				tracers[str("bullet_impact")][str("color")][str("green")] = vars.visuals.bullet_impact_color.get_green();
				tracers[str("bullet_impact")][str("color")][str("blue")] = vars.visuals.bullet_impact_color.get_blue();
				tracers[str("bullet_impact")][str("color")][str("alpha")] = vars.visuals.bullet_impact_color.get_alpha();

				tracers[str("bullet_impact_client")][str("color")][str("red")] = vars.visuals.client_impact_color.get_red();
				tracers[str("bullet_impact_client")][str("color")][str("green")] = vars.visuals.client_impact_color.get_green();
				tracers[str("bullet_impact_client")][str("color")][str("blue")] = vars.visuals.client_impact_color.get_blue();
				tracers[str("bullet_impact_client")][str("color")][str("alpha")] = vars.visuals.client_impact_color.get_alpha();
			}
		}
		auto& effects = world[str("effects")]; {
			
			effects[str("removals")] = vars.visuals.remove;
			effects[str("force_crosshair")] = vars.visuals.force_crosshair;
			effects[str("kill_effect")] = vars.visuals.kill_effect;
			effects[str("world_fov")] = vars.misc.worldfov;
			effects[str("viewmodel_fov")] = vars.misc.viewmodelfov;
			effects[str("zoom_fov")] = vars.misc.zoomfov;

			effects[str("nightmode")][str("enable")] = vars.visuals.nightmode; {
				effects[str("nightmode")][str("customize_color")] = vars.visuals.customize_color;
				effects[str("nightmode")][str("amount")] = vars.visuals.nightmode_amount;
				effects[str("nightmode")][str("prop_alpha")] = vars.visuals.prop_alpha_amount;

				effects[str("nightmode")][str("color")][str("world")][str("red")] = vars.visuals.nightmode_color.get_red();
				effects[str("nightmode")][str("color")][str("world")][str("green")] = vars.visuals.nightmode_color.get_green();
				effects[str("nightmode")][str("color")][str("world")][str("blue")] = vars.visuals.nightmode_color.get_blue();
				effects[str("nightmode")][str("color")][str("world")][str("alpha")] = vars.visuals.nightmode_color.get_alpha();

				effects[str("nightmode")][str("color")][str("prop")][str("red")] = vars.visuals.nightmode_prop_color.get_red();
				effects[str("nightmode")][str("color")][str("prop")][str("green")] = vars.visuals.nightmode_prop_color.get_green();
				effects[str("nightmode")][str("color")][str("prop")][str("blue")] = vars.visuals.nightmode_prop_color.get_blue();
				effects[str("nightmode")][str("color")][str("prop")][str("alpha")] = vars.visuals.nightmode_prop_color.get_alpha();

				effects[str("nightmode")][str("color")][str("skybox")][str("red")] = vars.visuals.nightmode_skybox_color.get_red();
				effects[str("nightmode")][str("color")][str("skybox")][str("green")] = vars.visuals.nightmode_skybox_color.get_green();
				effects[str("nightmode")][str("color")][str("skybox")][str("blue")] = vars.visuals.nightmode_skybox_color.get_blue();
				effects[str("nightmode")][str("color")][str("skybox")][str("alpha")] = vars.visuals.nightmode_skybox_color.get_alpha();
			}
		}
	}

	auto& misc = json[str("misc")]; {
		misc[str("anti_untrusted")] = vars.misc.antiuntrusted;
		misc[str("slidewalk")] = vars.misc.slidewalk;
		misc[str("bunnyhop")] = vars.misc.bunnyhop;
		misc[str("autostrafe")] = vars.misc.autostrafe;
		misc[str("knifebot")] = vars.misc.knifebot;
		misc[str("restrict_type")] = vars.misc.restrict_type;
		misc[str("clantag")] = vars.visuals.clantagspammer;

		misc[str("autobuy")][str("enable")] = vars.misc.autobuy.enable;
		misc[str("autobuy")][str("main")] = vars.misc.autobuy.main;
		misc[str("autobuy")][str("pistol")] = vars.misc.autobuy.pistol;
		misc[str("autobuy")][str("misc")] = vars.misc.autobuy.misc;

		misc[str("viewmodel")][str("x")] = vars.misc.viewmodel_x;
		misc[str("viewmodel")][str("y")] = vars.misc.viewmodel_y;
		misc[str("viewmodel")][str("z")] = vars.misc.viewmodel_z;

		misc[str("agents_ct")] = vars.misc.agents_ct;
		misc[str("agents_t")] = vars.misc.agents_t;
	}

	static TCHAR path[256];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + str("\\weave\\configs\\");
		file = std::string(path) + str("\\weave\\configs\\") + cfg_name;
	}

	auto str = configuration.toStyledString();
	std::vector<uint8_t> vec_enc = std::vector<uint8_t>(str.begin(), str.end());
	xor_crypt(vec_enc);
	auto str_enc = std::string(vec_enc.begin(), vec_enc.end());

	std::ofstream file_out(file, ios::binary | ios::ate);
	if (file_out.good())
		file_out.write(str_enc.data(), str_enc.size());

	file_out.close();
	if (vars.visuals.eventlog & 8)
		Msg(str("Config successfully saved"), vars.visuals.eventlog_color);
}
std::vector<uint8_t> ReadAllBytes(char const* filename)
{
	std::vector<uint8_t> result;

	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	if (ifs)
	{
		std::ifstream::pos_type pos = ifs.tellg();
		result.resize(pos);

		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)&result[0], pos);
	}

	return result;
}

void CConfig::PreLoad(string cfg_name, preload_cfg* p) {
	json_t configuration;
	static TCHAR path[MAX_PATH];
	std::string folder, file;
	memset(p, 0, sizeof(preload_cfg));
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + str("\\weave\\configs\\");
		file = std::string(path) + str("\\weave\\configs\\") + cfg_name;
	}
	std::vector<uint8_t> vec_enc;

	vec_enc = ReadAllBytes(file.c_str());

	if (vec_enc.empty()) {
		if (vars.visuals.eventlog & 8)
			Msg(str("Config cannot be loaded; empty"), vars.visuals.eventlog_color);
		p->can_be_loaded = false;
		return;
	}

	std::vector<uint8_t> vec_dec = vec_enc;
	xor_crypt(vec_dec);
	std::stringstream stream_dec;
	auto str_dec = std::string(vec_dec.begin(), vec_dec.end());
	stream_dec << str_dec;
	stream_dec >> configuration;

	if (!configuration.isMember(str("config"))) {
		if (vars.visuals.eventlog & 8)
			Msg(str("Config cannot be loaded"), vars.visuals.eventlog_color);
		p->can_be_loaded = false;
		return;
	}

	auto& json = configuration[str("config")];

	if (json.isMember(str("created_by")))
		p->created_by = json[str("created_by")].asString();

	if (json.isMember(str("created_at")))
		p->created_at = json[str("created_at")].asString();

	if (json.isMember(str("last_modified_date")))
		p->last_modified_date = json[str("last_modified_date")].asString();

	if (json.isMember(str("last_modified_user")))
		p->last_modified_user = json[str("last_modified_user")].asString();

	p->can_be_loaded = true;
}

void CConfig::Load(string cfg_name)
{
	json_t configuration;
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + str("\\weave\\configs\\");
		file = std::string(path) + str("\\weave\\configs\\") + cfg_name;
	}
	std::vector<uint8_t> vec_enc;

	vec_enc = ReadAllBytes(file.c_str());
	
	if (vec_enc.empty()) {
		if (vars.visuals.eventlog & 8)
			Msg(str("Config cannot be loaded; empty"),  vars.visuals.eventlog_color);
		return;
	}
	
	//auto vec_dec = bXor::decrypt(vec_enc, xs64_extp_key);
	std::vector<uint8_t> vec_dec = vec_enc;
	xor_crypt(vec_dec);
	std::stringstream stream_dec;
	auto str_dec = std::string(vec_dec.begin(), vec_dec.end());
	stream_dec << str_dec;
	stream_dec >> configuration;

	if (!configuration.isMember(str("config"))) {
		if (vars.visuals.eventlog & 8)
			Msg(str("Config cannot be loaded"), vars.visuals.eventlog_color);
		return;
	}

	auto& json = configuration[str("config")];

	for (uint16_t i = 0U; i < c_style::e_style_id_last; ++i)
		LoadColor(&style.get_color((c_style::e_style_id)i), std::string(str("menu_colors_") + std::to_string(i)).c_str(), json);

	auto& legitbot = json[str("legitbot")]; {
		LoadBool(&vars.legitbot.enable, str("enable"), legitbot);
		LoadBool(&vars.legitbot.backtrack, str("backtrack"), legitbot);
	}

	auto& ragebot = json[str("ragebot")]; {
		LoadBool(&vars.ragebot.enable, str("enable"), ragebot);
		LoadBool(&vars.ragebot.autoshoot, str("auto_shoot"), ragebot);
		LoadBool(&vars.ragebot.silent, str("silent"), ragebot);
		LoadInt(&vars.ragebot.fov, str("fov"), ragebot);
		LoadInt(&vars.ragebot.autoscope, str("auto_scope"), ragebot);
		LoadBool(&vars.ragebot.resolver, str("resolver"), ragebot);
		LoadBool(&vars.ragebot.zeusbot, str("zeusbot"), ragebot);
		LoadInt(&vars.ragebot.zeuschance, str("zeuschance"), ragebot);
		LoadBind(&g_Binds[bind_override_dmg], str("override_dmg"), ragebot);
		LoadBind(&g_Binds[bind_force_safepoint], str("force_safepoint"), ragebot);
		LoadBind(&g_Binds[bind_baim], str("baim"), ragebot);
		LoadBool(&vars.ragebot.dt_teleport, str("dt_teleport"), ragebot);
		LoadInt(&vars.ragebot.recharge_time, str("recharge_time"), ragebot);
		LoadBind(&g_Binds[bind_double_tap], str("double_tap"), ragebot);
		LoadBind(&g_Binds[bind_hide_shots], str("hide_shots"), ragebot);

		auto& weapon_cfg = ragebot[str("weapon_cfg")];
		for (int i = 0; i < 7; i++) {
			string category = str("weapon_") + std::to_string(i);
			LoadBool(&vars.ragebot.weapon[i].enable, str("enable"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].mindamage, str("mindamage"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].mindamage_override, str("mindamage_override"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].hitchance, str("hitchance"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].doubletap_hc, str("doubletap_hc"), weapon_cfg[category]);
			LoadBool(&vars.ragebot.weapon[i].multipoint, str("multipoint"), weapon_cfg[category]);
			LoadBool(&vars.ragebot.weapon[i].quickstop, str("quickstop"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].quickstop_options, str("quickstop_options"), weapon_cfg[category]);
			LoadUInt(&vars.ragebot.weapon[i].hitscan, str("hitboxes"), weapon_cfg[category]);
			LoadBool(&vars.ragebot.weapon[i].static_scale, str("static_scale"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].scale_body, str("scale_body"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].scale_head, str("scale_head"), weapon_cfg[category]);
			LoadUInt(&vars.ragebot.weapon[i].prefer_safepoint, str("prefer_safepoint"), weapon_cfg[category]);
			LoadUInt(&vars.ragebot.weapon[i].force_safepoint, str("force_safepoint"), weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].max_misses, str("max_misses"), weapon_cfg[category]);
		}
	}

	auto& antiaim = json[str("antiaim")]; {
		LoadBool(&vars.antiaim.enable, str("enable"), antiaim);
		LoadBool(&vars.antiaim.aa_on_use, str("aa_on_use"), antiaim);
		LoadBool(&vars.antiaim.desync, str("desync"), antiaim);
		LoadInt(&vars.antiaim.pitch, str("pitch"), antiaim);
		LoadInt(&vars.antiaim.yaw, str("yaw"), antiaim);
		LoadInt(&vars.antiaim.jitter_angle, str("jitter_angle"), antiaim);
		LoadBool(&vars.antiaim.attarget, str("at_target"), antiaim);
		LoadBool(&vars.antiaim.attarget_off_when_offsreen, str("attarget_off_when_offsreen"), antiaim);

		LoadBind(&g_Binds[bind_fake_duck], str("fakeduck"), antiaim);
		LoadBind(&g_Binds[bind_slow_walk], str("slowwalk"), antiaim);

		LoadBind(&g_Binds[bind_aa_inverter], str("inverter"), antiaim);
		LoadInt(&vars.antiaim.desync_direction, str("desync_direction"), antiaim);
		LoadInt(&vars.antiaim.desync_amount, str("desync_amount"), antiaim);

		LoadBool(&vars.antiaim.manual_antiaim, str("manual_antiaim"), antiaim);
		LoadBool(&vars.antiaim.ignore_attarget, str("ignore_attarget"), antiaim);

		LoadBind(&g_Binds[bind_manual_left], str("manual_left"), antiaim);
		LoadBind(&g_Binds[bind_manual_right], str("manual_right"), antiaim);
		LoadBind(&g_Binds[bind_manual_back], str("manual_back"), antiaim);
		LoadBind(&g_Binds[bind_manual_forward], str("manual_forward"), antiaim);

		auto& fakelag = antiaim[str("fakelag")]; {
			LoadInt(&vars.antiaim.fakelag, str("type"), fakelag);
			LoadBool(&vars.antiaim.fakelag_when_standing, str("when_standing"), fakelag);
			LoadBool(&vars.antiaim.fakelag_when_exploits, str("when_exploits"), fakelag);
			LoadBool(&vars.antiaim.fakelag_on_peek, str("on_peek"), fakelag);
			LoadInt(&vars.antiaim.fakelagfactor, str("factor"), fakelag);
		}
	}

	auto& players = json[str("players")]; {
		auto& esp = players[str("esp")];

		LoadBool(&vars.visuals.enable, str("enable"), esp);
		LoadBool(&vars.visuals.dormant, str("dormant"), esp);
		LoadBool(&vars.visuals.zeus_warning, str("zeus_warning"), esp);
		LoadBool(&vars.visuals.skeleton, str("skeleton"), esp);
		LoadColor(&vars.visuals.skeleton_color, str("skeleton_color"), esp);
		LoadBool(&vars.visuals.box, str("box"), esp);
		LoadColor(&vars.visuals.box_color, str("box_color"), esp);
		LoadBool(&vars.visuals.healthbar, str("enable"), esp[str("healthbar")]);
		LoadBool(&vars.visuals.override_hp, str("override_hp"), esp[str("healthbar")]);
		LoadColor(&vars.visuals.hp_color, str("hp_color"), esp[str("healthbar")]);

		LoadBool(&vars.visuals.name, str("name"), esp);
		LoadColor(&vars.visuals.name_color, str("name_color"), esp);

		LoadBool(&vars.visuals.weapon, str("weapon"), esp);
		LoadColor(&vars.visuals.weapon_color, str("weapon_color"), esp);

		LoadBool(&vars.visuals.ammo, str("ammo"), esp);
		LoadColor(&vars.visuals.ammo_color, str("ammo_color"), esp);

		LoadUInt(&vars.visuals.flags, str("flags"), esp);
		LoadColor(&vars.visuals.flags_color, str("flags_color"), esp);
		LoadBool(&vars.visuals.shot_multipoint, str("show_multipoint"), esp);
		LoadBool(&vars.visuals.out_of_fov, str("enable"), esp[str("out_of_fov")]);
		LoadInt(&vars.visuals.out_of_fov_distance, str("distance"), esp[str("out_of_fov")]);
		LoadInt(&vars.visuals.out_of_fov_size, str("size"), esp[str("out_of_fov")]);
		LoadColor(&vars.visuals.out_of_fov_color, str("color"), esp[str("out_of_fov")]);

		auto& models = players[str("models")]; {  
			auto& chams_cfg = models[str("chams")]; {
				LoadBool(&vars.visuals.ragdoll_force, str("ragdoll_force"), chams_cfg);
				LoadBool(&vars.visuals.blend_on_scope, str("blend_on_scope"), chams_cfg);
				LoadInt(&vars.visuals.blend_value, str("blend_value"), chams_cfg);

				for (int i = 0; i < chams_max; i++)
				{
					std::string category = str("model_") + std::to_string(i);

					auto& chams_category = chams_cfg[category]; {
						LoadBool(&vars.visuals.chams[i].enable, str("enable"), chams_category);
						LoadUInt(&vars.visuals.chams[i].overlay, str("overlay"), chams_category);

						LoadInt(&vars.visuals.chams[i].material, str("type"), chams_category[str("material")]);
						LoadColor(&vars.visuals.chams[i].material_color, str("color"), chams_category[str("material")]);

						LoadColor(&vars.visuals.chams[i].glass_color, str("color"), chams_category[str("glass")]);
						LoadColor(&vars.visuals.chams[i].metallic_color, str("color"), chams_category[str("metallic")]);

						LoadInt(&vars.visuals.chams[i].phong_amount, str("phong_amount"), chams_category[str("metallic")]);
						LoadInt(&vars.visuals.chams[i].rim_amount, str("rim_amount"), chams_category[str("metallic")]);

						for (int j = 0; j < 2; j++)
						{
							std::string second_category = str("glow_") + std::to_string(j);
							LoadColor(&vars.visuals.chams[i].glow_color[j], str("color"), chams_category[second_category]);
						}

						LoadColor(&vars.visuals.chams[i].wireframe_color, str("color"), chams_category[str("wireframe")]);
					}
				}
			}
			auto& glow = models[str("glow")]; {
				LoadInt(&vars.visuals.glowtype, str("style"), glow);
				LoadBool(&vars.visuals.glow, str("enable"), glow[str("enemy")]);
				LoadColor(&vars.visuals.glow_color, str("color"), glow[str("enemy")]);

				LoadBool(&vars.visuals.local_glow, str("enable"), glow[str("local")]);
				LoadColor(&vars.visuals.local_glow_clr, str("color"), glow[str("local")]);
			}
		}

		auto& misc = players[str("misc")]; {
			LoadInt(&vars.visuals.thirdperson_dist, str("distance"), misc[str("thirdperson")]);
			LoadBind(&g_Binds[bind_third_person], str("bind"), misc[str("thirdperson")]);

			LoadBind(&g_Binds[bind_peek_assist], str("peek_assist"), misc);

			LoadBool(&vars.visuals.indicators, str("indicators"), misc);
			LoadInt(&vars.visuals.aspect_ratio, str("aspect_ratio"), misc);
			LoadBool(&vars.visuals.watermark, str("watermark"), misc);
			LoadUInt(&vars.visuals.eventlog, str("enable"), misc[str("eventlog")]);
			LoadColor(&vars.visuals.eventlog_color, str("color"), misc[str("eventlog")]);
	
			LoadUInt(&vars.visuals.hitmarker, str("type"), misc[str("hitmarker")]);
			LoadBool(&vars.visuals.visualize_damage, str("visualize_damage"), misc[str("hitmarker")]);
			LoadBool(&vars.visuals.hitmarker_sound, str("enable_sound"), misc[str("hitmarker")]);
			LoadInt(&vars.visuals.hitmarker_sound_type, str("hitmarker_sound_type"), misc[str("hitmarker")]);
			LoadColor(&vars.visuals.hitmarker_color, str("color"), misc[str("hitmarker")]);

			LoadBool(&vars.visuals.preverse_killfeed, str("enable"), misc[str("preverse_killfeed")]);

			LoadBool(&vars.visuals.nadepred, str("enable"), misc[str("nadepred")]);
			LoadColor(&vars.visuals.nadepred_color, str("color"), misc[str("nadepred")]);
			LoadInt(&vars.visuals.taser_range, str("mode"), misc[str("taser_range")]);
			LoadColor(&vars.visuals.taser_range_color, str("color"), misc[str("taser_range")]);
		}
	}

	auto& world = json[str("world")]; {
		auto& weapons = world[str("weapons")]; {
			LoadBool(&vars.visuals.world.weapons.enabled, str("enable"), weapons);
			LoadBool(&vars.visuals.world.weapons.planted_bomb, str("planted_bomb"), weapons);
			LoadColor(&vars.visuals.world.weapons.color, str("color"), weapons);
		}
		auto& projectiles = world[str("projectiles")]; {
			LoadBool(&vars.visuals.world.projectiles.enable, str("enable"), projectiles);
			LoadUInt(&vars.visuals.world.projectiles.filter, str("filter"), projectiles);
			LoadBool(&vars.visuals.world.projectiles.trajectories, str("trajectories"), projectiles);
			LoadColor(&vars.visuals.world.projectiles.colors[0], str("trajectory_team"), projectiles);
			LoadColor(&vars.visuals.world.projectiles.colors[1], str("trajectory_enemy"), projectiles);
			LoadColor(&vars.visuals.world.projectiles.colors[2], str("trajectory_local"), projectiles);
		}
		auto& tracers = world[str("tracers")]; {
			LoadBool(&vars.visuals.bullet_tracer, str("enable"), tracers[str("bullet_tracer")]);
			LoadInt(&vars.visuals.impacts_size, str("size"), tracers[str("bullet_tracer")]);
			LoadInt(&vars.visuals.bullet_tracer_type, str("sprite"), tracers[str("bullet_tracer")]);
			LoadColor(&vars.visuals.bullet_tracer_color, str("color"), tracers[str("bullet_tracer")]);
			LoadBool(&vars.visuals.bullet_tracer_local, str("local"), tracers[str("bullet_tracer")]);
			LoadColor(&vars.visuals.bullet_tracer_local_color, str("local_color"), tracers[str("bullet_tracer")]);

			LoadBool(&vars.visuals.bullet_impact, str("enable"), tracers[str("bullet_impact")]);
			LoadColor(&vars.visuals.bullet_impact_color, str("color"), tracers[str("bullet_impact")]);
			LoadColor(&vars.visuals.client_impact_color, str("color"), tracers[str("bullet_impact_client")]);
		}
		auto& effects = world[str("effects")]; {
			LoadUInt(&vars.visuals.remove, str("removals"), effects);
			LoadBool(&vars.visuals.force_crosshair, str("force_crosshair"), effects);
			LoadBool(&vars.visuals.kill_effect, str("kill_effect"), effects);
			LoadInt(&vars.misc.worldfov, str("world_fov"), effects);
			LoadInt(&vars.misc.viewmodelfov, str("viewmodel_fov"), effects);
			LoadInt(&vars.misc.zoomfov, str("zoom_fov"), effects);

			LoadBool(&vars.visuals.nightmode, str("enable"), effects[str("nightmode")]);
			LoadBool(&vars.visuals.customize_color, str("customize_color"), effects[str("nightmode")]);
			LoadInt(&vars.visuals.nightmode_amount, str("nightmode_amount"), effects[str("nightmode")]);
			LoadInt(&vars.visuals.prop_alpha_amount, str("prop_alpha"), effects[str("nightmode")]);
			LoadColor(&vars.visuals.nightmode_color, str("world"), effects[str("nightmode")][str("color")]);
			LoadColor(&vars.visuals.nightmode_prop_color, str("prop"), effects[str("nightmode")][str("color")]);
			LoadColor(&vars.visuals.nightmode_skybox_color, str("skybox"), effects[str("nightmode")][str("color")]);
		}
	}

	auto& misc = json[str("misc")]; {
		LoadBool(&vars.misc.antiuntrusted, str("anti_untrusted"), misc);
		LoadBool(&vars.misc.slidewalk, str("slidewalk"), misc);
		LoadBool(&vars.misc.bunnyhop, str("bunnyhop"), misc);
		LoadBool(&vars.misc.autostrafe, str("autostrafe"), misc);
		LoadBool(&vars.misc.knifebot, str("knifebot"), misc);
		LoadInt(&vars.misc.restrict_type, str("restrict_type"), misc);
		LoadBool(&vars.visuals.clantagspammer, str("clantag"), misc);
		LoadBool(&vars.misc.autobuy.enable, str("enable"), misc[str("autobuy")]);
		LoadInt(&vars.misc.autobuy.pistol, str("pistol"), misc[str("autobuy")]);
		LoadInt(&vars.misc.autobuy.main, str("main"), misc[str("autobuy")]);
		LoadUInt(&vars.misc.autobuy.misc, str("misc"), misc[str("autobuy")]);

		LoadInt(&vars.misc.viewmodel_x, str("x"), misc[str("viewmodel")]);
		LoadInt(&vars.misc.viewmodel_y, str("y"), misc[str("viewmodel")]);
		LoadInt(&vars.misc.viewmodel_z, str("z"), misc[str("viewmodel")]);

		LoadInt(&vars.misc.agents_ct, str("agents_ct"), misc);
		LoadInt(&vars.misc.agents_t, str("agents_t"), misc);
	}
	if (vars.visuals.eventlog & 8)
		Msg(str("Config successfully loaded"), vars.visuals.eventlog_color);
}

CConfig Config;
CGlobalVariables vars;