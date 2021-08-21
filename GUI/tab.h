#pragma once
#include "window.h"

class c_tab : public c_window {
private:
	int index;
	float animation;
	void* texture;

	std::string hint;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_tab(std::string name, int index, void* tex, c_window* child) {
		this->child = child;
		this->index = index;
		this->texture = tex;
		this->set_title(name);
		this->animation = 0.f;
		this->hint.clear();
	}
	void draw(Vector2D pos, Vector2D size);
	void render();
	void special_render();
};