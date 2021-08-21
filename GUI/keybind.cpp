#include "keybind.h"
#include "child.h"
#include "../Math.h"

bool c_keybind::update()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return true;

	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);
	auto pos = Vector2D(right_border.x - 144, c->get_cursor_position().y);
	auto size = Vector2D(128, 24);
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

	bind->type = std::clamp<unsigned short>(bind->type, 0, 4);
	bind->key = std::clamp<unsigned short>(bind->key, 0, 255);

	animation = std::clamp(animation, 0.f, 1.f);

	if (binder.open)
		binder.active = false;

	if (binder.active) {
		ImGui::GetIO().WantTextInput = true;

		for (auto i = 0; i < 256; i++) {
			if (wnd->key_updated(i)) {
				if (i == VK_ESCAPE) {
					bind->key = 0;
					binder.active = false;
					wnd->g_active_element = nullptr;
					return true;
				}
				else {
					bind->key = i;
					binder.active = false;
					wnd->g_active_element = nullptr;
					return true;
				}
			}
		}

		if (b_switch) {
			if (pulsating >= 0.f)
				pulsating += animation_speed / 4.f;
			if (pulsating >= 1.f)
				b_switch = false;
		}
		else {
			if (pulsating <= 1.f)
				pulsating -= animation_speed / 4.f;
			if (pulsating <= 0.f)
				b_switch = true;
		}

		return true;
	}
	else {
		b_switch = false;
		pulsating = 1.f;
	}
	
	bool h2 = g_mouse.x >= pos.x + 63 && g_mouse.y >= pos.y
		&& g_mouse.x <= pos.x + 128 && g_mouse.y <= pos.y + size.y;

	if (h2 && c->hovered() && wnd->is_click()) {
		binder.open = true;
		wnd->g_active_element = this;
		wnd->reset_mouse();
		return true;
	}
	if (binder.open) {
		if (binder.open_animation < 1.f)
			binder.open_animation += animation_speed;
		if (binder.open_animation >= 1.f) {
			size = Vector2D(63, 24);
			pos += Vector2D(64, 0);
			h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
				&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y;
			if (wnd->is_click() && h) {
				binder.open = false;
				wnd->reset_mouse();
				return true;
			}
			pos -= Vector2D(64, 0);
			size = Vector2D(128, 24);
			h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
				&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y * (binder.elements.size() + 1);

			if ((wnd->is_click() || wnd->left_click()) && !h) {
				binder.open = false;
				wnd->reset_mouse();
				return true;
			}

			for (size_t i = 0; i < binder.elements.size(); i++) {
				pos.y += size.y;
				h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
					&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y;
				if (h) {
					if (binder.animations[i] < 1.f) binder.animations[i] += animation_speed;
				}
				else
				{
					if (binder.animations[i] > 0.f) binder.animations[i] -= animation_speed;
				}
				if (binder.animations[i] > 1.f) binder.animations[i] = 1.f;
				else if (binder.animations[i] < 0.f) binder.animations[i] = 0.f;
				if (wnd->is_click() && h) {
					bind->type = i;
					binder.open = false;
					wnd->reset_mouse();
					return true;
				}
			}
		}
		wnd->g_active_element = this;
		for (int i = 0; i < binder.elements.size(); i++) {
			if (i == 0 || binder.animations2[i - 1] >= 0.5f)
				binder.animations2[i] = std::clamp(binder.animations2[i] + animation_speed, 0.f, 1.f);
		}
	}
	else {
		size = Vector2D(63, 24);
		h = g_mouse.x > pos.x && g_mouse.y > pos.y
			&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
		if (h && c->hovered() && wnd->is_click() && bind->type > 0) {
			binder.active = true;
			wnd->g_active_element = this;
			return true;
		}
		if (h) {
			if (binder.animation < 1.f) binder.animation += animation_speed;
		}
		else {
			if (binder.animation > 0.f) binder.animation -= animation_speed;
		}

		for (int i = 0; i < binder.elements.size(); i++)
			binder.animations2[i] = 0.f;

		if (binder.open_animation > 0.f)
			binder.open_animation -= animation_speed;

		if (binder.animation > 1.f) binder.animation = 1.f;
		else if (binder.animation < 0.f) binder.animation = 0.f;
		if (binder.open_animation == 0.f)
			wnd->g_active_element = nullptr;
	}
	if (binder.open_animation > 1.f) binder.open_animation = 1.f;
	else if (binder.open_animation < 0.f) binder.open_animation = 0.f;
	pulsating = std::clamp(pulsating, 0.f, 1.f);
	return true;
}

