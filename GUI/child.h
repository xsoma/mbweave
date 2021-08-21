#pragma once
#include "window.h"

class c_child : public c_window {
private:
	struct {
		float value, animation;
		bool dragging;
	} scroll;
	struct {
		bool dragging;
	} resizer;
	bool visible;
	Vector2D active_element_position;
	float elements_padding_y = 6;
public:
	std::string get_hint() {
		return this->hint;
	}
	enum child_alignment {
		none,
		centered,
	};
	child_alignment alignment;
	c_element* get_parent();
	c_child(std::string label, int tab, c_window* wnd, child_alignment alignment = none) {
		this->set_title(label);
		this->child = wnd;
		this->tab = tab;
		this->type = c_elementtype::child;
		this->horizontal_offset = true;
		this->scroll.value = 0.f;
		this->scroll.animation = 0.f;
		this->scroll.dragging = false;
		this->alignment = alignment;
		this->visible = true;
		set_padding(Vector2D(0, 0));
		this->hint.clear();
	}
	
	void set_visible(bool value) {
		this->visible = value; 
	};
	void set_elements_padding_y(float padding) {
		elements_padding_y = padding;
	}
	bool should_draw_scroll;
	int total_elements_size;
	bool horizontal_offset;
	void lock_bounds();
	void unlock_bounds();
	int get_total_offset();
	int get_upper_offset();
	bool update();
	void update_total_size();
	void update_elements();
	void render_elements();
	void render();
	void special_render() {};
	bool hovered();
	void initialize_elements();
};