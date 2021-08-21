#include "api.h"
#include "js_loader.h"
#include "../Features.h"
#include "../Hooks.h"
#include "../GUI/gui.h"
#include "cache.h"

std::vector<uint8_t> read_all_bytes(std::string filename)
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

namespace js_variables {
	std::map<std::string, int*> ints;
	std::map<std::string, uint32_t*> uints;
	std::map<std::string, bool*> bools;
	std::map<std::string, color_t*> colors;
	std::map<std::string, c_bind*> binds;

	std::string find_bool(bool* ptr) {
		static std::string unk = str("unk");
		std::string ret = unk;
		for (const auto& i : bools) {
			if (i.second == ptr) {
				ret = i.first;
				break;
			}
		}
		return ret;
	}

	std::string find_int(int* ptr) {
		static std::string unk = str("unk");
		std::string ret = unk;
		for (const auto& i : ints) {
			if (i.second == ptr) {
				ret = i.first;
				break;
			}
		}
		return ret;
	}
	std::string find_uint(uint32_t* ptr) {
		static std::string unk = str("unk");
		std::string ret = unk;
		for (const auto& i : uints) {
			if (i.second == ptr) {
				ret = i.first;
				break;
			}
		}
		return ret;
	}
	void link() {
		static auto get_bind_name = [](int id) -> std::string {
			switch (id)
			{
			case bind_override_dmg:    return str("override_damage");
			case bind_force_safepoint: return str("force_safepoints");
			case bind_baim:            return str("body_aim");
			case bind_double_tap:      return str("doubletap");
			case bind_hide_shots:      return str("hide_shots");
			case bind_aa_inverter:     return str("inverter");
			case bind_manual_left:     return str("manual_left");
			case bind_manual_right:    return str("manual_right");
			case bind_manual_back:     return str("manual_back");
			case bind_manual_forward:  return str("manual_forward");
			case bind_fake_duck:       return str("fake_duck");
			case bind_slow_walk:       return str("slow_walk");
			case bind_third_person:    return str("thirdperson");
			case bind_peek_assist:     return str("peek_assist");
			default: return str("?");
			}
		};

		for (int i = 0; i < bind_max; ++i) {
			binds[(get_bind_name(i).c_str())] = &g_Binds[i];
		}

		// legitbot
		bools[str("legitbot.enable")] = &vars.legitbot.enable;
		bools[str("legitbot.backtrack")] = &vars.legitbot.backtrack;

		// ragebot
		bools[str("ragebot.enable")] = &vars.ragebot.enable;
		bools[str("ragebot.silent")] = &vars.ragebot.silent;
		bools[str("ragebot.autoshoot")] = &vars.ragebot.autoshoot;
		bools[str("ragebot.resolver")] = &vars.ragebot.resolver;
		bools[str("ragebot.dt_teleport")] = &vars.ragebot.dt_teleport;
		bools[str("ragebot.zeusbot")] = &vars.ragebot.zeusbot;

		ints[str("ragebot.fov")] = &vars.ragebot.fov;
		ints[str("ragebot.autoscope")] = &vars.ragebot.autoscope;
		ints[str("ragebot.zeuschance")] = &vars.ragebot.zeuschance;

		for (int i = 0; i < 7; ++i) {
			bools[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].enable")).c_str())] = &vars.ragebot.weapon[i].enable;
			bools[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].multipoint")).c_str())] = &vars.ragebot.weapon[i].multipoint;
			bools[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].quickstop")).c_str())] = &vars.ragebot.weapon[i].quickstop;
			bools[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].static_scale")).c_str())] = &vars.ragebot.weapon[i].static_scale;

			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].mindamage")).c_str())] = &vars.ragebot.weapon[i].mindamage;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].mindamage_override")).c_str())] = &vars.ragebot.weapon[i].mindamage_override;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].hitchance")).c_str())] = &vars.ragebot.weapon[i].hitchance;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].doubletap_hc")).c_str())] = &vars.ragebot.weapon[i].doubletap_hc;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].quickstop_options")).c_str())] = &vars.ragebot.weapon[i].quickstop_options;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].max_misses")).c_str())] = &vars.ragebot.weapon[i].max_misses;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].scale_head")).c_str())] = &vars.ragebot.weapon[i].scale_head;
			ints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].scale_body")).c_str())] = &vars.ragebot.weapon[i].scale_body;

			uints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].hitscan")).c_str())] = &vars.ragebot.weapon[i].hitscan;
			uints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].prefer_safepoint")).c_str())] = &vars.ragebot.weapon[i].prefer_safepoint;
			uints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].force_safepoint")).c_str())] = &vars.ragebot.weapon[i].force_safepoint;
			uints[(std::string(str("ragebot.weapons[") + std::to_string(i) + str("].quickstop_options")).c_str())] = (uint32_t*)&vars.ragebot.weapon[i].quickstop_options;
		}

		// anti-aims
		bools[str("antiaim.enable")] = &vars.antiaim.enable;
		bools[str("antiaim.attarget")] = &vars.antiaim.attarget;
		bools[str("antiaim.attarget_off_when_offsreen")] = &vars.antiaim.attarget_off_when_offsreen;
		bools[str("antiaim.aa_on_use")] = &vars.antiaim.aa_on_use;
		bools[str("antiaim.fakelag_when_standing")] = &vars.antiaim.fakelag_when_standing;
		bools[str("antiaim.fakelag_when_exploits")] = &vars.antiaim.fakelag_when_exploits;
		bools[str("antiaim.fakelag_on_peek")] = &vars.antiaim.fakelag_on_peek;
		bools[str("antiaim.manual_antiaim")] = &vars.antiaim.manual_antiaim;
		bools[str("antiaim.ignore_attarget")] = &vars.antiaim.ignore_attarget;

		ints[str("antiaim.pitch")] = &vars.antiaim.pitch;
		ints[str("antiaim.yaw")] = &vars.antiaim.yaw;
		ints[str("antiaim.desync_direction")] = &vars.antiaim.desync_direction;
		ints[str("antiaim.desync_amount")] = &vars.antiaim.desync_amount;
		ints[str("antiaim.fakelag")] = &vars.antiaim.fakelag;
		ints[str("antiaim.fakelagfactor")] = &vars.antiaim.fakelagfactor;
		ints[str("antiaim.jitter_angle")] = &vars.antiaim.jitter_angle;

		bools[str("misc.bunnyhop")] = &vars.misc.bunnyhop;
		bools[str("misc.autostrafe")] = &vars.misc.autostrafe;
		bools[str("misc.knifebot")] = &vars.misc.knifebot;
		bools[str("misc.antiuntrusted")] = &vars.misc.antiuntrusted;
		bools[str("misc.slidewalk")] = &vars.misc.slidewalk;

		ints[str("misc.restrict_type")] = &vars.misc.restrict_type;
		ints[str("misc.viewmodelfov")] = &vars.misc.viewmodelfov;
		ints[str("misc.viewmodel_x")] = &vars.misc.viewmodel_x;
		ints[str("misc.viewmodel_y")] = &vars.misc.viewmodel_y;
		ints[str("misc.viewmodel_z")] = &vars.misc.viewmodel_z;
		ints[str("misc.worldfov")] = &vars.misc.worldfov;
		ints[str("misc.zoomfov")] = &vars.misc.zoomfov;

		bools[str("buybot.enable")] = &vars.misc.autobuy.enable;
		ints[str("buybot.main")] = &vars.misc.autobuy.main;
		ints[str("buybot.pistol")] = &vars.misc.autobuy.pistol;
		uints[str("buybot.misc")] = &vars.misc.autobuy.misc;

		ints[str("agents.ct")] = &vars.misc.agents_t;
		ints[str("agents.t")] = &vars.misc.agents_ct;
	}
}

