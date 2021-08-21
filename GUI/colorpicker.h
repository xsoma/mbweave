#pragma once
#include "element.h"

class c_window;
class c_colorpicker : public c_element {
private:
	float animation;
	float animations[2];
	void* value;
	bool open;
	int dragging_slider;
	float val_alpha;
	float hue, saturation, brightness;
	struct {
		float animations[2];
		std::vector<std::string> elements = { "Copy", "Paste" };
		bool open = false;
	} dialogue;
	bool(*should_render)();
	void render_sliderbg(Vector2D pos, float alpha);
	float open_animation;
	bool use_alpha_slider;
	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	void update_colors() {
		hue = (*(color_t*)value).get_hue();
		saturation = (*(color_t*)value).get_saturation();
		brightness = (*(color_t*)value).get_brightness();
		val_alpha = (*(color_t*)value).get_alpha() / 255.f;
	}
	void clamp_colors() {
		hue = std::clamp(hue, 0.f, 1.f);
		saturation = std::clamp(saturation, 0.f, 1.f);
		brightness = std::clamp(brightness, 0.f, 1.f);
	}
	c_colorpicker(void* val, bool use_alpha_slider = false, bool(*should_render)() = nullptr) {
		this->value = val;
		this->type = c_elementtype::colorpicker;
		this->dragging_slider = 0;
		this->update_colors();
		this->should_render = should_render;
		this->open = false;
		this->dialogue.open = false;
		this->animation = 0.f;
		this->dialogue.animations[0] = 0.f;
		this->dialogue.animations[1] = 0.f;
		this->animations[0] = 0.f;
		this->animations[1] = 0.f;
		this->open_animation = 0.f;
		this->use_alpha_slider = use_alpha_slider;
		this->hint.clear();
	}
	void change_pointer(void* ptr) { value = ptr; };
	bool update();
	bool hovered();
	void render();
	void special_render() {};
	int get_total_offset() { return 0; };
};