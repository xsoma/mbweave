#pragma once
#include "element.h"

class c_combo : public c_element {
private:
	std::string label;
	void* value;
	float open_animation;
	float animation;
	float* animations;
	float* animations2;
	bool open;
	std::vector<std::string> elements;
	Vector2D pos;
	bool(*should_render)();
	void(*on_change_value)(int new_index);
	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_combo(std::string label, void* val, std::vector<std::string> el, 
			bool(*should_render)() = nullptr, void(*on_change_value)(int new_index) = nullptr) {
		this->hint = js_variables::find_int((int*)val);
		this->label = label;
		this->value = val;
		this->elements = el;
		this->type = c_elementtype::combobox;
		this->animations = new float[elements.size()];
		this->animations2 = new float[elements.size()];

		for (int i = 0; i < elements.size(); i++)
			animations[i] = 0.f;

		for (int i = 0; i < elements.size(); i++)
			animations2[i] = 0.f;

		this->should_render = should_render;
		this->on_change_value = on_change_value;
		this->open = false;
		this->animation = 0.f;
		this->open_animation = 0.f;
	}
	void change_pointer(void* ptr) { value = ptr; };
	bool hovered(); 
	bool update();
	void render();
	int get_total_offset();
};