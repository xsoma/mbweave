#include "tab_selector.h"
#include "window.h"
#include "child.h"
#include <thread>

std::string c_tab_selector::get_label() {
	return this->label;
}
void c_tab_selector::set_label(std::string label) {
	this->label = label;
}
bool c_tab_selector::update()
{
	if (should_render)
		if (!should_render())
			return false;

	c_child* c = (c_child*)child;
	if (!c) return false;

	bool inactive = active;
	if (inactive) inactive = !active();

	auto pos = c->get_cursor_position();
	auto size = Vector2D(576, 72);
	bool h = hovered();

	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;

	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false; 

	if (h) {
		wnd->g_hovered_element = this;
		if (c->hovered() && wnd->is_click()) {
			if (fn && !inactive) {
				std::thread(fn).detach();
				press_animation = 1.f;
				wnd->reset_mouse();
			}
			return true;
		}
	}
	if (fn && !inactive) {
		if (press_animation > 0.f)
			press_animation -= animation_speed;
		if (h) {
			if (animation < 1.f) animation += animation_speed;
		}
		else {
			if (animation > 0.f) animation -= animation_speed;
		}
	}
	animation = std::clamp(animation, 0.f, 1.f);
	press_animation = std::clamp(press_animation, 0.f, 1.f);
	return false;
}
bool c_tab_selector::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return false;
	auto pos = c->get_cursor_position();
	auto size = Vector2D(576, 72);
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y && !wnd->g_active_element;
}
void c_tab_selector::render() {
	if (should_render)
		if (!should_render())
			return;
	bool inactive = active;
	if (inactive) inactive = !active();
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();

	auto size = Vector2D(576, 72);

	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);

	auto clr = style.get_color(c_style::child_background)
		.transition(style.get_color(c_style::child_background_hovered), animation)
		.manage_alpha(alpha);

	g_Render->FilledRect(pos.x, pos.y, size.x, size.y, clr, 5.f);

	g_Render->DrawString(pos.x + 49, pos.y + 14, (fn && !inactive) ?
		color_t(230, 231, 238, alpha) :
		color_t(96, 96, 96, alpha),
		render::none, fonts::menu_desc, label.c_str());

	g_Render->DrawString(pos.x + 49, pos.y + 39, color_t(91, 91, 91, alpha),
		render::none, fonts::menu_main, desc.c_str());

	if (texture)
		g_Render->_drawList->AddImage(texture, ImVec2(pos.x + 16, pos.y + 14), ImVec2(pos.x + 32, pos.y + 32), ImVec2(0, 0), ImVec2(1, 1),
			color_t(255, 255, 255, alpha).u32());

}