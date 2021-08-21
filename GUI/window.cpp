#include "tab.h"
#include "child.h"
#include "../menu.h"

float animation_speed = 0.f;

void c_window::set_title(std::string title) {
	this->title = title;
}
void c_window::set_position(Vector2D pos) {
	this->pos = pos;
}
void c_window::set_size(Vector2D size) {
	this->size = size;
}
std::string c_window::get_title() {
	return this->title;
}
std::vector<c_tab*> c_window::get_tabs() {
	return this->tabs;
}
Vector2D c_window::get_position() {
	return this->pos;
}
Vector2D c_window::get_size() {
	return this->size;
}
void c_window::add_element(c_element* e) {
	this->elements.push_back(e);
}
void c_window::add_tab(c_tab* t) {
	this->tabs.push_back(t);
}
void c_window::set_cursor_position(Vector2D cursor) {
	this->cursor = cursor;
}
Vector2D c_window::get_cursor_position() {
	return this->cursor;
}
void c_window::apply_element(int offset) {
	this->cursor.y += offset;
}
float c_window::get_transparency() {
	transparency = std::clamp(transparency, 0.f, 100.f);
	return transparency;
}
void c_window::set_transparency(float transparency) {
	this->transparency = std::clamp(transparency, 0.f, 100.f);
}
void c_window::increase_transparency(float add) {
	transparency += add;
	transparency = std::clamp(transparency, 0.f, 100.f);
}
void c_window::decrease_transparency(float subtract)
{
	transparency -= subtract;
	transparency = std::clamp(transparency, 0.f, 100.f);
}
bool c_window::is_click() {
	return is_pressed;
}
bool c_window::left_click() {
	return is_pressed_left;
}
bool c_window::reset_mouse() {
	is_pressed = false;
	holding = false;
	needs_reset = true;
	return true;
}
void c_window::set_active_tab_index(int tab) {
	this->active_tab_index = tab;
}
int c_window::get_active_tab_index() {
	return this->active_tab_index;
}
bool c_window::is_holding() {
	return holding;
}
void c_window::lock_bounds() {
	auto pos = this->get_position();
	auto size = this->get_size();
	auto m = Vector2D(pos.x + size.x, pos.y + size.y);
	g_Render->_drawList->PushClipRect(ImVec2(pos.x, pos.y), ImVec2(m.x, m.y), true);
}
void c_window::unlock_bounds() {
	g_Render->_drawList->PopClipRect();
}
bool c_window::hovered() {
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}
bool c_window::key_updated(int key)
{
	return keystates[key] && !oldstates[key];
}
bool c_window::update() {
	if (ImGui::GetIO().MouseDownDuration[0] >= 0.f && needs_reset)
		return false;
	else if (needs_reset)
		needs_reset = false;

	is_pressed = ImGui::GetIO().MouseDownDuration[0] == 0.f;
	holding = ImGui::GetIO().MouseDownDuration[0] > 0.f;

	is_pressed_left = ImGui::GetIO().MouseDownDuration[1] == 0.f;
	holding_left = ImGui::GetIO().MouseDownDuration[1] > 0.f;

	g_hovered_element = nullptr;
	if (on_click_back != nullptr) {
		if (g_mouse.x > pos.x && g_mouse.y > pos.y
			&& g_mouse.x < pos.x + 40 && g_mouse.y < pos.y + 40 && is_click() && !g_active_element) {
			this->on_click_back();
			reset_mouse();
		}
	}
	return false;
}
bool c_window::is_holding_menu()
{
	return holding_menu && is_holding();
}
c_element* c_window::get_parent() {
	return nullptr;
}
void c_window::render_tabs() {
	int t_size = tabs.size();
	Vector2D tab_size = Vector2D((size.x - 30.f) / t_size, 40.f);
	Vector2D pos_render = Vector2D(pos.x + 15.f, pos.y + 5.f);
	auto alpha = (int)(get_transparency() * 2.55f);

	for (auto& t : tabs) {
		t->draw(pos_render, tab_size);
		pos_render.x += tab_size.x;
	}
	pos_render.x -= tab_size.x * tabs.size();
}
void c_window::update_animation() {
	if (last_time_updated == -1.f)
		last_time_updated = csgo->get_absolute_time();
	animation_speed = fabsf(last_time_updated - csgo->get_absolute_time()) * 5.f;
	last_time_updated = csgo->get_absolute_time();
}
void c_window::update_keystates() {
	memcpy(oldstates, keystates, sizeof(bool) * 256);
	for (auto i = 0; i < 255; i++)
		keystates[i] = HIWORD(GetKeyState(i));
}

bool c_window::get_clicked()
{
	if (hovered()) {
		if (!holding) {
			drag.x = g_mouse.x - pos.x;
			drag.y = g_mouse.y - pos.y;
		}
		return true;
	}
	return false;
}