bool c_keybind::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;

	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab
		&& wnd->get_tabs().size() > 0) return false;

	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);
	auto pos = Vector2D(right_border.x - 144, c->get_cursor_position().y);
	auto size = Vector2D(128, 24);
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}

void c_keybind::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab
		&& wnd->get_tabs().size() > 0) return;

	auto size = Vector2D(128, 24);

	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	auto clr = color_t(35, 39, 50, alpha - 15.f * animation);
	auto clr2 = color_t(27, 31, 40, alpha - 15.f * animation);

	g_Render->DrawString(pos.x, pos.y,
		style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
		render::none, fonts::menu_desc, label.c_str());

	pos.y -= 4;
	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);

	ImGui::PushFont(fonts::menu_desc);
	float key_size = ImGui::CalcTextSize(KeyStrings[bind->key].c_str()).x;
	ImGui::PopFont();

	g_Render->FilledRect(right_border.x - 144, pos.y, size.x, size.y,
		style.get_color(c_style::button_color)
		.transition(style.get_color(c_style::button_hovered_color), animation)
		.manage_alpha(alpha),
		2.f, binder.open ? ImDrawCornerFlags_Top : ImDrawCornerFlags_All);

	g_Render->DrawLine(right_border.x - 80, pos.y, right_border.x - 80, pos.y + size.y, style.get_color(c_style::child_background).manage_alpha(alpha));

	g_Render->DrawString(right_border.x - 48, pos.y + size.y / 2,
		style.get_color(c_style::text_color)
		.transition(style.get_color(c_style::text_color_hovered), animation)
		.manage_alpha(alpha),
		render::centered_x | render::centered_y, fonts::menu_desc, binder.elements[bind->type].c_str());

	g_Render->DrawString(right_border.x - 112, pos.y + size.y / 2,
		binder.active ? style.get_color(c_style::accented_color).manage_alpha(alpha) :
		style.get_color(c_style::text_color)
		.transition(style.get_color(c_style::text_color_hovered), animation)
		.manage_alpha(alpha),
		render::centered_x | render::centered_y, fonts::menu_desc, KeyStrings[bind->key].c_str());

	if (binder.open_animation > 0.f) {
		auto new_alpha = std::clamp(alpha * binder.open_animation, 0.f, 255.f);

		for (size_t i = 0; i < binder.elements.size(); ++i) {

			ImGui::PushFont(fonts::menu_desc);
			float element_size = ImGui::CalcTextSize(binder.elements[i].c_str()).x;
			ImGui::PopFont();

			g_Render->FilledRect(right_border.x - 144, pos.y + 25, 128, 24,
				style.get_color(c_style::button_color)
				.transition(style.get_color(c_style::button_hovered_color), binder.animations[i])
				.manage_alpha(new_alpha),
				2.f, i == binder.elements.size() - 1 ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_None);

			pos.y += size.y;

			g_Render->DrawString(right_border.x - 134, pos.y + size.y / 2,
				bind->type == i
				? style.get_color(c_style::accented_color).manage_alpha((new_alpha - 50.f * (1.f - binder.animations[i])))
				: style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), binder.animations[i]).manage_alpha(new_alpha),
				render::centered_y, fonts::menu_desc, binder.elements[i].c_str());
		}
	}
	
}
