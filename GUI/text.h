#pragma once
#include "element.h"

class c_text : public c_element {
private:
	std::string label;
	bool(*should_render)();
	Vector2D size;
	color_t color;
	int offset_x;

	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_text(std::string str, int offset_x = 0, bool(*should_render)() = nullptr, color_t clr = color_t(200, 200, 200)) {
		this->label = str;
		this->offset_x = offset_x;
		this->type = c_elementtype::text;
		this->should_render = should_render;
		this->color = clr;
		this->hint.clear();
	}
	std::string get_label();
	void set_label(std::string label);
	void change_pointer(void* ptr) {};
	bool hovered();
	bool update();
	void render();
	void special_render() {};
	int get_total_offset() {
		if (should_render)
			if (!should_render())
				return 0; 
		return 24;
	};
};