class current_event_t {
private:
	std::map<std::string, int> ints;
	std::map<std::string, float> floats;
	std::map<std::string, std::string> strings;
	inline void __clear_data() {
		ints.clear();
		floats.clear();
		strings.clear();
	}
	IGameEvent* current_event = nullptr;

public:
	current_event_t() {};
	inline void push(IGameEvent* _event) {
		this->current_event = _event;
	}
	inline void push(std::string key, int v) {
		this->ints[key] = v;
	}
	inline void push(std::string key, float v) {
		this->floats[key] = v;
	}
	inline void push(std::string key, std::string v) {
		this->strings[key] = v;
	}

	inline int get_int(std::string key) {
		if (current_event) {
			return current_event->GetInt(key.c_str());
		}
		else
			return ints[key];
	}

	inline float get_float(std::string key) {
		if (current_event) {
			return current_event->GetFloat(key.c_str());
		}
		else
			return floats[key];
	}

	inline std::string get_string(std::string key) {
		if (current_event) {
			return current_event->GetString(key.c_str());
		}
		else
			return strings[key];
	}

	inline void clear() {
		this->current_event = nullptr;
		__clear_data();
	}
};

current_event_t current_event;

void current_event_push(IGameEvent* e) {
	current_event.push(e);
}
void current_event_push(std::string key, int v) {
	current_event.push(key, v);
}
void current_event_push(std::string key, float v) {
	current_event.push(key, v);
}
void current_event_push(std::string key, std::string v) {
	current_event.push(key, v);
}

