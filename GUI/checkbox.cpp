#include "checkbox.h"
#include "window.h"
#include "child.h"

bool c_checkbox::update()
{
	if (should_render)
		if (!should_render())
			return false;

	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return true;
	auto pos = c->get_cursor_position();
	bool h = hovered();

	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	if (h) wnd->g_hovered_element = this;

	// bullshit animations
	if (h) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = std::clamp(animation, 0.f, 1.f);

	if (h && c->hovered() && wnd->is_click()) {
		*(bool*)value = !(*(bool*)value);
		return true;
	}

	if (*(bool*)value == true) {
		if (press_animation < 1.f)
			press_animation += animation_speed;
	}
	else {
		if (press_animation > 0.f)
			press_animation -= animation_speed;
	}
	
	press_animation = std::clamp(press_animation, 0.f, 1.f);
	return true;
}

bool c_checkbox::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto pos = c->get_cursor_position();
	ImGui::PushFont(fonts::menu_desc);
	auto size = ImGui::CalcTextSize(label.c_str());
	ImGui::PopFont();
	return g_mouse.x > pos.x - 30 && g_mouse.y > pos.y - 2
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y + 2;
}

void c_checkbox::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();

	auto wnd = (c_window*)c->get_parent();
	auto size = ImGui::CalcTextSize(label.c_str());
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab 
		&& wnd->get_tabs().size() > 0) return;
	auto alpha = (int)(c->get_transparency() * 2.55f) * (1.f - showing_animation);
	g_Render->DrawString(pos.x, pos.y, 
		style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
		render::none, fonts::menu_desc, label.c_str());

	pos -= Vector2D(26, 0);

	g_Render->FilledRect(pos.x - press_animation, pos.y + 1 - press_animation,
		14 + press_animation * 2.f, 14 + press_animation * 2.f,
		style.get_color(c_style::window_background)
		.transition(style.get_color(c_style::button_hovered_color), animation)
		.transition(style.get_color(c_style::accented_color), press_animation)
		.manage_alpha(alpha), 2.f * (press_animation));

	{
		const ImVec2 points[3] = { 
		ImVec2(pos.x + 3, pos.y + 7),
		ImVec2(pos.x + 6, pos.y + 10),
		ImVec2(pos.x + 11, pos.y + 5),
		};
		g_Render->_drawList->Flags &= ~ImDrawListFlags_AntiAliasedLines;
		g_Render->_drawList->AddPolyline(points, 3, color_t(255, 255, 255, 255.f * press_animation * c->get_transparency() / 100.f).u32(), false, 1.f);
	}


	g_Render->Rect(pos.x - 1, pos.y, 16, 16,
		style.get_color(c_style::button_color)
		.transition(style.get_color(c_style::button_hovered_color), animation)
		.manage_alpha(alpha * (1.f - press_animation)), 2.f);
}

