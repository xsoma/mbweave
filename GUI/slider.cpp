#include "slider.h"
#include "window.h"
#include "child.h"

bool c_slider::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	auto size = Vector2D(98, 15);
	auto pos = c->get_cursor_position();

	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);
	const auto& textbox_size = Vector2D(40, 24);
	const auto& textbox_pos = Vector2D(right_border.x - textbox_size.x - 16, pos.y);

	pos.x = textbox_pos.x - 14 - size.x;
	pos.y += textbox_size.y / 2 - 8;

	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}

bool c_slider::update()
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
	auto pos = c->get_cursor_position();
	auto alpha = (int)(c->get_transparency() * 2.55f);
	auto size = Vector2D(98, 15);

	bool h = hovered() && c->hovered();
	if (h) wnd->g_hovered_element = this;

	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);
	const auto& textbox_size = Vector2D(40, 24);
	const auto& textbox_pos = Vector2D(right_border.x - textbox_size.x - 16, pos.y);

	pos.x = textbox_pos.x - 14 - size.x;
	pos.y += textbox_size.y / 2 - 8;

	pos.y -= 4;

	if (wnd->is_holding() && h) {
		dragging = true;
	}
	else {
		if (!wnd->is_holding())
			dragging = false;
		else if (wnd->is_holding() && !h && dragging)
			dragging = true;
		else
			dragging = false;
	}
	if (h) {
		float newpos = g_mouse.x - pos.x;
		if (newpos < 0)
			newpos = 0;
		if (newpos > size.x)
			newpos = size.x;
		float ratio = newpos / size.x;
		pseudo_value = (int)(min + (max - min) * ratio);
	}
	if (dragging) {
		float newpos = g_mouse.x - pos.x;
		if (newpos < 0)
			newpos = 0;
		if (newpos > size.x)
			newpos = size.x;
		float ratio = newpos / size.x;
		*(int*)value = (int)(min + (max - min) * ratio);
		wnd->g_active_element = this;
		drop_animation = 1.f;
		anim_plus = 0.f;
		anim_minus = 0.f;
	} 
	else
	{
		if (drop_animation >= 0.f)
			drop_animation -= animation_speed * 0.75f;

		drop_animation = std::clamp(drop_animation, 0.f, 1.f);

		const auto& slider_size = Vector2D(15, 15);
		pos = Vector2D(pos.x - 18, pos.y + size.y / 2 - 8);

		bool h_plus = g_mouse.x > pos.x && g_mouse.y > pos.y
			&& g_mouse.x < pos.x + slider_size.x && g_mouse.y < pos.y + slider_size.y;

		pos.x -= (slider_size.x + 3);

		bool h_minus = g_mouse.x > pos.x && g_mouse.y > pos.y
			&& g_mouse.x < pos.x + slider_size.x && g_mouse.y < pos.y + slider_size.y;

		if (h_minus) {
			wnd->g_hovered_element = this;
			anim_minus += animation_speed;
			anim_plus -= animation_speed;
			if (wnd->is_click()) {
				(*(int*)value)--;
				wnd->reset_mouse();
			}
		}
		else if (h_plus) {
			wnd->g_hovered_element = this;
			anim_plus += animation_speed;
			anim_minus -= animation_speed;
			if (wnd->is_click()) {
				(*(int*)value)++;
				wnd->reset_mouse();
			}
		}
		else
		{
			anim_plus -= animation_speed;
			anim_minus -= animation_speed;
			wnd->g_active_element = nullptr;
		}
	}

	anim_plus = std::clamp(anim_plus, 0.f, 1.f);
	anim_minus = std::clamp(anim_minus, 0.f, 1.f);

	if (*(int*)value > max)
		*(int*)value = max;
	else if (*(int*)value < min)
		*(int*)value = min;

	// bullshit animations
	if (h || dragging) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = std::clamp(animation, 0.f, 1.f);
	return dragging;
}

