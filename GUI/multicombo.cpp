#include "multicombo.h"
#include "child.h"
#include "../Math.h"

void c_multicombo::update_format()
{
	format.clear();
	int t = 0;
	for (int i = 0; i < elements.size(); i++) {
		if (*(uint32_t*)value & 1 << i) {
			if (t > 0) format += ", ";
			format += elements[i];
			t++;
		}
	}
	if (!format.size())
		format = "...";
	ImGui::PushFont(fonts::menu_desc);
	format_size = ImGui::CalcTextSize(format.c_str()).x;
	ImGui::PopFont();
}

bool c_multicombo::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	auto c = (c_child*)child;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (!c->hovered())
		return false;
	pos.y -= 4;
	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);
	auto pos = Vector2D(right_border.x - 160, c->get_cursor_position().y);
	auto size = Vector2D(144, 24);
	return g_mouse.x >= pos.x && g_mouse.y >= pos.y
		&& g_mouse.x <= pos.x + size.x && g_mouse.y <= pos.y + size.y;
}

bool c_multicombo::update()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false;
	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);
	auto pos = Vector2D(right_border.x - 160, c->get_cursor_position().y);
	auto size = Vector2D(144, 24);
	pos.y -= 4;
	bool h = hovered();
	if (h) wnd->g_hovered_element = this;
	
	if (h && c->hovered() && wnd->is_click()) {
		if (open) {
			open = false;
			//wnd->g_active_element = nullptr;
		}
		else
		{
			open = true;
			wnd->g_active_element = this;
		}
		return true;
	}

	update_format();

	if (open) {
		if (open_animation < 1.f)
			open_animation += animation_speed;

		h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
			&& g_mouse.x <= pos.x + size.x && g_mouse.y <= pos.y + size.y * (elements.size() + 1);
		if (wnd->is_click() && !h) {
			open = false;
			//wnd->g_active_element = nullptr;
			wnd->reset_mouse();
			return true;
		}
		if (open_animation == 1.f) {
		//	pos.y += 5;
			for (size_t i = 0; i < elements.size(); i++) {
				pos.y += size.y;
				h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
					&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y;
				if (h) {
					if (animations[i] < 1.f) animations[i] += animation_speed;
					if (h) wnd->g_hovered_element = this;
				}
				else
				{
					if (animations[i] > 0.f) animations[i] -= animation_speed;
				}
				if (animations[i] > 1.f) animations[i] = 1.f;
				else if (animations[i] < 0.f) animations[i] = 0.f;

				if (wnd->is_click() && h) {
					*(uint32_t*)value ^= 1 << i;
					return false;
				}
			}
		}
		wnd->g_active_element = this;
		for (int i = 0; i < elements.size(); i++) {
			if (i == 0 || animations2[i - 1] >= 0.5f)
				animations2[i] = std::clamp(animations2[i] + animation_speed, 0.f, 1.f);
		}
	}
	else
	{
		//for (int i = 0; i < elements.size(); i++)
		//	this->animations2[i] = 0.f;

		//hovering_animation = 0.f;
		//
		//if (wnd->g_active_element == this)
		//	wnd->g_active_element = nullptr;
		//if (open_animation > 0.f)
		//	open_animation -= animation_speed;
		for (int i = 0; i < elements.size(); i++)
			this->animations2[i] = 0.f;

		if (wnd->g_active_element == this) {
			if (open_animation == 0.f)
				wnd->g_active_element = nullptr;
		}
		//wnd->g_active_element = nullptr;
		if (open_animation > 0.f)
			open_animation -= animation_speed;
	}
	// bullshit animations
	if (h || open) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = std::clamp(animation, 0.f, 1.f);
	
	if (open_animation > 1.f) open_animation = 1.f;
	else if (open_animation < 0.f) open_animation = 0.f;
	return false;
}
void c_multicombo::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	if (!c)
		return;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return;
	if (wnd->get_active_tab_index() != tab && wnd->get_tabs().size() > 0)
		return;
	auto pos = c->get_cursor_position();
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	auto size = Vector2D(g_size, 24);

	bool h = hovered();
	if (label.size() > 0) {
		g_Render->DrawString(pos.x, pos.y,
			style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
			render::none, fonts::menu_desc, label.c_str());
	}
	pos.y -= 4;
	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);

	g_Render->FilledRect(right_border.x - 40, pos.y, 24, 24,
		style.get_color(c_style::button_color)
		.transition(style.get_color(c_style::button_hovered_color), animation)
		.manage_alpha(alpha),
		2.f, open ? ImDrawCornerFlags_TopRight : ImDrawCornerFlags_Right);

	g_Render->FilledRect(right_border.x - 160, pos.y, 119, 24,
		style.get_color(c_style::button_color)
		.transition(style.get_color(c_style::button_hovered_color), animation)
		.manage_alpha(alpha),
		2.f, open ? ImDrawCornerFlags_TopLeft : ImDrawCornerFlags_Left);

	{
		const auto& arrow_pos = ImVec2(right_border.x - 28, pos.y + 11 + open_animation);

		static auto easeInOutExpo = [](float x) {
			return x == 0.f
				? 0.f
				: x == 1.f
				? 1.f
				: x < 0.5f ? pow(2.f, (double)(20.f * x - 10.f)) / 2.f
				: (2.f - pow(2.f, (double)(10.f - 20.f * x))) / 2.f;
		};

		constexpr auto base_angle1 = DEG2RAD(135);
		constexpr auto base_angle2 = DEG2RAD(45);
		constexpr auto base_angle3 = DEG2RAD(-90);
		const float& rot_angle = (1.f - easeInOutExpo(open_animation)) * PI;

		const ImVec2 points[] = {
			ImVec2(arrow_pos.x + cos(base_angle1 - rot_angle) * 5.f, arrow_pos.y + sin(base_angle1 - rot_angle) * 3.f),
			ImVec2(arrow_pos.x + cos(base_angle3 - rot_angle) * 5.f, arrow_pos.y + sin(base_angle3 - rot_angle) * 5.f),
			ImVec2(arrow_pos.x + cos(base_angle2 - rot_angle) * 5.f, arrow_pos.y + sin(base_angle2 - rot_angle) * 3.f)
		};
		g_Render->_drawList->AddPolyline(points, 3, color_t(200, 200, 200, alpha).u32(), false, 1.f);
	}

	ImGui::PushFont(fonts::menu_desc);
	float label_size = ImGui::CalcTextSize("Select items").x;
	ImGui::PopFont();

	g_Render->DrawString(right_border.x - 150.f, pos.y + 12,
		style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
		render::centered_y, fonts::menu_desc, "Select items");

	if (open_animation > 0.f) {
		auto new_alpha = std::clamp(alpha * open_animation, 0.f, 255.f);

		for (size_t i = 0; i < elements.size(); ++i) {

			ImGui::PushFont(fonts::menu_desc);
			float element_size = ImGui::CalcTextSize(elements[i].c_str()).x;
			ImGui::PopFont();

			g_Render->FilledRect(right_border.x - 160, pos.y + 25, 144, 24,
				style.get_color(c_style::button_color)
				.transition(style.get_color(c_style::button_hovered_color), animations[i])
				.manage_alpha(new_alpha),
				2.f, i == elements.size() - 1 ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_None);

			pos.y += size.y;

			g_Render->DrawString(right_border.x - 150.f, pos.y + size.y / 2,
				((*(uint32_t*)value) & 1 << i)
				? style.get_color(c_style::accented_color).manage_alpha((new_alpha - 50.f * (1.f - animations[i])))
				: style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animations[i]).manage_alpha(new_alpha),
				render::centered_y, fonts::menu_desc, elements[i].c_str());
		}
	}
}

int c_multicombo::get_total_offset() {
	if (should_render)
		if (!should_render())
			return 0;
	return 24;
};