void current_event_clear() {
	current_event.clear();
}

namespace js_api {
	namespace js_vars {
		c_func(_get_int) {
			int* pointer;
			const std::string& arg = args_string(1);

			if (arg.starts_with(str("js.")))
			{
				auto& menu_elements = js_handler.menu_elements[args_string(0)];
				const std::string element_name = arg.substr(3);
				if (menu_elements.count(element_name) > 0)
				{
					add_int((int)menu_elements[element_name]->element_data);
					return 1;
				}
				goto err;
			}

			pointer = js_variables::ints[(arg.c_str())];
			if (!pointer)
				goto err;

			add_int(*pointer);
			return 1;

		err:
			Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			return 0;
		}

		c_func(_set_int) {
			int* pointer;
			const std::string& arg = args_string(1);

			if (arg.starts_with(str("js.")))
			{
				auto& menu_elements = js_handler.menu_elements[args_string(0)];
				const std::string element_name = arg.substr(3);
				if (menu_elements.count(element_name) > 0)
				{
					(int&)menu_elements[element_name]->element_data = args_int(2);
					return 1;
				}
				goto err;
			}

			pointer = js_variables::ints[(arg.c_str())];
			if (!pointer)
				goto err;

			*pointer = args_int(2);
			return 0;

		err:
			Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			return 0;
		}

		c_func(_get_uint) {
			uint32_t* pointer;
			const std::string& arg = args_string(1);

			if (arg.starts_with(str("js.")))
			{
				auto& menu_elements = js_handler.menu_elements[args_string(0)];
				const std::string element_name = arg.substr(3);
				if (menu_elements.count(element_name) > 0)
				{
					add_uint((uint32_t)menu_elements[element_name]->element_data);
					return 1;
				}
				goto err;
			}

			pointer = js_variables::uints[(arg.c_str())];
			if (!pointer)
				goto err;

			add_uint(*pointer);
			return 1;

		err:
			Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			return 0;
		}

		c_func(_set_uint) {
			uint32_t* pointer;
			const std::string& arg = args_string(1);

			if (arg.starts_with(str("js.")))
			{
				auto& menu_elements = js_handler.menu_elements[args_string(0)];
				const std::string element_name = arg.substr(3);
				if (menu_elements.count(element_name) > 0)
				{
					(uint32_t&)menu_elements[element_name]->element_data = args_uint(2);
					return 1;
				}
				goto err;
			}

			pointer = js_variables::uints[(arg.c_str())];
			if (!pointer)
				goto err;

			*pointer = args_uint(2);
			return 0;

		err:
			Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			return 0;
		}

		c_func(_get_bool) {
			bool* pointer;
			const std::string& arg = args_string(1);

			if (arg.starts_with(str("js.")))
			{
				auto& menu_elements = js_handler.menu_elements[args_string(0)];
				const std::string element_name = arg.substr(3);
				if (menu_elements.count(element_name) > 0)
				{
					add_bool((bool)menu_elements[element_name]->element_data);
					return 1;
				}
				goto err;
			}

			pointer = js_variables::bools[(arg.c_str())];
			if (!pointer)
				goto err;

			add_bool(*pointer);
			return 1;

		err:
			Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			return 0;
		}

		c_func(_set_bool) {
			bool* pointer;
			const std::string& arg = args_string(1);

			if (arg.starts_with(str("js.")))
			{
				auto& menu_elements = js_handler.menu_elements[args_string(0)];
				const std::string element_name = arg.substr(3);
				if (menu_elements.count(element_name) > 0)
				{
					(bool&)menu_elements[element_name]->element_data = args_bool(2);
					return 1;
				}
				goto err;
			}

			pointer = js_variables::bools[(arg.c_str())];
			if (!pointer)
				goto err;

			*pointer = args_bool(2);
			return 0;

		err:
			Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			return 0;
		}