void c_slider::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();
	
	auto size = Vector2D(90, 15);
	bool h = hovered() && c->hovered();
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);

	const auto& textbox_size = Vector2D(40, 24);

	if (!label.empty()) {
		g_Render->DrawString(pos.x, pos.y,
			style.get_color(c_style::text_color)
			.transition(style.get_color(c_style::text_color_hovered), animation)
			.manage_alpha(alpha),
			render::none, fonts::menu_desc, label.c_str());
	}
	pos.y -= 4;
	float ratio = ((float)((*(int*)value) - this->min) / (float)(this->max - this->min));
	ratio = std::clamp(ratio - (float)sin(showing_animation) * 0.1f, 0.f, 1.f);

	// draw slider current value
	const auto& textbox_pos = Vector2D(right_border.x - textbox_size.x - 16, pos.y);
	{
		g_Render->FilledRect(textbox_pos.x, textbox_pos.y, textbox_size.x, textbox_size.y, style.get_color(c_style::window_background).manage_alpha(alpha), 1.f);
		g_Render->Rect(textbox_pos.x, textbox_pos.y, textbox_size.x, textbox_size.y, style.get_color(c_style::borders_color).manage_alpha(alpha), 2.f);

		g_Render->DrawString(textbox_pos.x + textbox_size.x / 2, textbox_pos.y + textbox_size.y / 2,
			style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
			render::centered_x | render::centered_y, fonts::menu_desc, "%i", *(int*)value);
	}

	pos.x = textbox_pos.x - 14 - size.x;
	pos.y += textbox_size.y / 2 - 8;

	// slider base line
	{
		g_Render->Rect(pos.x - 1, pos.y + size.y / 2 - 4, size.x + 2, 8, style.get_color(c_style::borders_color).manage_alpha(alpha), 1.f);
		g_Render->FilledRect(pos.x, pos.y + size.y / 2 - 3, size.x, 6, style.get_color(c_style::window_background).manage_alpha(alpha), 1.f);
		g_Render->FilledRect(pos.x, pos.y + size.y / 2 - 3, ratio * size.x, 6, style.get_color(c_style::accented_color).manage_alpha(alpha), 1.f);
	}

	// draw exactly slider (rectangle)
	{
		const auto& slider_size = Vector2D(15, 15);
		const auto& slider_pos = Vector2D(pos.x + ratio * size.x - 7, pos.y + size.y / 2 - 8);

		g_Render->FilledRect(slider_pos.x, slider_pos.y, slider_size.x, slider_size.y,
			style.get_color(c_style::button_color)
			.transition(style.get_color(c_style::button_hovered_color), animation).increase(10)
			.manage_alpha(alpha), 2.f);

		for (int i = 1; i <= 3; ++i) {
			g_Render->DrawLine(slider_pos.x + 4 * i - 1, slider_pos.y + 3, slider_pos.x + 4 * i - 1, slider_pos.y + 11,
				style.get_color(c_style::text_color_inactive)
				.transition(style.get_color(c_style::text_color), animation).manage_alpha(alpha));
		}
	}

	// +
	{
		const auto& slider_size = Vector2D(15, 15);
		const auto& slider_pos = Vector2D(pos.x - 25, pos.y + size.y / 2 - 8);

		g_Render->FilledRect(slider_pos.x, slider_pos.y, slider_size.x, slider_size.y,
			style.get_color(c_style::button_color)
			.transition(style.get_color(c_style::button_hovered_color), anim_plus).increase(10)
			.manage_alpha(alpha), 2.f);

		// top to bottom
		g_Render->DrawLine(slider_pos.x + slider_size.x / 2, slider_pos.y + 4,
			slider_pos.x + slider_size.x / 2, slider_pos.y + slider_size.y - 4,
			style.get_color(c_style::text_color_inactive)
			.transition(style.get_color(c_style::text_color), anim_plus)
			.manage_alpha(alpha));

		// left to right
		g_Render->DrawLine(slider_pos.x + 4, slider_pos.y + slider_size.y / 2,
			slider_pos.x + slider_size.x - 4, slider_pos.y + slider_size.y / 2,
			style.get_color(c_style::text_color_inactive)
			.transition(style.get_color(c_style::text_color), anim_plus)
			.manage_alpha(alpha));
	}

	// -
	{
		const auto& slider_size = Vector2D(15, 15);
		const auto& slider_pos = Vector2D(pos.x - 25 - slider_size.x - 3, pos.y + size.y / 2 - 8);

		g_Render->FilledRect(slider_pos.x, slider_pos.y, slider_size.x, slider_size.y,
			style.get_color(c_style::button_color)
			.transition(style.get_color(c_style::button_hovered_color), anim_minus).increase(10)
			.manage_alpha(alpha), 2.f);

		// left to right
		g_Render->DrawLine(slider_pos.x + 4, slider_pos.y + slider_size.y / 2,
			slider_pos.x + slider_size.x - 4, slider_pos.y + slider_size.y / 2,
			style.get_color(c_style::text_color_inactive)
			.transition(style.get_color(c_style::text_color), anim_minus)
			.manage_alpha(alpha));
	}
}