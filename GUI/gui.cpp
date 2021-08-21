#include "gui.h"

c_style style;
c_gui g_gui;

void c_gui::add_window(c_window* window)
{
	windows.push_back(window);
}

void c_gui::render()
{

}

void c_style::init()
{
	style[accented_color] = color_t(255, 108, 0);

	style[window_background] = color_t(32, 32, 32);
	style[window_background_hovered] = color_t(32, 32, 32);

	style[child_background] = color_t(39, 39, 39);
	style[child_background_hovered] = color_t(44, 44, 44);

	style[button_color] = color_t(48, 48, 48);
	style[button_hovered_color] = color_t(55, 55, 55);
	style[button_inactive_color] = color_t(48, 48, 48);
	style[button_holding_color] = color_t(65, 65, 65);

	style[borders_color] = color_t(48, 48, 48);
	style[borders_color_hovered] = color_t(55, 55, 55);
	
	style[text_color] = color_t(200, 200, 200);
	style[text_color_hovered] = color_t(255, 255, 255);
	style[text_color_active] = color_t(255, 150, 0);
	style[text_color_inactive] = color_t(127, 127, 127);
}

void c_style::set_color(e_style_id id, const color_t clr)
{
	style[id] = clr;
}

color_t& c_style::get_color(e_style_id id)
{
	return style[id];
}