		c_func(is_bind_active) {
			//csgo->mtx.lock();
			const std::string& arg = args_string(0);
			auto pointer = js_variables::binds[(arg.c_str())];
			if (!pointer) {
				Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			//	csgo->mtx.unlock();
				return 0;
			}
			add_bool(pointer->active);
			//csgo->mtx.unlock();
			return 1;
		}

		c_func(set_bind_active) {
			//csgo->mtx.lock();
			const std::string& arg = args_string(0);
			auto pointer = js_variables::binds[(arg.c_str())];
			if (!pointer) {
				Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			//	csgo->mtx.unlock();
				return 0;
			}
			pointer->active = args_bool(1);
			//csgo->mtx.unlock();
			return 0;
		}

		c_func(set_bind_type) {
		//	csgo->mtx.lock();
			const std::string& arg = args_string(0);
			auto pointer = js_variables::binds[(arg.c_str())];
			if (!pointer) {
				Msg(std::string(str("JavaScript: Unknown var: \'") + arg + '\''), color_t(255, 35, 35));
			//	csgo->mtx.unlock();
				return 0;
			}
			pointer->type = args_int(1);
		//	csgo->mtx.unlock();
			return 0;
		}
	}

	namespace ui {
		c_func(_add_checkbox) {
			std::string script_name = args_string(0),
				checkbox_name = args_string(2),
				checkbox_text = args_string(1);

			auto menu_element = new c_js_handler::menu_element();
			menu_element->p_element = new c_checkbox(checkbox_text, &menu_element->element_data);
			js_handler.menu_elements[script_name][checkbox_name] = menu_element;

			return 0;
		}

		c_func(_add_slider) {
			std::string script_name = args_string(0),
				slider_name = args_string(2),
				slider_text = args_string(1);

			auto menu_element = new c_js_handler::menu_element();
			menu_element->p_element = new c_slider(slider_text, &menu_element->element_data, args_int(3), args_int(4));
			js_handler.menu_elements[script_name][slider_name] = menu_element;

			return 0;
		}
	}

	namespace cheat {
		c_func(log_custom) {
			Msg(args_string(0), args_rgb(1));
			return 0;
		}
		c_func(log) {
			Msg(args_string(0), vars.visuals.eventlog_color);
			return 0;
		}

		c_func(get_username) {
			protected_call(csgo, add_string(csgo->username.c_str()), str("cheat.get_username"));
			return 1;
		}

		c_func(get_choked_commands) {
			protected_call(csgo->client_state, add_int(std::clamp(csgo->client_state->iChokedCommands, 0, 14)), str("cheat.get_choked_commands"));
			return 1;
		}

		c_func(get_desync_amount) {
			add_float(csgo->desync_angle);
			return 1;
		}

		c_func(execute_command) {
			interfaces.engine->ExecuteClientCmd(args_string(0));
			return 0;
		}
		c_func(print_to_console) {
			interfaces.cvars->ConsoleColorPrintf(args_rgb(1), args_string(0));
			return 0;
		}
	}

	namespace utils {
		c_func(play_sound) {
			std::string name = args_string(0);
			PlaySound(TEXT(name.c_str()), NULL, SND_FILENAME | SND_ASYNC);
			return 0;
		}
	}

	namespace math {
		c_func(random_int) {
			add_int(Math::RandomInt(args_int(0), args_int(1)));
			return 1;
		}
		c_func(random_float) {
			add_float(Math::RandomFloat(args_float(0), args_float(1)));
			return 1;
		}
	}

	namespace render {

		c_func(_picture) {
			std::string imagePath = args_string(1);
			auto& script_cache = scripts_cache[args_string(0)];
			if(script_cache.images.count(imagePath) == 0)
			{
				LPDIRECT3DTEXTURE9 image = nullptr;
				auto imageVec = read_all_bytes(imagePath);
				if ((imageVec.size() > 0) && SUCCEEDED(D3DXCreateTextureFromFileInMemory(g_Render->GetDevice(), &imageVec[0], imageVec.size(), &image)))
					script_cache.images[imagePath] = image;
				else
					return 0;
			}

			auto pos = args_pos2d(2);
			auto size = args_pos2d(3);

			g_Render->_drawList->AddImage((void*)script_cache.images[imagePath], ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y),
				ImVec2(0, 0), ImVec2(1, 1), color_t(255, 255, 255, args_int(4)).u32());

