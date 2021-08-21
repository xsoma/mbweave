#include <sstream>
#include <math.h>
#include <algorithm>

#include "colorpicker.h"
#include "element.h"
#include "child.h"
#include "../colorpicker_bg.h"
#include "../alpha_slider_bg.h"


color_t hue_colors[7] = {
	color_t(255, 0, 0, 255),
	color_t(255, 255, 0, 255),
	color_t(0, 255, 0, 255),
	color_t(0, 255, 255, 255),
	color_t(0, 0, 255, 255),
	color_t(255, 0, 255, 255),
	color_t(255, 0, 0, 255)
};

void to_clipboard(const char* text)
{
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		char* clip_data = (char*)(GlobalAlloc(GMEM_FIXED, MAX_PATH));
		lstrcpyA(clip_data, text);
		SetClipboardData(CF_TEXT, (HANDLE)(clip_data));
		LCID* lcid = (DWORD*)(GlobalAlloc(GMEM_FIXED, sizeof(DWORD)));
		*lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT);
		SetClipboardData(CF_LOCALE, (HANDLE)(lcid));
		CloseClipboard();
	}
}

std::string from_clipboard() {
	std::string fromClipboard;//в эту переменную сохраним текст из буфера обмена
	if (OpenClipboard(0))//открываем буфер обмена
	{
		HANDLE hData = GetClipboardData(CF_TEXT);//извлекаем текст из буфера обмена
		char* chBuffer = (char*)GlobalLock(hData);//блокируем память
		fromClipboard = chBuffer;
		GlobalUnlock(hData);//разблокируем память
		CloseClipboard();//закрываем буфер обмена
	}
	return fromClipboard;
}

void c_colorpicker::render_sliderbg(Vector2D pos, float alpha)
{
	static LPDIRECT3DTEXTURE9 img;

	if (img == NULL)
		D3DXCreateTextureFromFileInMemory(g_Render->GetDevice(), &alpha_slider_bg, sizeof(alpha_slider_bg), &img);

	if (img)
		g_Render->_drawList->AddImage((void*)img, ImVec2(pos.x, pos.y), ImVec2(pos.x + 16, pos.y + 200), ImVec2(0, 0), ImVec2(1, 1),
			color_t(255, 255, 255, alpha).u32());
}

bool c_colorpicker::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	auto c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;

	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);

	auto size = Vector2D(16, 16);

	size.x += 8 * open_animation;
	size.y += 8 * open_animation;

	auto pos = Vector2D(right_border.x - 16 - size.x, c->get_cursor_position().y);
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}

