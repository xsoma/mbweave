#pragma once
#include "element.h"

class c_separator : public c_element {
private:
	bool(*should_render)();
	Vector2D size;
	color_t color;
	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_separator(bool(*should_render)() = nullptr) {
		this->type = c_elementtype::separator;
		this->should_render = should_render;
		this->hint.clear();
	}
	bool hovered();
	bool update();
	void render();
	void special_render() {};
	int get_total_offset() {
		if (should_render)
			if (!should_render())
				return 0; 
		return 5;
	};
};