			return 0;
		}

		c_func(text) {
			int font_id = args_int(3);
			ImFont* font = nullptr;
			switch (font_id)
			{
			case 0: font = fonts::esp_name; break;
			case 1: font = fonts::esp_info; break;
			case 2: font = fonts::esp_logs; break;
			case 3: font = fonts::lby_indicator; break;
			case 4: font = fonts::menu_main; break;
			case 5: font = fonts::menu_desc; break;
			default: font = fonts::menu_desc; break;
			}
			auto pos = args_pos2d(0);
			g_Render->DrawString(pos.x, pos.y, args_rgba(1), args_int(2), font, args_string(4));
			return 0;
		}

		c_func(filled_rect) {
			const auto pos = args_pos2d(0);
			const auto size = args_pos2d(1);
			g_Render->FilledRect(pos.x, pos.y, size.x, size.y, args_rgba(2), args_float(3));
			return 0;
		}

		c_func(rect) {
			const auto pos = args_pos2d(0);
			const auto size = args_pos2d(1);
			g_Render->Rect(pos.x, pos.y, size.x, size.y, args_rgba(2), args_float(3));
			return 0;
		}

		c_func(line) {
			const auto pos1 = args_pos2d(0);
			const auto pos2 = args_pos2d(1);
			g_Render->DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, args_rgba(2), args_float(3));
			return 0;
		}

		c_func(triangle) {
			const auto pos1 = args_pos2d(0);
			const auto pos2 = args_pos2d(1);
			const auto pos3 = args_pos2d(2);

			g_Render->Triangle(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y, args_rgba(3), args_float(4));
			return 0;
		}

		c_func(filled_triangle) {
			const auto pos1 = args_pos2d(0);
			const auto pos2 = args_pos2d(1);
			const auto pos3 = args_pos2d(2);

			g_Render->TriangleFilled(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y, args_rgba(3));
			return 0;
		}

		c_func(arc) {
			const auto pos = args_pos2d(0);
			g_Render->_drawList->PathArcTo(ImVec2(pos.x, pos.y), args_float(1), args_float(2), args_float(3), args_int(4));
			g_Render->_drawList->PathStroke(args_rgba(5).u32(), args_bool(6), args_float(7));
			return 0;
		}

		c_func(arc_filled) {
			const auto pos = args_pos2d(0);
			g_Render->_drawList->PathArcTo(ImVec2(pos.x, pos.y), args_float(1), args_float(2), args_float(3), args_int(4));
			g_Render->_drawList->PathFillConvex(args_rgba(5).u32());
			return 0;
		}

		c_func(circle) {
			const auto pos = args_pos2d(0);
			g_Render->Circle(pos.x, pos.y, args_float(1),
				args_rgba(2), args_int(3));
			return 0;
		}

		c_func(filled_circle) {
			const auto pos = args_pos2d(0);
			g_Render->CircleFilled(pos.x, pos.y, args_float(1),
				args_rgba(2), args_int(3));
			return 0;
		}

		ex_c_func(enable_aa) {
			g_Render->_drawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
			return 0;
		}

		ex_c_func(disable_aa) {
			g_Render->_drawList->Flags = ImDrawListFlags_None;
			return 0;
		}

		c_func(get_screen_size) {
			protected_call(csgo, [&]() {
				auto idx = begin_array();
				add_array_float(idx, csgo->w, 0);
				add_array_float(idx, csgo->h, 1);
			}(), str("render.get_screen_size"));
			return 1;
		}
		
		c_func(world_to_screen) {
			Vector ret = Vector(0, 0, 0);
			auto pos = args_pos3d(0);
			Math::WorldToScreen(pos, ret);
			auto idx = begin_array();
			add_array_float(idx, ret.x, 0);
			add_array_float(idx, ret.y, 1);
			return 1;
		}
	}

	namespace global_vars {
		c_func(realtime) {
			protected_call(interfaces.global_vars, add_float(interfaces.global_vars->realtime), str("global_vars.realtime")); 
			return 1; 
		};
		c_func(frame_count) { 
			protected_call(interfaces.global_vars, add_int(interfaces.global_vars->framecount), str("global_vars.frame_count")); 
			return 1; 
		};
		c_func(absolute_frametime) {
			protected_call(interfaces.global_vars, add_float(interfaces.global_vars->absoluteframetime), str("global_vars.absolute_frametime")); 
			return 1;
		};
		c_func(curtime) {
			protected_call(interfaces.global_vars, add_float(interfaces.global_vars->curtime), str("global_vars.curtime")); 
			return 1; 
		};
		c_func(frametime) {
			protected_call(interfaces.global_vars, add_float(interfaces.global_vars->frametime), str("global_vars.frametime")); 
			return 1;
		};
		c_func(max_clients) {
			protected_call(interfaces.global_vars, add_int(interfaces.global_vars->maxClients), str("global_vars.max_clients"));
			return 1; 
		};
		c_func(tick_count) {
			protected_call(interfaces.global_vars, add_int(interfaces.global_vars->tickcount), str("global_vars.tick_count")); 
			return 1;
		};
		c_func(interval_per_tick) {
			protected_call(interfaces.global_vars, add_float(interfaces.global_vars->interval_per_tick), str("global_vars.interval_per_tick")); 
			return 1; 
		};
	}

	namespace user_cmd {
		c_func(get_buttons) {
			protected_call(csgo->cmd, add_int(csgo->cmd->buttons), str("user_cmd.get_buttons"));
			return 1;
		}

		c_func(set_buttons) {
			protected_call(csgo->cmd, csgo->cmd->buttons = args_int(0), str("user_cmd.set_buttons"));
			return 0;
		}

		c_func(get_command_number) {
			protected_call(csgo->cmd, add_int(csgo->cmd->command_number), str("user_cmd.get_command_number"));
			return 1;
		}

		c_func(set_command_number) {
			protected_call(csgo->cmd, csgo->cmd->command_number = args_int(0), str("user_cmd.set_command_number"));
			return 0;
		}

		c_func(get_tick_count) {
			protected_call(csgo->cmd, add_int(csgo->cmd->tick_count), str("user_cmd.get_tick_count"));
			return 1;
		}

		c_func(set_tick_count) {
			protected_call(csgo->cmd, csgo->cmd->tick_count = args_int(0), str("user_cmd.set_tick_count"));
			return 0;
		}
		
		c_func(get_view_angles) {
			protected_call(csgo->cmd, [&]() {
				auto idx = begin_array();
				add_array_float(idx, csgo->cmd->viewangles.x, 0);
				add_array_float(idx, csgo->cmd->viewangles.y, 1);
				add_array_float(idx, csgo->cmd->viewangles.z, 2);
			}(), str("user_cmd.get_view_angles"));
			return 1;
		}

		c_func(set_view_angles) {
			protected_call(csgo->cmd, csgo->cmd->viewangles = Math::normalize(args_pos3d(0)), str("user_cmd.set_view_angles"));
			return 0;
		}

		c_func(set_forwardmove) {
			protected_call(csgo->cmd, csgo->cmd->forwardmove = args_float(0), str("user_cmd.set_forwardmove"));
			return 0;
		};

		c_func(get_forwardmove) {
			protected_call(csgo->cmd, add_float(csgo->cmd->forwardmove), str("user_cmd.get_forwardmove"));
			return 1;
		};

		c_func(set_sidemove) {
			protected_call(csgo->cmd, csgo->cmd->sidemove = args_float(0), str("user_cmd.set_sidemove"));
			return 0;
		};

		c_func(get_sidemove) {
			protected_call(csgo->cmd, add_float(csgo->cmd->sidemove), str("user_cmd.get_sidemove"));
			return 1;
		};

		c_func(set_upmove) {
			protected_call(csgo->cmd, csgo->cmd->upmove = args_float(0), str("user_cmd.set_upmove"));
			return 0;
		};

		c_func(get_upmove) {
			protected_call(csgo->cmd, add_float(csgo->cmd->upmove), str("user_cmd.get_upmove"));
			return 1;
		};

	}

	namespace _event {
		c_func(get_int) {
			add_int(current_event.get_int(args_string(0)));
			return 1;
		}
		c_func(get_float) {
			add_float(current_event.get_float(args_string(0)));
			return 1;
		}
		c_func(get_string) {
			add_string(current_event.get_string(args_string(0)).c_str());
			return 1;
		}
	}

	namespace entity {
		c_func(get_local_player) {
			add_int(interfaces.engine->GetLocalPlayer());
			return 1;
		}
		c_func(get_valid_players) {
			auto idx = begin_array();
			for (int i = 0; i < g_Cache->valid_players.size(); ++i) {
				add_array_int(idx, g_Cache->valid_players[i], i);
			}
			return 1;
		}
		c_func(get_velocity) {
			const auto& velocity = g_Cache->player_info[args_int(0)].velocity;
			auto idx = begin_array();
			add_array_float(idx, velocity.x, 0);
			add_array_float(idx, velocity.y, 1);
			add_array_float(idx, velocity.z, 2);
			return 1;
		}
		c_func(get_origin) {
			const auto& vec_origin = g_Cache->player_info[args_int(0)].vec_origin;
			auto idx = begin_array();
			add_array_float(idx, vec_origin.x, 0);
			add_array_float(idx, vec_origin.y, 1);
			add_array_float(idx, vec_origin.z, 2);
			return 1;
		}
		c_func(get_flags) {
			add_int(g_Cache->player_info[args_int(0)].flags);
			return 1;
		}
		c_func(get_name) {
			add_string(g_Cache->player_info[args_int(0)].name.c_str());
			return 1;
		}
		c_func(get_player_for_user_id) {
			add_int(interfaces.engine->GetPlayerForUserID(args_int(0)));
			return 1;
		}
	}

	namespace antiaim {
		c_func(should_work) {
			add_bool(g_AntiAim->ShouldAA());
			return 1;
		}
		c_func(get_send_packet) {
			add_bool(csgo->send_packet);
			return 1;
		}

		c_func(override_off_pitch) {
			g_AntiAim->override_off_pitch = true;
			return 0;
		}

		c_func(override_off_yaw) {
			g_AntiAim->override_off_yaw = true;
			return 0;
		}

		c_func(override_body_lean) {
			g_AntiAim->body_lean = args_float(0);
			return 0;
		}
	}

	void create_context(duk_context* ctx) {
		js_variables::link();
		// cheat
		{
			begin_object();

			add_obj_function(js_vars::_get_bool, str("_get_bool"), 2);
			add_obj_function(js_vars::_set_bool, str("_set_bool"), 3);

			add_obj_function(js_vars::_get_uint, str("_get_uint"), 2);
			add_obj_function(js_vars::_set_uint, str("_set_uint"), 3);

			add_obj_function(js_vars::_get_int, str("_get_int"), 2);
			add_obj_function(js_vars::_set_int, str("_set_int"), 3);

			add_obj_function(js_vars::is_bind_active, str("is_bind_active"), 1);
			add_obj_function(js_vars::set_bind_active, str("set_bind_active"), 2);
			add_obj_function(js_vars::set_bind_type, str("set_bind_type"), 2);

			end_object(str("vars"));
		}

		// ui
		{
			begin_object();

			add_obj_function(ui::_add_checkbox, str("_add_checkbox"), 3);
			add_obj_function(ui::_add_slider, str("_add_slider"), 5);

			end_object(str("ui"));
		}

		// math
		{
			begin_object();

			add_obj_function(math::random_int, str("random_int"), 2);
			add_obj_function(math::random_float, str("random_float"), 2);

			end_object(str("math"));
		}

		// render
		{
			begin_object();

			add_obj_function(render::_picture, str("_picture"), 5);

			add_obj_function(render::text, str("text"), 5);

			add_obj_function(render::filled_rect, str("filled_rect"), 4);
			add_obj_function(render::rect, str("rect"), 4);

			add_obj_function(render::line, str("line"), 4);

			add_obj_function(render::triangle, str("triangle"), 5);
			add_obj_function(render::filled_triangle, str("filled_triangle"), 5);

			add_obj_function(render::arc, str("arc"), 8);
			add_obj_function(render::arc_filled, str("arc_filled"), 6);

			add_obj_function(render::circle, str("circle"), 4);
			add_obj_function(render::filled_circle, str("filled_circle"), 4);

			add_obj_function(render::world_to_screen, str("world_to_screen"), 1);
			add_obj_function(render::get_screen_size, str("get_screen_size"), 0);
			add_obj_function(render::enable_aa, str("enable_aa"), 0);
			add_obj_function(render::disable_aa, str("disable_aa"), 0);

			end_object(str("render"));
		}

		// global_vars
		{
			begin_object();

			add_obj_function(global_vars::realtime, str("realtime"), 0);
			add_obj_function(global_vars::frame_count, str("frame_count"), 0);
			add_obj_function(global_vars::absolute_frametime, str("absolute_frametime"), 0);
			add_obj_function(global_vars::curtime, str("curtime"), 0);
			add_obj_function(global_vars::frametime, str("frametime"), 0);
			add_obj_function(global_vars::max_clients, str("max_clients"), 0);
			add_obj_function(global_vars::tick_count, str("tick_count"), 0);
			add_obj_function(global_vars::interval_per_tick, str("interval_per_tick"), 0);

			end_object(str("global_vars"));
		}

		// event
		{
			begin_object();

			add_obj_function(_event::get_int, str("get_int"), 1);
			add_obj_function(_event::get_float, str("get_float"), 1);
			add_obj_function(_event::get_string, str("get_string"), 1);

			end_object(str("current_event"));
		}

		// user_cmd
		{
			begin_object();

			add_obj_function(user_cmd::get_buttons, str("get_buttons"), 0);
			add_obj_function(user_cmd::set_buttons, str("set_buttons"), 1);

			add_obj_function(user_cmd::get_command_number, str("get_command_number"), 0);
			add_obj_function(user_cmd::set_command_number, str("set_command_number"), 1);

			add_obj_function(user_cmd::get_tick_count, str("get_tick_count"), 0);
			add_obj_function(user_cmd::set_tick_count, str("set_tick_count"), 1);

			add_obj_function(user_cmd::get_view_angles, str("get_view_angles"), 0);
			add_obj_function(user_cmd::set_view_angles, str("set_view_angles"), 1);

			add_obj_function(user_cmd::set_forwardmove, str("set_forwardmove"), 1);
			add_obj_function(user_cmd::get_forwardmove, str("get_forwardmove"), 0);

			add_obj_function(user_cmd::set_sidemove, str("set_sidemove"), 1);
			add_obj_function(user_cmd::get_sidemove, str("get_sidemove"), 0);

			add_obj_function(user_cmd::set_upmove, str("set_upmove"), 1);
			add_obj_function(user_cmd::get_upmove, str("get_upmove"), 0);

			end_object(str("user_cmd"));
		}

		// utils
		{
			begin_object();

			add_obj_function(utils::play_sound, str("play_sound"), 1);

			end_object(str("utils"));
		}

		// cheat
		{
			begin_object();

			add_obj_function(cheat::log, str("log"), 1);
			add_obj_function(cheat::log_custom, str("log_custom"), 2);
			add_obj_function(cheat::get_username, str("get_username"), 0);
			add_obj_function(cheat::get_choked_commands, str("get_choked_commands"), 0);
			add_obj_function(cheat::get_desync_amount, str("get_desync_amount"), 0);
			add_obj_function(cheat::execute_command, str("execute_command"), 1);
			add_obj_function(cheat::print_to_console, str("print_to_console"), 2);

			end_object(str("cheat"));
		}
	
		// entity
		{
			begin_object();

			add_obj_function(entity::get_local_player, str("get_local_player"), 0);
			add_obj_function(entity::get_valid_players, str("get_valid_players"), 0);

			add_obj_function(entity::get_velocity, str("get_velocity"), 1);
			add_obj_function(entity::get_origin, str("get_origin"), 1);
			add_obj_function(entity::get_flags, str("get_flags"), 1);
			add_obj_function(entity::get_name, str("get_name"), 1);

			add_obj_function(entity::get_player_for_user_id, str("get_player_for_user_id"), 1);

			end_object(str("entity"));
		}

		// antiaim
		{
			begin_object();

			add_obj_function(antiaim::get_send_packet, str("get_send_packet"), 0);
			add_obj_function(antiaim::should_work, str("should_work"), 0);

			add_obj_function(antiaim::override_body_lean, str("override_body_lean"), 1);
			add_obj_function(antiaim::override_off_pitch, str("override_off_pitch"), 0);
			add_obj_function(antiaim::override_off_yaw, str("override_off_yaw"), 0);

			end_object(str("antiaim"));
		}
	}
}