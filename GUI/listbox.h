#pragma once
#include "element.h"

class c_listbox : public c_element {
public:
	struct c_selector {
	public:
		void init() {
			this->animations = new float[elements.size()];
			for (int i = 0; i < elements.size(); ++i)
				animations[i] = 0.f;
		}
		std::string name;
		std::vector<std::string> elements;
		float* animations;
	};
private:
	std::string label;
	float animation, press_animation;
	float height;
	bool(*should_render)();
	void(*on_value_changed)(int);
	void* value;
	float scroll;
	float* animations;
	std::vector<std::string> elements;
	bool should_draw_scroll;
	float total_elements_size;
	bool autosize;
	std::vector<c_selector> selectors;
	bool subtabs = false;
	std::string hint;
	float width = 176.f;
public:
	std::string get_hint() {
		return this->hint;
	}
	c_listbox(std::string str, void* value, std::vector<std::string> elements, float height, bool autosize = true,
		bool(*should_render)() = nullptr, void(*on_value_changed)(int) = nullptr) {
		this->label = str;
		this->type = listbox;
		this->height = height;
		this->value = value;
		this->elements = elements;
		this->animation = 0.f;
		this->press_animation = 0.f;
		this->scroll = 0.f;
		this->animations = new float[elements.size()];
		for (SSIZE_T i = 0; i < static_cast<SSIZE_T>(elements.size()); ++i)
			animations[i] = 0.f;
		this->should_render = should_render;
		this->should_draw_scroll = false;
		this->total_elements_size = elements.size() * 32;
		this->autosize = autosize;
		this->on_value_changed = on_value_changed;
		this->hint.clear();
	}
	void set_width(float val) {
		width = val;
	}
	void make_subtabs() {
		this->subtabs = true;
	}
	void add_selector(c_selector selector) {
		selector.init();
		//if (selectors.empty())
		//	this->total_elements_size += selector.elements.size() * 32;
		//else
			this->total_elements_size += selector.elements.size() * 32 + 48;
		selectors.emplace_back(selector);
		
	}
	void change_pointer(void* ptr) { };
	bool update();
	bool hovered();
	void render();
	void special_render() {};
	int get_total_offset() {
		if (should_render)
			if (!should_render())
				return 0;
		if (this->autosize)
			return total_elements_size + 5 + (label.size() > 0) * after_text_offset;
		return height + 5 + (label.size() > 0) * after_text_offset;
	};
};
