#pragma once
#include "element.h"

class c_slider : public c_element {
private:
	std::string label;
	void* value;
	float pseudo_value;
	float animation, drop_animation;
	float max, min;
	bool dragging;
	int decimal;
	bool(*should_render)();
	float anim_plus, anim_minus;
	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_slider(std::string label, void* val, float min, float max, bool(*should_render)() = nullptr) {
		this->hint = js_variables::find_int((int*)val);
		this->label = label;
		this->value = val;
		this->min = min;
		this->max = max;
		this->should_render = should_render;
		this->dragging = false;

		this->drop_animation = 0.f;
		this->animation = 0.f;
		this->anim_plus = 0.f;
		this->anim_minus = 0.f;

		this->type = c_elementtype::slider;
	}
	c_slider* clone() {
		return new c_slider(this->label, this->value, this->min, this->max, this->should_render);
	}
	void change_pointer(void* ptr) { value = ptr; };
	bool hovered();
	bool update();
	void render();
	void special_render() {};
	int get_total_offset() { 
		if (should_render)
			if (!should_render())
				return 0;
		return 24; };
};