#pragma once
#include "element.h"

class c_keybind : public c_element {
private:
	float animation;
	float reading_animation;
	bool(*should_render)();
	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	std::string label;
	c_bind* bind;
	c_keybind(std::string label, c_bind* bind, bool(*should_render)() = nullptr) {
		this->label = label;
		this->bind = bind;
		this->type = c_elementtype::keybind;
		this->binder.open = false;
		this->binder.active = false;
		this->should_render = should_render;
		for (int i = 0; i < binder.elements.size(); i++)
			this->binder.animations[i] = 0.f;
		for (int i = 0; i < binder.elements.size(); i++)
			this->binder.animations2[i] = 0.f;
		this->animation = 0.f;
		this->binder.open_animation = 0.f;
		this->hint.clear();
		this->pulsating = 0.f;
		this->b_switch = 0.f;
	}
	struct {
		bool open, active;
		float open_animation;
		float animations[5];
		float animations2[5];
		float animation;
		std::vector<std::string> elements = { "Off", "Hold", "Toggle", "Release", "On" };
	} binder;
	float pulsating;
	bool b_switch;
	std::string get_label() { return this->label; }
	bool update();
	bool hovered();
	void render();
	void change_pointer(void* ptr) { };
	void special_render() {};
	int get_total_offset() {
		if (should_render)
			if (!should_render())
				return 0;
		return 24;
	};
};
