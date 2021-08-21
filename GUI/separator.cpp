#include "separator.h"
#include "window.h"
#include "child.h"

bool c_separator::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}
bool c_separator::update()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (c->get_active_tab_index() != this->tab && c->get_tabs().size() > 0)
		return false;
	this->size = Vector2D(c->get_size().x, 3);
	return false;
}
void c_separator::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	if (c->get_active_tab_index() != this->tab && c->get_tabs().size() > 0)
		return;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return;
	auto pos = c->get_cursor_position();
	auto c_size = c->get_size();
	auto c_pos = wnd->get_position() + c->get_position();
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	g_Render->DrawLine(c_pos.x, pos.y, c_pos.x + c_size.x, pos.y, hovered() ? 
		style.get_color(c_style::borders_color_hovered).manage_alpha(alpha) : 
		style.get_color(c_style::borders_color).manage_alpha(alpha));
}