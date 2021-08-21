#include "js_handler.h"
#include <map>

namespace js_api {
	extern void create_context(duk_context* ctx);

	namespace js_vars {
		ex_c_func(_get_int);
		ex_c_func(_set_int);

		ex_c_func(_get_uint);
		ex_c_func(_set_uint);

		ex_c_func(_get_bool);
		ex_c_func(_set_bool);

		ex_c_func(set_bind_active);
		ex_c_func(set_bind_type);

		ex_c_func(is_bind_active);
	}

	namespace ui {
		ex_c_func(_add_checkbox);
		ex_c_func(_add_slider);
	}

	namespace cheat {
		ex_c_func(log);
		ex_c_func(log_custom);

		ex_c_func(get_username);

		ex_c_func(get_choked_commands);
		ex_c_func(get_desync_amount);

		ex_c_func(execute_command);

		ex_c_func(print_to_console);
	}

	namespace utils {
		ex_c_func(play_sound);
	}

	namespace math {
		ex_c_func(random_int);
		ex_c_func(random_float);
	}

	namespace render {
		ex_c_func(_picture);

		ex_c_func(text);

		ex_c_func(filled_rect);
		ex_c_func(rect);

		ex_c_func(line);

		ex_c_func(triangle);
		ex_c_func(filled_triangle);	

		ex_c_func(arc);
		ex_c_func(arc_filled);

		ex_c_func(circle);
		ex_c_func(filled_circle);

		ex_c_func(get_screen_size);
		ex_c_func(world_to_screen);

		ex_c_func(enable_aa);
		ex_c_func(disable_aa);
	}

	namespace global_vars {
		ex_c_func(realtime);
		ex_c_func(frame_count);
		ex_c_func(absolute_frametime);
		ex_c_func(curtime);
		ex_c_func(frametime);
		ex_c_func(max_clients);
		ex_c_func(tick_count);
		ex_c_func(interval_per_tick);
	}

	namespace user_cmd {
		ex_c_func(get_buttons);
		ex_c_func(set_buttons);

		ex_c_func(get_command_number);
		ex_c_func(set_command_number);

		ex_c_func(get_tick_count);
		ex_c_func(set_tick_count);

		ex_c_func(get_viewangles);
		ex_c_func(set_viewangles);

		ex_c_func(set_forwardmove);
		ex_c_func(get_forwardmove);

		ex_c_func(set_sidemove);
		ex_c_func(get_sidemove);

		ex_c_func(set_upmove);
		ex_c_func(get_upmove);
	}

	namespace _event {
		ex_c_func(get_int);
		ex_c_func(get_string);

	}

	namespace entity {
		ex_c_func(get_local_player);
		ex_c_func(get_valid_players);

		ex_c_func(get_player_for_user_id);
		
		ex_c_func(get_velocity);
		ex_c_func(get_origin);
		ex_c_func(get_flags);
		ex_c_func(get_name);
	}

	namespace antiaim {
		ex_c_func(get_send_packet);
		ex_c_func(should_work);
		ex_c_func(override_off_pitch);
		ex_c_func(override_off_yaw);
		ex_c_func(override_body_lean);
	}
}
