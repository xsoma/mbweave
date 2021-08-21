#include "listbox.h"
#include "window.h"
#include "child.h"
#include "../Menu.h"

int get_absolute_index(int current_index, int selector_id, std::vector<c_listbox::c_selector> selectors) {
	if (selector_id == 0)
		return current_index;

	int total_size = 0;
	int sidx = 0;
	for (const auto& s : selectors) {
		total_size += s.elements.size();
		if (++sidx == selector_id)
			break;
	}
	return total_size + current_index;
}


bool c_listbox::update()
{
	if (should_render)
		if (!should_render())
			return false;

	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false;
	auto pos = c->get_cursor_position();
	if (label.size() > 0) pos.y += after_text_offset;
	auto size = Vector2D(width, height);
	if (this->autosize)
		size.y = total_elements_size;
	bool h = hovered();

	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	if (this->subtabs) {
		for (int j = 0; j < selectors.size(); ++j) {
			auto& selector = selectors[j];
			pos.y += after_text_offset;

			for (SSIZE_T i = 0; i < static_cast<SSIZE_T>(selector.elements.size()); ++i) {
				int current_idx = get_absolute_index(i, j, this->selectors);
				bool hvrd = g_mouse.x > pos.x && g_mouse.y >= pos.y
					&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + 32;
				if (hvrd) {
					if (selector.animations[i] < 1.f) selector.animations[i] += animation_speed;
				}
				else {
					if (selector.animations[i] > 0.f) selector.animations[i] -= animation_speed;
				}

	
				selector.animations[i] = std::clamp(selector.animations[i], 0.f, 1.f);
				if (hvrd && c->hovered() && wnd->is_click()) {
					if (*(unsigned int*)value != current_idx) {
						*(unsigned int*)value = current_idx;
						this->on_value_changed(current_idx);
					}
					wnd->reset_mouse();
					return true;
				}
				
				pos.y += 32;
			}

			pos.y += 32;
		}

	}
	else {
		should_draw_scroll = total_elements_size > size.y;
		if (h) {
			wnd->g_hovered_element = this;
			while (csgo->scroll_amount > 0) {
				scroll -= 20;
				csgo->scroll_amount--;
			}
			while (csgo->scroll_amount < 0) {
				scroll += 20;
				csgo->scroll_amount++;
			}
		}

		if (should_draw_scroll) {
			scroll = std::clamp(scroll, 0.f, std::clamp(total_elements_size - size.y, 0.f, FLT_MAX));
		}
		else
			scroll = 0.f;
		if (should_draw_scroll)
			pos.y -= scroll;
		size = Vector2D(width, 32);
		for (SSIZE_T i = 0; i < static_cast<SSIZE_T>(elements.size()); ++i) {

			bool hvrd = g_mouse.x > pos.x && g_mouse.y >= pos.y
				&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
			if (hvrd) {
				if (animations[i] < 1.f) animations[i] += animation_speed;
			}
			else {
				if (animations[i] > 0.f) animations[i] -= animation_speed;
			}
			if (hvrd && c->hovered() && wnd->is_click()) {
				if (*(unsigned int*)value != i) {
					*(unsigned int*)value = i;
					if (on_value_changed)
						this->on_value_changed(i);
				}
				wnd->reset_mouse();
				return true;
			}
			animations[i] = std::clamp(animations[i], 0.f, 1.f);
			pos.y += 32;

		}
	}

	if (h) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = std::clamp(animation, 0.f, 1.f);
	return false;
}
bool c_listbox::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto pos = c->get_cursor_position();
	if (label.size() > 0) pos.y += after_text_offset;
	auto size = Vector2D(width, height);
	if (this->autosize)
		size.y = total_elements_size;
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}
void c_listbox::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();
	auto size = Vector2D(width, height);
	if (this->autosize)
		size.y = total_elements_size;
	auto wnd = (c_window*)c->get_parent();
	auto c_size = c->get_size();
	auto c_pos = wnd->get_position() + c->get_position();
	if (!wnd)
		return;

	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;

	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);
	if (this->subtabs) {

		for (int j = 0; j < selectors.size(); ++j) {
			auto& selector = selectors[j];
			if (j > 0)
				g_Render->DrawLine(c_pos.x, pos.y - 12, c_pos.x + c_size.x, pos.y - 12, style.get_color(c_style::borders_color).manage_alpha(alpha));
			g_Render->DrawString(pos.x + 8, pos.y, color_t(93, 93, 93, alpha),
				render::none, fonts::menu_desc, selector.name.c_str());
			pos.y += after_text_offset;

			for (SSIZE_T i = 0; i < static_cast<SSIZE_T>(selector.elements.size()); ++i) {
				if (i == 0) {
					g_Render->FilledRect(pos.x, pos.y, size.x, 32,
						style.get_color(c_style::button_color)
						.transition(style.get_color(c_style::button_hovered_color), selector.animations[i])
						.manage_alpha(alpha)
						, 5.f, ImDrawCornerFlags_Top);
				}
				else if (i == selector.elements.size() - 1) {
					g_Render->FilledRect(pos.x, pos.y, size.x, 32,
						style.get_color(c_style::button_color)
						.transition(style.get_color(c_style::button_hovered_color), selector.animations[i])
						.manage_alpha(alpha), 5.f, ImDrawCornerFlags_Bot);
				}
				else
					g_Render->FilledRect(pos.x, pos.y, size.x, 32,
						style.get_color(c_style::button_color)
						.transition(style.get_color(c_style::button_hovered_color), selector.animations[i])
						.manage_alpha(alpha));

				const auto& main_color1337 = style.get_color(c_style::accented_color);
				bool active = get_absolute_index(i, j, this->selectors) == *(unsigned int*)value;
				
				if (active) {
					const auto& chld_clr =
						style.get_color(c_style::button_color)
						.transition(style.get_color(c_style::button_hovered_color), selector.animations[i]);
					ImVec4 main_clr = ImVec4(main_color1337[0] / 255.f, main_color1337[1] / 255.f, main_color1337[2] / 255.f, alpha / 255.f * 0.25f);
					g_Render->_drawList->AddRectFilledMultiColor(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + 32),
						ImDrawPaint().SetLinearGradient(
							ImVec2(pos.x - 35, pos.y),
							ImVec2(pos.x + 10, pos.y),
							main_clr, ImVec4(
								chld_clr.get_red() / 255.f,
								chld_clr.get_green() / 255.f,
								chld_clr.get_blue() / 255.f,
								(alpha) / 255.f)
						), 5.f);
					g_Render->FilledRect(pos.x, pos.y + 8, 2, 16, main_color1337.manage_alpha(alpha));
					g_Render->FilledRect(pos.x + 2, pos.y + 8, 1, 16, main_color1337.manage_alpha(alpha * 0.5f));
					g_Render->FilledRect(pos.x + 3, pos.y + 8, 1, 16, main_color1337.manage_alpha(alpha * 0.25f));
				}

				g_Render->DrawString(pos.x + 16, pos.y + 32 / 2,
					active ?
					main_color1337.manage_alpha(alpha - 50.f * selector.animations[i]) :
					color_t(93 + 25 * selector.animations[i], 93 + 25 * selector.animations[i], 93 + 25 * selector.animations[i], alpha), render::centered_y,

					fonts::menu_desc, selector.elements[i].c_str());
				pos.y += 32;
			}

			pos.y += 32;
		}

	}
	else {
		if (label.size() > 0) {
			g_Render->DrawString(pos.x + 8, pos.y, color_t(93, 93, 93, alpha),
				render::none, fonts::menu_desc, label.c_str());
			pos.y += after_text_offset;
		}
		g_Render->_drawList->PushClipRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), true);

		if (should_draw_scroll)
			pos.y -= scroll;
		for (SSIZE_T i = 0; i < static_cast<SSIZE_T>(elements.size()); ++i) {
			if (i == 0) {
				g_Render->FilledRect(pos.x, pos.y, size.x, 32,
					style.get_color(c_style::button_color)
					.transition(style.get_color(c_style::button_hovered_color), animations[i])
					.manage_alpha(alpha)
					, 5.f, ImDrawCornerFlags_Top);
			}
			else if (i == elements.size() - 1) {
				g_Render->FilledRect(pos.x, pos.y, size.x, 32,
					style.get_color(c_style::button_color)
					.transition(style.get_color(c_style::button_hovered_color), animations[i])
					.manage_alpha(alpha), 5.f, ImDrawCornerFlags_Bot);
			}
			else
				g_Render->FilledRect(pos.x, pos.y, size.x, 32,
					style.get_color(c_style::button_color)
					.transition(style.get_color(c_style::button_hovered_color), animations[i])
					.manage_alpha(alpha));

			const auto& main_color1337 = style.get_color(c_style::accented_color);

			const auto& chld_clr = 
				style.get_color(c_style::button_color)
				.transition(style.get_color(c_style::button_hovered_color), animations[i]);

			if (*(int*)value == i) {
				ImVec4 main_clr = ImVec4(main_color1337[0] / 255.f, main_color1337[1] / 255.f, main_color1337[2] / 255.f, alpha / 255.f * 0.25f);
				g_Render->_drawList->AddRectFilledMultiColor(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + 32),
					ImDrawPaint().SetLinearGradient(
						ImVec2(pos.x - 35, pos.y),
						ImVec2(pos.x + 10, pos.y),
						main_clr, ImVec4(
							chld_clr.get_red() / 255.f,
							chld_clr.get_green() / 255.f,
							chld_clr.get_blue() / 255.f,
							(alpha) / 255.f)
					), 5.f);
			}

			g_Render->DrawString(pos.x + 16, pos.y + 32 / 2,
				*(int*)value == i ?
				main_color1337.manage_alpha(alpha - 50.f * animations[i]) :
				color_t(93 + 25 * animations[i], 93 + 25 * animations[i], 93 + 25 * animations[i], alpha), render::centered_y,

				fonts::menu_desc, elements[i].c_str());

			pos.y += 32;
		}
		if (should_draw_scroll)
			pos.y += scroll;
		pos.y -= 32 * (elements.size());
		g_Render->_drawList->PopClipRect();
	}
}
