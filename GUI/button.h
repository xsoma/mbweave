#pragma once
#include "element.h"

class c_button : public c_element {
private:
	std::string label; // название отображаемое в меню
	float animation, press_animation; // анимация при наведении, анимация при нажатии
	bool(*should_render)(); // функция которая отвечает за то, рендерится элемент или нет
	bool(*active)(); // функция которая отвечает за то, можно ли нажать на кнопку или нет
	Vector2D size;
	std::string hint;
public:
	c_button(std::string str, Vector2D size = Vector2D(-1.f, -1.f), void(*_call)() = nullptr, bool(*active)() = nullptr, bool(*should_render)() = nullptr) {
		this->label = str;
		this->fn = _call;
		this->type = button;
		this->size = size;
		this->press_animation = 0.f;
		this->animation = 0.f;
		this->active = active;
		this->should_render = should_render;
		this->hint.clear();
	}
	std::string get_hint() {
		return this->hint;
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
		auto size = this->size;
		if (size == Vector2D(-1.f, -1.f))
			size = Vector2D(g_size, element_height);
		if (should_render)
			if (!should_render())
				return 0;
		return size.y + 3;
	};
};