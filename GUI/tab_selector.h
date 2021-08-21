#pragma once
#include "element.h"


class c_tab_selector : public c_element {
private:
	std::string label;
	std::string desc;
	float animation, press_animation;
	bool(*should_render)();
	bool(*active)();
	void* texture;
	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_tab_selector(std::string str, std::string desc, void* texture, void(*_call)(), bool(*active)() = nullptr, bool(*should_render)() = nullptr) {
		this->label = str;
		this->desc = desc;
		this->fn = _call;
		this->type = tab_selector;
		this->texture = texture;
		this->press_animation = 0.f;
		this->animation = 0.f;
		this->active = active;
		this->should_render = should_render;
		this->hint.clear();
	}
	void(*fn)();
	std::string get_label();
	void set_label(std::string label);
	void change_pointer(void* ptr) { };
	bool update();
	bool hovered();
	void render();
	void special_render() {};
	int get_total_offset() {
		if (should_render)
			if (!should_render())
				return 0;
		return 77;
	};
};