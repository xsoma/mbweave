#pragma once
#include "../duktape/duktape.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>

class c_element;

class c_js_handler {
public:
	struct menu_element
	{
		c_element* p_element;
		uintptr_t element_data;

		menu_element() : p_element(nullptr), element_data(0)
		{
		}
	};
	std::map<std::string, std::map<std::string, menu_element*>> menu_elements;
	c_js_handler() {
		create_context();
	}
	duk_context* ctx;
	void create_context();
	void create_callbacks();
	void callback(std::string);
};

extern c_js_handler js_handler;

class current_event_t;

extern current_event_t current_event;

// API creation tools
#define ex_c_func(func_name) extern duk_ret_t func_name(duk_context* ctx)
#define c_func(func_name) duk_ret_t func_name(duk_context* ctx)

#define add_function(func_c_name, func_js_name, nargs)	duk_push_c_function(ctx, func_c_name, nargs); \
														duk_put_global_string(ctx, func_js_name)

#define begin_object() auto obj_idx = duk_push_object(ctx)

#define add_obj_function(func_c_name, func_js_name, nargs)	duk_push_c_function(ctx, func_c_name, nargs); \
															duk_put_prop_string(ctx, obj_idx, func_js_name)

#define end_object(final_name) duk_put_global_string(ctx, final_name)

// Arguments API
extern std::vector<int>			get_array_int(duk_context* ctx, duk_idx_t idx);
extern std::vector<float>		get_array_float(duk_context* ctx, duk_idx_t idx);
extern std::vector<std::string> get_array_string(duk_context* ctx, duk_idx_t idx);

class Vector2D;
class Vector;

extern Vector2D					get_args_pos2d(duk_context* ctx, duk_idx_t idx);
extern Vector					get_args_pos3d(duk_context* ctx, duk_idx_t idx);

extern uint32_t					get_args_rgb(duk_context* ctx, duk_idx_t idx);
extern uint32_t					get_args_rgba(duk_context* ctx, duk_idx_t idx);

#define args_bool(idx)			duk_require_boolean(ctx, idx)
#define args_uint(idx)			duk_require_uint(ctx, idx)
#define args_int(idx)			duk_require_int(ctx, idx)
#define args_float(idx)			duk_require_number(ctx, idx)
#define args_string(idx)		duk_require_string(ctx, idx)

#define args_array_int(idx)		get_array_int(ctx, idx)
#define args_array_float(idx)	get_array_float(ctx, idx)
#define args_array_string(idx)	get_array_string(ctx, idx)

#define args_pos2d(idx)			get_args_pos2d(ctx, idx)
#define args_pos3d(idx)			get_args_pos3d(ctx, idx)

#define args_rgb(idx)			color_t(get_args_rgb(ctx, idx))
#define args_rgba(idx)			color_t(get_args_rgba(ctx, idx))

// Return value API
#define add_bool(value)		duk_push_boolean(ctx, value)
#define add_int(value)		duk_push_int(ctx, value)
#define add_uint(value)		duk_push_uint(ctx, value)
#define add_float(value)	duk_push_number(ctx, value)
#define add_string(value)	duk_push_string(ctx, value)

#define begin_array() duk_push_array(ctx)
#define add_array_int(arr_idx, value, idx) add_int(value); \
									duk_put_prop_index(ctx, arr_idx, idx)
#define add_array_float(arr_idx, value, idx) add_float(value); \
									duk_put_prop_index(ctx, arr_idx, idx)

#define protected_call(global_obj, expression, err_msg) if (global_obj) expression; \
														else Msg(str("JavaScript: Internal error: Getting value when unavailable: ") \
																			+ std::string(err_msg), color_t(255, 35, 35))

// No XOR needed
#define run_callback(name) []() { \
	static std::string callback_name = str(name); \
	js_handler.callback(callback_name); \
}()