bool c_colorpicker::update()
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
	bool h = hovered();
	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);

	auto size = Vector2D(16, 16);

	auto pos = Vector2D(right_border.x - 16 - size.x, c->get_cursor_position().y);
	
	// bullshit animations
	if (h) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = std::clamp(animation, 0.f, 1.f);
	
	if (h) {
		wnd->g_hovered_element = this;
		if (wnd->is_click()) {
			if (open) {
				//wnd->g_active_element = nullptr;
				open = false;
			}
			else if (wnd->g_active_element == nullptr) {
				open = true;
				wnd->g_active_element = this;
			}
			update_colors();
			return true;
		}
		else if (wnd->left_click() && !open) {
			if (wnd->g_active_element == nullptr && !open) {
				dialogue.open = true;
				wnd->g_active_element = this;
				update_colors();
				return true;
			}
			
		}
		
	}
	if (dialogue.open) {
		pos.y += size.y + 2;
		size = Vector2D(80, 20);
		h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
			&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y * dialogue.elements.size();
		if (wnd->is_click() && !h) {
			dialogue.open = false;
			wnd->g_active_element = nullptr;
			wnd->reset_mouse();
			update_colors();
			return true;
		}
		for (size_t i = 0; i < dialogue.elements.size(); i++) {
			h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
				&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y;
			
			if (h) {
				wnd->g_hovered_element = this;
				if (dialogue.animations[i] < 1.f) dialogue.animations[i] += animation_speed;
			}
			else
			{
				if (dialogue.animations[i] > 0.f) dialogue.animations[i] -= animation_speed;
			}
			if (dialogue.animations[i] > 1.f) dialogue.animations[i] = 1.f;
			else if (dialogue.animations[i] < 0.f) dialogue.animations[i] = 0.f;
			if (wnd->is_click() && h) {
				if (i == 0) { // copy
					to_clipboard(std::to_string((*(color_t*)value).u32()).c_str());
				}
				else if (i == 1) { // paste
					(*(color_t*)value).set_u32(atoll(from_clipboard().c_str()));
					//update_colors();
				}
				dialogue.open = false;
				wnd->g_active_element = nullptr;
				wnd->reset_mouse();
				update_colors();
				return true;
			}
			pos.y += 20;
		}
	}
	else if (open) {	
		pos.y += 30;
		size.x = 200.f;
		if (open_animation < 1.f)
			open_animation += animation_speed;
		size.x += 8 * open_animation;
		size.y += 8 * open_animation;

		const auto& bg_size = Vector2D(264, 248);
		bool full_hovered = g_mouse.x > pos.x - 9 && g_mouse.y > pos.y - 9
			&& g_mouse.x < pos.x + bg_size.x - 9 && g_mouse.y < pos.y + bg_size.y - 9;

		if (!full_hovered && wnd->is_click()) {
			//wnd->g_active_element = nullptr;
			open = false;
			wnd->reset_mouse();
			update_colors();
			return true;
		}

		const auto& slider_size = Vector2D(16, 200);

		const auto& hue_pos = Vector2D(pos.x + 207, pos.y);
		const auto& alpha_pos = Vector2D(pos.x + 232, pos.y);

		bool s1 = g_mouse.x > hue_pos.x && g_mouse.y > hue_pos.y
			&& g_mouse.x < hue_pos.x + slider_size.x && g_mouse.y < hue_pos.y + slider_size.y;

		bool s2 = g_mouse.x > alpha_pos.x && g_mouse.y > alpha_pos.y
			&& g_mouse.x < alpha_pos.x + slider_size.x && g_mouse.y < alpha_pos.y + slider_size.y;
	
		bool hvrd = g_mouse.x > pos.x && g_mouse.y > pos.y
			&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + 200;

		if (s1 || s2 || hvrd)
			wnd->g_hovered_element = this;

		size.x -= 8 * open_animation;
		size.y -= 8 * open_animation;

		if (s1) {
			if (animations[0] < 1.f) animations[0] += animation_speed;
		}
		else {
			if (animations[0] > 0.f) animations[0] -= animation_speed;
		}
		if (s2 && this->use_alpha_slider) {
			if (animations[1] < 1.f) animations[1] += animation_speed;
		}
		else {
			if (animations[1] > 0.f) animations[1] -= animation_speed;
		}
		if (animations[0] > 1.f) animations[0] = 1.f;
		else if (animations[0] < 0.f) animations[0] = 0.f;
		if (animations[1] > 1.f) animations[1] = 1.f;
		else if (animations[1] < 0.f) animations[1] = 0.f;
		
		if (wnd->is_holding()) {
			if (dragging_slider == 0) {
				if (s1) {
					dragging_slider = 1;
				}
				else if (s2) {
					dragging_slider = 2;
				}
				else if (hvrd) {
					dragging_slider = 3;
				}
			}
			
		}
		else
			dragging_slider = 0;

		if (dragging_slider > 0) {
			float newpos_x = g_mouse.x - (pos.x);
			float newpos_y = g_mouse.y - (pos.y);

			if (newpos_x < 0) newpos_x = 0;
			if (newpos_x > (size.x)) newpos_x = (size.x);

			if (newpos_y < 0) newpos_y = 0;
			if (newpos_y > 200) newpos_y = 200;

			if (dragging_slider == 1) {
				hue = std::clamp(newpos_y / 200.f, 0.f, 1.f);
				//hue = std::clamp(newpos_x / (size.x), 0.f, 1.f);
			}
			else if (dragging_slider == 2 && this->use_alpha_slider) {
				val_alpha = 1.f - std::clamp(newpos_y / 200.f, 0.f, 1.f);
			}
			else if (dragging_slider == 3) {
				saturation = newpos_x / size.x;
				brightness = 1.f - newpos_y / 200;
			}
			clamp_colors();
			*(color_t*)value = color_t::hsb(hue, saturation, brightness);
			if (this->use_alpha_slider)
				(*(color_t*)value).set_alpha(val_alpha * 255.f);
			else
				(*(color_t*)value).set_alpha(255.f);
		}
	}
	
	if (!open) {
		if (open_animation > 0.f)
			open_animation -= animation_speed;
		if (open_animation <= 0.f && !dialogue.open) {
			wnd->g_active_element = nullptr;
		}
	}
	open_animation = std::clamp(open_animation, 0.f, 1.f);
	return false;
}

