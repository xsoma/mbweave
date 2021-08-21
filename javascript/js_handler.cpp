#include "api.h"
#include "../Hooks.h"

std::vector<std::string> get_array_string(duk_context* ctx, duk_idx_t idx) {
    std::vector<std::string> result;
    if (!duk_is_array(ctx, idx))
    {
        Msg(str("JavaScript: Array required at index ") + std::to_string(idx), color_t(255, 35, 35));
        return result;
    }
    duk_size_t length = duk_get_length(ctx, idx);
    result.resize(length);

    for (duk_size_t i = 0; i < length; i++) {
        duk_get_prop_index(ctx, idx, i);
        result[i] = duk_require_string(ctx, -1);
        duk_pop(ctx);
    }
    return result;
}

std::vector<float> get_array_float(duk_context* ctx, duk_idx_t idx) {
    std::vector<float> result;
    if (!duk_is_array(ctx, idx))
    {
        Msg(str("JavaScript: Array required at index ") + std::to_string(idx), color_t(255, 35, 35));
        return result;
    }
    duk_size_t length = duk_get_length(ctx, idx);
    result.resize(length);

    for (duk_size_t i = 0; i < length; i++) {
        duk_get_prop_index(ctx, idx, i);
        result[i] = duk_require_number(ctx, -1);
        duk_pop(ctx);
    }
    return result;
}

std::vector<int> get_array_int(duk_context* ctx, duk_idx_t idx) {
    std::vector<int> result;
    if (!duk_is_array(ctx, idx))
    {
        Msg(str("JavaScript: Array required at index ") + std::to_string(idx), color_t(255, 35, 35));
        return result;
    }
    duk_size_t length = duk_get_length(ctx, idx);
    result.resize(length);

    for (duk_size_t i = 0; i < length; i++) {
        duk_get_prop_index(ctx, idx, i);
        result[i] = duk_require_int(ctx, -1);
        duk_pop(ctx);
    }
    return result;
}

Vector2D get_args_pos2d(duk_context* ctx, duk_idx_t idx) {
    const auto& pos_raw = get_array_float(ctx, idx);
    if (pos_raw.size() != 2) {
        Msg(str("JavaScript: Position 2D must be strictly 2 numbers; index ") + std::to_string(idx), color_t(255, 35, 35));
        return Vector2D(0, 0);
    }
    return Vector2D(pos_raw[0], pos_raw[1]);
}

Vector get_args_pos3d(duk_context* ctx, duk_idx_t idx) {
    const auto& pos_raw = get_array_float(ctx, idx);
    if (pos_raw.size() != 2) {
        Msg(str("JavaScript: Position 3D must be strictly 3 numbers; index ") + std::to_string(idx), color_t(255, 35, 35));
        return Vector(0, 0, 0);
    }
    return Vector(pos_raw[0], pos_raw[1], pos_raw[2]);
}

uint32_t get_args_rgb(duk_context* ctx, duk_idx_t idx)
{
    const auto& clr_raw = get_array_int(ctx, idx);
    if (clr_raw.size() != 3) {
        Msg(str("JavaScript: Color must be strictly 3 numbers; index ") + std::to_string(idx), color_t(255, 35, 35));
        return 0;
    }
    return color_t(clr_raw[0], clr_raw[1], clr_raw[2], 255).u32();
}

uint32_t get_args_rgba(duk_context* ctx, duk_idx_t idx)
{
    const auto& clr_raw = get_array_int(ctx, idx);
    if (clr_raw.size() != 4) {
        Msg(str("JavaScript: Color must be strictly 4 numbers; index ") + std::to_string(idx), color_t(255, 35, 35));
        return 0;
    }
    return color_t(clr_raw[0], clr_raw[1], clr_raw[2], clr_raw[3]).u32();
}

c_js_handler js_handler;

void c_js_handler::create_callbacks() {
    static std::vector<std::string> callbacks = {
        str("unload"),
        str("render"),
        str("createmove"),

        str("ragebot_fire"),

        // csgo events
        str("player_hurt"),
        str("bullet_impact"),
        str("weapon_fire"),
        str("player_death"),
        str("round_end"),
    };

    std::string callbacks_str = str("const Callbacks = {\n");

    for (const auto& callback : callbacks)
        callbacks_str += callback + str(": {},\n");

    callbacks_str += str("};");

    const std::string& str_to_eval = callbacks_str + str(R"(
        function _register_callback(script_name, cb_type, callback){
            Callbacks[cb_type][script_name].push(callback);
        }

        function call_callbacks(cb_type){
            for(var script_name in Callbacks[cb_type]){
                const script_callbacks = Callbacks[cb_type][script_name];
                for(var i = 0; i < script_callbacks.length; ++i)
                    script_callbacks[i]();
            }
        }
    )");

    if (duk_peval_string(ctx, str_to_eval.c_str()) != 0) {
        Msg(str("JavaScript error: ") + std::string(duk_to_string(ctx, -1)), color_t(255, 35, 35, 255));
    }
}

void c_js_handler::create_context()
{
	ctx = duk_create_heap_default();
    js_api::create_context(ctx);
    create_callbacks();
}

void c_js_handler::callback(std::string location)
{
    if (location.empty())
        return;

    csgo->mtx.lock();

    /* Get current callback. */
    duk_get_global_string(ctx, str("call_callbacks"));

    /* Explicit check for callback existence; log and exit if no callback. */
    if (!duk_is_function(ctx, -1)) {
        Msg(str("JavaScript: Unknown internal error"), color_t(255, 35, 35, 255));
        csgo->mtx.unlock();
        return;
    }

    /* Protected call, log callback errors. */
    duk_push_string(ctx, location.c_str());
    if (duk_pcall(ctx, 1) != 0) {
        Msg(str("JavaScript: Callback failed: '") + std::string(duk_safe_to_string(ctx, -1)) + '\'', color_t(255, 35, 35, 255));
    }
    duk_pop(ctx);
    csgo->mtx.unlock();
    return;
}
