#pragma once
#include "element.h"

class c_checkbox : public c_element {
private:
	std::string label;
	float animation, press_animation;
	void* value;
	bool (*should_render)();
	std::string hint;
public:
	c_checkbox(std::string label, void* val, bool (*should_render)() = nullptr) {
		this->hint = js_variables::find_bool((bool*)val);
		this->label = label;
		this->value = val;
		this->type = c_elementtype::checkbox;
		this->should_render = should_render;
		this->press_animation = 0.f;
		this->animation = 0.f;
	}
	std::string get_hint() {
		return this->hint;
	}
	c_checkbox* clone() {
		return new c_checkbox(this->label, this->value, this->should_render);
	}
	void set_label(std::string label) {
		this->label = label;
	}
	void set_pointer(void* val) {
		this->value = val;
	}
	void* get_ptr() { return value; };
	bool update();
	bool hovered();
	void render();
	void change_pointer(void* ptr) { value = ptr; };
	void special_render() {};
	int get_total_offset() {
		if (should_render)
			if (!should_render())
				return 0;
		return 24;
	};
};