float easeOutExpo(float x) {
	return x == 1.f ? 1.f : 1.f - pow(2.f, x * -10.f);
}

void c_colorpicker::render() {
	if (should_render)
		if (!should_render())
			return;
	auto c = (c_child*)child;
	if (!c) return;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;

	const auto& right_border = wnd->get_position() + c->get_position() + Vector2D(c->get_size().x, 0);

	auto size = Vector2D(16, 16);

	auto pos = Vector2D(right_border.x - 16 - size.x, c->get_cursor_position().y);

	auto alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);

	auto clr = *(color_t*)value;

	if (open_animation > 0.f) {
		size.x += 8 * easeOutExpo(open_animation);
		size.y += 8 * easeOutExpo(open_animation);

		alpha = (c->get_transparency() / 100.f) * open_animation * 255.f;

		const auto& bg_size = Vector2D(264 - 24 * (!this->use_alpha_slider), 248);
		g_Render->FilledRect(pos.x - 9, pos.y - 9, bg_size.x, bg_size.y, style.get_color(c_style::button_color).manage_alpha(alpha), 2.f);

		const color_t& picker = color_t::hsb(hue, 1.f, 1.f).manage_alpha(alpha);

		g_Render->FilledRectGradient(pos.x, pos.y + 30, 200, 200,
			color_t(255, 255, 255, alpha), picker,
			picker, color_t(255, 255, 255, alpha));

		g_Render->FilledRectGradient(pos.x, pos.y + 30, 200, 200,
			color_t(0, 0, 0, 0), color_t(0, 0, 0, 0),
			color_t(0, 0, 0, alpha), color_t(0, 0, 0, alpha));

		// color pointer
		g_Render->_drawList->AddCircleFilled(ImVec2(pos.x + 200 * saturation, pos.y + 30 + 200.f * (1.f - brightness)), 4,
			style.get_color(c_style::accented_color).manage_alpha(alpha).u32());
		g_Render->_drawList->AddCircleFilled(ImVec2(pos.x + 200 * saturation, pos.y + 30 + 200.f * (1.f - brightness)), 3,
			color_t(255, 255, 255, alpha).u32());

		// HEX value box
		{
			const auto& hex_value_size = Vector2D(64, 24);
			const auto& hex_value_pos = Vector2D(pos.x + 31, pos.y);
			g_Render->FilledRect(hex_value_pos.x, hex_value_pos.y, hex_value_size.x, hex_value_size.y,
				style.get_color(c_style::window_background).manage_alpha(alpha), 1.f);

			g_Render->Rect(hex_value_pos.x, hex_value_pos.y, hex_value_size.x, hex_value_size.y,
				style.get_color(c_style::borders_color)
				.increase(15, false)
				.manage_alpha(alpha), 2.f);

			std::stringstream ss;
			ss << std::hex << ((clr[0] << 16) | (clr[1] << 8) | clr[2]); // int decimal_value
			std::string res;
			ss >> res;
			const std::string res2 = "00000" + res;
			res = res2.substr(res2.size() - 6);
			std::transform(res.begin(), res.end(), res.begin(), ::toupper);
			g_Render->DrawString(hex_value_pos.x + hex_value_size.x / 2, hex_value_pos.y + hex_value_size.y / 2,
				style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
				render::centered_x | render::centered_y, fonts::menu_desc, "#%s", res.c_str());
		}

		// alpha value box
		if (this->use_alpha_slider)
		{
			const auto& alpha_value_size = Vector2D(50, 24);
			const auto& alpha_value_pos = Vector2D(pos.x + 100, pos.y);
			g_Render->FilledRect(alpha_value_pos.x, alpha_value_pos.y, alpha_value_size.x, alpha_value_size.y,
				style.get_color(c_style::window_background).manage_alpha(alpha), 1.f);

			g_Render->Rect(alpha_value_pos.x, alpha_value_pos.y, alpha_value_size.x, alpha_value_size.y,
				style.get_color(c_style::borders_color)
				.increase(15, false)
				.manage_alpha(alpha), 2.f);
			
			g_Render->DrawString(alpha_value_pos.x + alpha_value_size.x / 2, alpha_value_pos.y + alpha_value_size.y / 2,
				style.get_color(c_style::text_color).transition(style.get_color(c_style::text_color_hovered), animation).manage_alpha(alpha),
				render::centered_x | render::centered_y, fonts::menu_desc, "%i%%", int(val_alpha * 100.f));
		}

		// hue slider
		{
			const auto& hue_size = Vector2D(16, 200);
			const auto& hue_pos = Vector2D(pos.x + 207, pos.y + 30);
			constexpr int iterations = 25;
			color_t last_color = color_t(255, 0, 0, alpha);
			for (auto i = 0; i < iterations; i++) {
				const auto& current_color = color_t::hsb((float)i / iterations, 1.f, 1.f).manage_alpha(alpha);
				g_Render->FilledRectGradient(
					hue_pos.x, hue_pos.y + i * hue_size.y / iterations, 
					hue_size.x, hue_size.y / iterations, 
					last_color, last_color,
					current_color, current_color
					);

				last_color = current_color;
			}
			
			g_Render->FilledRect(hue_pos.x, hue_pos.y + hue_size.y * hue, 16, 4,
				style.get_color(c_style::text_color)
				.transition(style.get_color(c_style::text_color_hovered), animations[0])
				.manage_alpha(alpha));
			g_Render->Rect(hue_pos.x, hue_pos.y + hue_size.y * hue, 16, 4,
				style.get_color(c_style::accented_color)
				.increase(10 * animations[1])
				.manage_alpha(alpha));
		}

		// alpha slider
		if (this->use_alpha_slider)
		{
			const auto& alpha_size = Vector2D(16, 200);
			const auto& alpha_pos = Vector2D(pos.x + 232, pos.y + 30);

			render_sliderbg(alpha_pos, alpha);
			g_Render->FilledRectGradient(alpha_pos.x, alpha_pos.y, alpha_size.x, alpha_size.y,
				clr.manage_alpha(alpha), clr.manage_alpha(alpha),
				clr.manage_alpha(0), clr.manage_alpha(0));


			g_Render->FilledRect(alpha_pos.x, alpha_pos.y + alpha_size.y * (1.f - val_alpha), 16, 4,
				style.get_color(c_style::text_color)
				.transition(style.get_color(c_style::text_color_hovered), animations[0])
				.manage_alpha(alpha));
			g_Render->Rect(alpha_pos.x, alpha_pos.y + alpha_size.y * (1.f - val_alpha), 16, 4,
				style.get_color(c_style::accented_color)
				.increase(10 * animations[1])
				.manage_alpha(alpha));
		}
	}

	alpha = (int)(wnd->get_transparency() * 2.55f) * (1.f - showing_animation);

	g_Render->FilledRect(pos.x + 1, pos.y + 1, size.x - 2, size.y - 2, ((color_t*)value)->manage_alpha(min((*(color_t*)value)[3], alpha)), 1.f);

	g_Render->Rect(pos.x, pos.y, size.x, size.y, 
		style.get_color(c_style::borders_color)
		.increase(15 + 75 * animation, false)
		.manage_alpha(alpha), 1.f);

	if (dialogue.open) {
		pos.y += size.y + 2;
		size = Vector2D(80, 20);
		g_Render->Rect(pos.x - 1, pos.y - 1, size.x + 2, size.y * (dialogue.elements.size()) + 2, color_t(0, 0, 0, alpha));

		for (size_t i = 0; i < dialogue.elements.size(); i++) {
			g_Render->FilledRect(pos.x, pos.y, size.x, size.y,
				color_t(27 + 15.f * dialogue.animations[i], 31 + 15.f * dialogue.animations[i], 40 + 15.f * dialogue.animations[i], alpha));
			auto clr2 = color_t(200 + dialogue.animations[i] * 55.f, 200 + dialogue.animations[i] * 55.f, 200 + dialogue.animations[i] * 55.f, alpha);
			g_Render->DrawString(pos.x + 10, pos.y + size.y / 2, clr2, render::centered_y,
				fonts::menu_desc, dialogue.elements[i].c_str());
			pos.y += size.y;
		}
	}
}