void c_window::render() {
	if (transparency <= 0.f)
		return;

	int a = std::clamp((int)(transparency * 2.55f - 50.f), 0, 255);

	const auto& windowbg_color = style.get_color(c_style::window_background);
	const auto& border_color = style.get_color(c_style::borders_color);

	g_Render->FilledRect(pos.x, pos.y, size.x, size.y, windowbg_color.manage_alpha((int)(transparency * 2.55f)), 5.f);
	g_Render->DrawLine(pos.x, pos.y + 40, pos.x + size.x, pos.y + 40, border_color.manage_alpha((int)(transparency * 2.55f)));
	g_Render->DrawLine(pos.x + 40, pos.y, pos.x + 40, pos.y + 40, border_color.manage_alpha((int)(transparency * 2.55f)));

	constexpr int base_x = 18;
	const color_t& back_button_color = on_click_back != nullptr ?
		color_t(200, 200, 200, (int)(transparency * 2.55f)) :
		color_t(88, 88, 88, (int)(transparency * 2.55f));
	const color_t& text_color = style.get_color(c_style::text_color);

	g_Render->DrawLine(pos.x + base_x, pos.y + 20, pos.x + base_x + 5, pos.y + 25, back_button_color);
	g_Render->DrawLine(pos.x + base_x, pos.y + 20, pos.x + base_x + 5, pos.y + 15, back_button_color);
	g_Render->DrawString(pos.x + 57, pos.y + 20, text_color.manage_alpha((int)(transparency * 2.55f)), render::centered_y, fonts::menu_main, title.c_str());


	csgo->username = "admin";

	if (transparency > 99.f)
		g_Render->DrawString(pos.x + 400, pos.y + 20, style.get_color(c_style::window_background).increase(2), render::centered_y, fonts::lby_indicator, csgo->username.c_str());
	update();

	cursor = pos + padding;

	Vector2D backup_cursor_pos = cursor;
	update_elements();
	cursor = backup_cursor_pos;
	render_elements();

	if (g_active_element) {
		if (g_active_element->type != c_elementtype::child)
			((c_child*)active_element_parent)->set_cursor_position(g_active_element_pos);
		g_active_element->render();
	}

	if (!holding)
		holding_menu = !g_hovered_element && get_clicked();

	if (holding_menu)
	{
		this->pos = Vector2D(g_mouse.x - drag.x, g_mouse.y - drag.y);
	}

	if (update_state_fn) {

		// remove all elements
		for (auto el : elements)
			delete el;

		elements.clear();

		// run callback
		update_state_fn();
		// remove callback
		update_state_fn = nullptr;
	}

	if (style.debug_mode) {
		bool check = false;
		for (auto e : elements) {
			if (e->type == c_elementtype::child)
				for (const auto& el : ((c_child*)e)->elements)
					if (el == g_hovered_element) {
						check = true;
						break;
					}
			if (check)
				break;
		}
		if (check) {
			if (g_hovered_element->type != c_elementtype::child) {
				const std::string hint = g_hovered_element->get_hint();
				if (!hint.empty()) {
					ImGui::PushFont(fonts::menu_main);
					auto hint_size = ImGui::CalcTextSize(hint.c_str()).x;
					ImGui::PopFont();

					g_Render->DrawString(pos.x + size.x - hint_size - 25, pos.y + 20, text_color.manage_alpha((int)(transparency * 2.55f)),
						render::centered_y, fonts::menu_main, hint.c_str());
				}
			}
		}
	}
}

void c_window::update_elements()
{
	for (auto e : elements) {
		if (e->tab != active_tab_index) {
			e->showing_animation = 1.f;
			if (e->type == c_elementtype::child)
				for (const auto& el : ((c_child*)e)->elements)
					el->showing_animation = 1.f;
			continue;
		}

		if (e->showing_animation > 0.f)
			e->showing_animation -= animation_speed * 0.5f;
		if (e->showing_animation < 0.f)
			e->showing_animation = 0.f;

		e->update();
	}
}

void c_window::render_elements()
{
	for (auto e : elements) {
		if (e->tab != active_tab_index)
			continue;
		e->render();
		if (e->type != c_elementtype::child) {
			if (e == g_active_element) {
				g_active_element_pos = get_cursor_position();
				active_element_parent = this;
			}
		}
	}
};

const std::vector<std::string> KeyStrings = {
	"no key", "m1", "m2", "c+b", "m3", "m4", "m5",
	"unk", "bkspc", "tab", "unk", "unk", "unk", "enter", "unk", "unk", "shift", "ctrl", "alt", "pause",
	"caps lock", "unk", "unk", "unk", "unk", "unk", "unk", "esc", "unk", "unk", "unk", "unk", "spacebar",
	"pg up", "pg down", "end", "home", "left", "up", "right", "down", "unk", "print", "unk", "print screen", "insert",
	"delete", "unk", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
	"y", "z", "l-win", "r-win", "unk", "unk", "unk", "num0", "num1", "num2", "num 3", "num4", "num5", "num6",
	"num7", "num8", "num9", "*", "+", "_", "-", ".", "/", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
	"f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "num lock", "scroll lock", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "lshift", "rshift", "lctrl", "rctrl", "lmenu", "rmenu", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "nxt", "prv", "stop", "play/pause", "unk", "unk",
	"unk", "unk", "unk", "unk", ";", "+", ",", "-", ".", "/?", "~", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "[{", "\\|", "}]", "'\"", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk","unk", "unk", "unk"
};