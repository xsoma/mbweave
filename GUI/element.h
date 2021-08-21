#pragma once
#include "../DLL_MAIN.h"
#include "../render.h"

#define element_height 26
#define after_text_offset 23
#define g_size 220


extern float animation_speed;

extern color_t main_color;

extern Vector2D g_mouse;

enum c_elementtype : int {
	window,
	child,
	text,
	dynamic_text,
	button,
	tab_selector,
	checkbox,
	slider,
	combobox,
	icons_combobox,
	multi_combobox,
	colorpicker,
	keybind,
	input_text,
	listbox,
	bar,
	separator,
};

class c_element {
public:
	int order = 0;
	int tab;
	c_elementtype type;
	c_element* child;
	float showing_animation = 1.f;
	virtual int get_total_offset() = 0;
	virtual void render() = 0;
	virtual bool update() = 0;
	virtual bool hovered() = 0;
	virtual std::string get_hint() = 0;
};

class c_style {
public:
	bool debug_mode;
	enum e_style_id : uint16_t {
		accented_color,

		window_background,
		window_background_hovered,

		child_background,
		child_background_hovered,

		button_color,
		button_hovered_color,
		button_inactive_color,
		button_holding_color,

		borders_color,
		borders_color_hovered,

		text_color,
		text_color_hovered,
		text_color_active,
		text_color_inactive,

		e_style_id_last
	};
	void init();
	void set_color(e_style_id id, const color_t clr);
	color_t& get_color(e_style_id id);
private:
	color_t style[e_style_id_last];
};

extern c_style style;

extern const std::vector<std::string> KeyStrings;