#pragma once
#include "direct_render_shonax.h"
#include "input_state_shonax.h"
#include <functional>
#include <algorithm>



namespace ui
{
	//the offset between each item
	constexpr int ITEM_OFFSET = 5;

	class base_item : public std::enable_shared_from_this< base_item > {
	public:
		base_item() { }
		base_item(int x, int y, int w, int h, const char* name = nullptr) :
			m_x(x), m_y(y), m_width(w), m_height(h) {
			if (name) {
				strcpy(m_text, name);
			}
		}

		virtual void render() { };
		virtual bool is_hovered() { return false; }
		virtual bool is_form() const { return false; }

		virtual void reset() {
			m_y_offset = 0;
		}

		virtual int get_total_height() const {
			return m_height;
		}

		void set_y_offset(int offset) {
			m_y_offset = offset;
		}

		int get_y_offset() const {
			return m_y_offset;
		}

		auto add_item(std::shared_ptr< base_item > item) {
			item.get()->m_parent = shared_from_this();
			m_items.emplace(m_items.begin(), item);

			return item;
		}

		auto& get_items() { return m_items; }
		auto  get_parent() { return m_parent; }

		virtual int x() const { return m_x; }
		virtual int y() const { return m_y; }
		virtual int w() const { return m_width; }
		virtual int h() const { return m_height; }

		void set_x(int x) { m_x = x; }
		void set_y(int y) { m_y = y; }

		void set_width(int w) { m_width = w; }
		void set_height(int h) { m_height = h; }

		bool get_visible() const {
			if (m_cond && !m_cond()) {
				return false;
			}

			return m_visible;
		}

		void set_cond(std::function< bool() > func) {
			m_cond = func;
		}

		void set_visible(bool vis) { m_visible = vis; }
		void set_text(const char* text) {
			strcpy(m_text, text);
		}
		auto get_text() const {
			return m_text;
		}

		std::shared_ptr< base_item > find_item(const char* name) {
			if (!m_items.empty()) {
				for (auto& it : m_items) {
					if (it->get_text() && !strcmp(it->get_text(), name)) {
						return it;
					}

					auto it_find = it->find_item(name);
					if (it_find != it) return it_find;
				}
			}

			return shared_from_this();
		}

		auto get_top_parent() {
			for (auto parent = m_parent; ;
				parent = parent->get_parent()) {
				if (!parent->get_parent()) {
					return parent;
				}
			}

			return shared_from_this();
		}

		int get_relative_x() {
			int x = m_x;
			for (auto parent = get_parent(); !!parent;
				parent = parent->get_parent()) {
				x += parent->x();
			}

			return x;
		}

		int get_relative_y() {
			int y = m_y + get_y_offset();
			for (auto parent = get_parent(); !!parent;
				parent = parent->get_parent()) {
				y += parent->y() + parent->get_y_offset();
			}

			return y;
		}

		void set_disabled(bool disabled) {
			m_disabled = disabled;

			for (auto& it : m_items) {
				it->set_disabled(disabled);
			}
		}

		void set_disabled_callbacks(bool disabled) {
			auto top = get_top_parent();

			top->set_disabled(disabled);

			m_disabled = false;
		}

	protected:
		int m_x{ };
		int m_y{ };

		int m_width{ };
		int m_height{ };

		//current y position for rendering
		int m_y_offset{ };

		bool m_visible = true;
		bool m_disabled = false;
		char m_text[256]{ };

		std::shared_ptr< base_item > m_parent;
		std::vector< std::shared_ptr< base_item > > m_items;
		std::function< bool() > m_cond;
	};
}


namespace ui
{
	class c_menu : public base_item {
	public:
		c_menu(int start_x, int start_y, int width,
			int height, const char* name, const char* right_text = nullptr) :
			base_item(start_x, start_y, width, height, name),
			m_right_text(right_text) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);

			return mouse_x >= m_x && mouse_x <= m_x + w()
				&& mouse_y >= m_y && mouse_y <= m_y + 19;
		}

		virtual int y() const override {
			return m_y + 19;
		}

		inline void input() {
			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);
			bool mouse_clicked = g_input.is_key_pressed(KEYS_MOUSE1);
			//bool mouse_clicked = GetAsyncKeyState(VK_LBUTTON);

			if (is_hovered()) {
				m_mouse_held = true;
			}

			if (!mouse_clicked) {
				m_mouse_held = is_hovered();
			}

			if (m_mouse_held && !mouse_clicked) {
				m_drag_offset_y = mouse_y - m_y;
				m_drag_offset_x = mouse_x - m_x;
			}

			if (m_mouse_held && mouse_clicked) {
				m_x = mouse_x - m_drag_offset_x;
				m_y = mouse_y - m_drag_offset_y;
			}
		}

		virtual void render() override {
			constexpr auto top_height = 19;

			input();

			//draw a c00l shadow
			ui_draw_outlined_rect(m_x, m_y + 1, m_width + 1, m_height, color_t(0, 0, 0, 166));

			ui_draw_rect(m_x + 1, m_y + 1, m_width - 1, top_height - 2, ui_get_bg_col() * 1.2f);
			if (is_hovered()) {
				ui_draw_rect(m_x + 1, m_y + 1, m_width - 1, top_height - 2, color_t(61, 61, 61));
			}

			for (int i{ }; i < 8; ++i) {
				color_t col = ui_get_bg_col() * (0.72f + i * 0.04f);
				ui_draw_rect(m_x, m_y + i + top_height - 1, m_width, m_height - i * 2 - top_height + 1, col);
			}
			ui_draw_outlined_rect(m_x, m_y, m_width, m_height, ui_get_accent_col());

			if (m_right_text) {
				ui_draw_string(m_x + 5, m_y + 4, false, ui_get_text_col(), m_text);

				int width, height;
				ui_get_text_size(width, height, m_right_text);

				ui_draw_string(m_x + m_width - 5 - width, m_y + 4, false, ui_get_text_col(), m_right_text);
			}
			else {
				ui_draw_string(m_x + m_width / 2, m_y + 4, true, ui_get_text_col(), m_text);
			}
		}

	protected:
		int m_drag_offset_x{ };
		int m_drag_offset_y{ };
		bool m_mouse_held{ };
		const char* m_right_text{ };
	};
}





namespace ui
{
	class c_form : public base_item {
	public:
		c_form(int x, int y, int w, int h, const char* name, int max_h = 0, bool collapse = false, bool default_collapsed = false) :
			base_item(x, y, w, h, name), m_dynamic(!h), m_max_height(max_h), m_collapse(collapse), m_open(!default_collapsed) { };

		virtual int x() const override {
			return m_x + 10;
		}

		virtual int y() const override {
			return m_y + m_scroll_offset + 9;
		}

		virtual bool is_form() const override {
			return true;
		}

		virtual bool is_hovered() override {
			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);

			int x = get_relative_x();
			int y = get_relative_y();

			return mouse_x > x && mouse_x < x + w()
				&& mouse_y > y && mouse_y < y + h();
		}

		bool is_collapse_hovered() {
			if (!m_collapse) return false;

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);

			int x = get_relative_x() + m_width - 8;
			int y = get_relative_y() + 2;

			return mouse_x >= x && mouse_x <= x + 5
				&& mouse_y >= y && mouse_y <= y + 7;
		}

		bool is_collapsed() {
			return (m_collapse && !m_open);
		}

		inline void update_size() {
			if (!m_dynamic) return;

			int total_height{ ITEM_OFFSET * 2 };
			for (auto& it : m_items) {
				if (it->get_visible()) {
					auto item_height = it->get_total_height() + ITEM_OFFSET;
					if (m_max_height && total_height + item_height > m_max_height) {
						total_height = m_max_height;
						break;
					}
					total_height += it->get_total_height() + ITEM_OFFSET;
				}
			}

			m_height = total_height;
		}

		virtual int get_total_height() const override {
			if (!m_open && m_collapse) return 18;
			return m_height + 5;
		}

		int get_total_item_height() {
			int total_height{ ITEM_OFFSET * 2 };
			for (auto& it : m_items) {
				if (it->get_visible()) {
					auto item_height = it->get_total_height() + ITEM_OFFSET;
					total_height += it->get_total_height() + ITEM_OFFSET;
				}
			}

			return total_height;
		}

		void input() {
			if (m_collapse) {
				bool hovered = is_collapse_hovered();
				bool clicked = g_input.is_key_pressed(KEYS_MOUSE1);
				if (hovered && clicked) {
					if (!m_was_pressed) {
						m_open ^= 1;
					}
					m_was_pressed = true;
				}
				else {
					m_was_pressed = false;
				}
			}

			if (m_max_height && get_total_item_height() > m_max_height) {
				if (!m_disabled && is_hovered()) {
					auto scroll_state = g_input.get_scroll_state();
					if (!!scroll_state && m_was_hovered) {
						scroll_state > 0 ? m_scroll_offset += 13 : m_scroll_offset -= 13;
					}
				}
				m_scroll_offset = std::clamp(m_scroll_offset, -(get_total_item_height() - m_height + 3), 0);
			}
			else {
				m_scroll_offset = 0;
			}
		}

		virtual void render() override {
			update_size();
			input();
			int x = get_relative_x();
			int y = get_relative_y();

			int text_w, text_h;
			ui_get_text_size(text_w, text_h, m_text);

			if (m_collapse) {
				if (!m_open && m_collapse) {
					ui_draw_outlined_rect(x, y, m_width, 14, ui_get_accent_col());
					ui_draw_line(x + 3, y, x + text_w + 1, y, ui_get_bg_col());
					ui_draw_string(x + 3, y - 7, false, ui_get_text_col(), m_text);

					int button_x = x + m_width - 6;
					int button_y = y + 2;

					ui_draw_string(button_x, button_y, true,
						is_collapse_hovered() ? ui_get_accent_col() : ui_get_text_col(), "+");
					return;
				}
			}

			for (int i{ }; i < 8; ++i) {
				color_t col = ui_get_bg_col() * (0.72f + i * 0.04f);
				ui_draw_rect(x, y + i, m_width, m_height - i * 2, col);
			}

			ui_draw_outlined_rect(x, y, m_width, m_height, ui_get_accent_col());

			if (m_max_height && get_total_item_height() > m_height) {
				const size_t height = get_total_height() - 20;
				const float delta = (float)(get_total_item_height() - height + 1);
				const float slider_step = ((float)(height) / delta);
				const float slider_height = slider_step * 13.f;

				size_t slider_pos = static_cast<size_t>(slider_step * m_scroll_offset);
				ui_draw_rect(x + m_width - 7, y + 8, 4, height, ui_get_disabled_col());
				ui_draw_rect(x + m_width - 7, y - slider_pos + 8, 4, (int)slider_height + 1, ui_get_accent_col());
			}

			if (is_hovered() != m_was_hovered) {
				bool backup = m_disabled;
				if (!backup) {
					set_disabled(!is_hovered());
				}
				m_disabled = backup;
			}

			ui_draw_line(x + 3, y, x + text_w + 1, y, ui_get_bg_col());
			ui_draw_string(x + 3, y - 7, false, ui_get_text_col(), m_text);
			if (m_collapse) {
				int button_x = x + m_width - 6;
				int button_y = y + 2;

				ui_draw_string(button_x, button_y, true,
					is_collapse_hovered() ? ui_get_accent_col() : ui_get_text_col(), "-");
			}

			m_was_hovered = is_hovered();
		}

	protected:
		bool m_dynamic{ };
		bool m_was_hovered{ };
		int m_max_height{ };
		int m_scroll_offset{ };
		bool m_was_pressed{ };
		bool m_collapse{ };
		bool m_open{ true };
	};
}




namespace ui
{
	//recursively render all items
	static void render_item(base_item* item, int offset = 0, bool allow_rect = true) {
		if (!item->get_visible()) return;

		item->reset();
		item->render();
		bool reset = false;
		RECT original;

		if (allow_rect && item->is_form()) {
			auto device = g_d3d.get_device();
			device->GetScissorRect(&original);

			auto x = item->get_relative_x();
			auto y = item->get_relative_y();

			RECT new_rect{
				x,
				y + 4,
				x + item->w(),
				y + item->get_total_height() - 7
			};

			device->SetScissorRect(&new_rect);
			reset = true;
		}

		bool draw = true;
		if (item->is_form()) {
			auto form_cast = static_cast<c_form*>(item);
			draw = !form_cast->is_collapsed();
		}

		if (item->get_items().size() && draw) {
			//madr0fl
			int* height_offset = (int*)_alloca(sizeof(int) * item->get_items().size());
			int cur_offset = 0;
			for (int i = (int)item->get_items().size() - 1; i >= 0; --i) {
				auto& cur_item = item->get_items()[i];
				height_offset[i] = cur_offset;
				cur_offset += cur_item->get_visible() ? cur_item->get_total_height() + ITEM_OFFSET : 0;
			}

			int i{ };
			for (auto& it : item->get_items()) {
				item->set_y_offset(height_offset[i]);
				render_item(it.get(), height_offset[i], !reset && allow_rect);
				++i;
			}
		}

		if (reset) {
			auto device = g_d3d.get_device();
			device->SetScissorRect(&original);
		}
	}
}



namespace ui
{
	class c_checkbox : public base_item {
	public:
		c_checkbox(int x, int y, const char* txt, bool* setting) :
			base_item(x, y, 16, 16, txt), m_setting(setting) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);

			int rel_x = get_relative_x();
			int rel_y = get_relative_y();

			return mouse_x >= rel_x && mouse_x <= rel_x + m_width
				&& mouse_y >= rel_y && mouse_y <= rel_y + m_height;
		}

		inline void render_checkbox(const int& x, const int& y) {
			color_t col = ui_get_bg_col();
			if (is_hovered()) {
				col = *m_setting ? ui_get_accent_col() * 0.8f : ui_get_bg_col() * 1.3f;
			}
			else if (*m_setting) {
				col = ui_get_accent_col();
			}

			ui_draw_rect(x, y, m_width, m_height, ui_get_disabled_col());
			ui_draw_rect(x + 1, y + 1, m_width - 2, m_height - 2, col);

			//ui_draw_outlined_rect( x, y, m_width, m_height, ui_get_accent_col( ) );
		}

		inline void input() {
			bool mouse_presesed = g_input.is_key_pressed(KEYS_MOUSE1);

			if (is_hovered() && mouse_presesed) {
				if (!m_mouse_held) {
					*m_setting = !*m_setting;
				}
				m_mouse_held = true;
			}
			else {
				m_mouse_held = false;
			}
		}

		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();

			render_checkbox(x, y);
			input();


			ui_draw_string(x + m_width + 6, y + 2, false, ui_get_text_col(), m_text);
		}

	protected:
		bool* m_setting;
		bool m_mouse_held{ };
	};
}





namespace ui
{
	namespace {
		constexpr int BUTTON_WIDTH = 120;
		constexpr int BUTTON_HEIGHT = 80;

		constexpr int SUBTAB_HEIGHT = 25;
	}

	class c_tab_sheet : public base_item {
	public:
		c_tab_sheet(const char* tab_name) :
			base_item(0, 0, 0, 0, tab_name),
			m_fade_progress(0) { };

		c_tab_sheet(const char* tab_name, d3d::c_sprite* sprite) :
			base_item(0, 0, 0, 0, tab_name), m_sprite(sprite),
			m_fade_progress(0.f) { };

		virtual void render() override { };
		virtual bool is_hovered() override {
			return false;
		};

	public:
		float m_fade_progress{ };
		d3d::c_sprite* m_sprite{ };
	};


	class c_tab_manager : public base_item {
	public:
		c_tab_manager() : base_item(BUTTON_WIDTH + 11, 0, 0, 0,
			("TAB_MANAGER")) { };

		virtual bool is_hovered() override {
			return false;
		}

		inline void scale_button_fade(c_tab_sheet* item, bool hovered) {
			if (hovered) {
				constexpr float frequency = 1.f / 0.3f;
				const float step = ui_get_frametime() * frequency;

				item->m_fade_progress = std::clamp(item->m_fade_progress + step, 0.f, 0.8f);
			}
			else {
				item->m_fade_progress = 0.f;
			}
		}

		auto get_selected_tab() {
			return m_selected_tab;
		}

		void draw_tab_button(decltype(m_parent)& button, int start, bool hovered) {
			auto item = (c_tab_sheet*)button.get();
			auto text = item->get_text();
			auto parent_x = m_parent->x() + 5;
			auto parent_y = m_parent->y() + 5;
			int item_height = BUTTON_HEIGHT;

			scale_button_fade(item, hovered);

			ui_draw_rect(parent_x, parent_y + start, BUTTON_WIDTH + 1,
				item_height + 1, ui_get_bg_col());

			ui_draw_line(parent_x + BUTTON_WIDTH - 8, parent_y + start - 1,
				parent_x + BUTTON_WIDTH - 8, parent_y + start + BUTTON_HEIGHT + 2,
				ui_get_disabled_col());

			if (hovered) {
				ui_draw_line(parent_x + BUTTON_WIDTH - 8, parent_y + start - 1,
					parent_x + BUTTON_WIDTH - 8, parent_y + start + BUTTON_HEIGHT + 2,
					ui_get_accent_col() * (item->m_fade_progress + 0.2f) * 0.8f);
			}

			if (item->m_sprite) {
				auto sprite_color = hovered ? ui_get_text_col() * ((item->m_fade_progress + 0.3f) * 0.7f) : ui_get_disabled_col();
				item->m_sprite->draw(parent_x + (BUTTON_WIDTH - 8) / 2 - 2,
					parent_y + BUTTON_HEIGHT / 2 + start, sprite_color);
			}
			else {
				
			}
		}

		inline bool is_button_hovered(int start) {
			auto item_x = m_parent->x() + 5;
			auto item_y = m_parent->y() + 5 + start;
			int	item_height = BUTTON_HEIGHT;
			int mouse_x, mouse_y;

			ui_get_cursor_pos(mouse_x, mouse_y);

			return mouse_x >= item_x && mouse_x <= item_x + BUTTON_WIDTH - 8
				&& mouse_y >= item_y && mouse_y <= item_y + item_height;
		}

		virtual void render() override {
			if (!m_items.empty()) {
				int cur_y{ 10 };
				for (auto& it : m_items) {
					it->set_visible(false);
					bool hovered = is_button_hovered(cur_y);
					if (g_input.is_key_pressed(KEYS_MOUSE1) && hovered) {
						//fix items that disable input mess it up when changing tabs
						set_disabled_callbacks(false);
						m_selected_tab = it;
					}

					draw_tab_button(it, cur_y, hovered || it == m_selected_tab);
					cur_y += BUTTON_HEIGHT + 4;
				}
			}

			if (!m_selected_tab) {
				m_selected_tab = m_items.front();
			}

			m_selected_tab->set_visible(true);
		}

	protected:
		decltype(m_parent) m_selected_tab{ };
	};


	class c_subtab_manager : public base_item {
	public:
		c_subtab_manager() : base_item(0, 35, 0, 5,
			("SUBTAB_MANAGER")) { }

		virtual bool is_hovered() override { return false; }

		int get_button_width() {
			int width = get_top_parent()->w() - BUTTON_WIDTH - 26;

			if (!m_items.empty())
				return (int)std::ceilf(float(width) / float(m_items.size()));

			return width;
		}

		inline void scale_button_fade(c_tab_sheet* item, bool hovered) {
			if (hovered) {
				constexpr float frequency = 1.f / 0.3f;
				const float step = ui_get_frametime() * frequency;

				item->m_fade_progress = std::clamp(item->m_fade_progress + step, 0.f, 0.8f);
			}
			else {
				item->m_fade_progress = 0.f;
			}
		}

		auto get_selected_tab() {
			return m_selected_tab;
		}

		void render_button(decltype(m_parent)& button, int start, bool hovered) {
			auto item = (c_tab_sheet*)button.get();
			auto item_x = get_relative_x() + start - 2;
			auto item_y = get_relative_y() + 3 - m_y;
			auto width = get_button_width();

			scale_button_fade(item, hovered);
			color_t tab_clr = ui_get_disabled_col();

			//ui_draw_rect( item_x, item_y, get_button_width( ), SUBTAB_HEIGHT, ui_get_bg_col( ) );
			ui_draw_rect(item_x + 1, item_y + SUBTAB_HEIGHT - 2, width, 2, tab_clr);
			if (hovered) {
				color_t col = ui_get_accent_col();
				col.set_alpha(col.get_alpha() * item->m_fade_progress);
				//col.a() *= item->m_fade_progress;
				ui_draw_rect(item_x + 1, item_y + SUBTAB_HEIGHT - 2, width, 2, col);
			}

			ui_draw_string(item_x + width / 2, item_y + 4, true, ui_get_text_col(), item->get_text());
		}

		bool is_button_hovered(decltype(m_parent)& button, int start) {
			int item_x = get_relative_x() + start - 2;
			int item_y = get_relative_y() + 3 - m_y - 3;
			int item_w = get_button_width();
			int mouse_x, mouse_y;

			ui_get_cursor_pos(mouse_x, mouse_y);

			return mouse_x >= item_x && mouse_x <= item_x + item_w &&
				mouse_y >= item_y && mouse_y <= item_y + SUBTAB_HEIGHT;
		}

		virtual void render() override {
			if (!m_items.empty()) {
				int start = 2;
				int width = get_button_width();

				for (auto& it : m_items) {
					it->set_visible(false);
					bool hovered = is_button_hovered(it, start);
					if (g_input.is_key_pressed(KEYS_MOUSE1) && hovered) {
						//fix items that disable input mess it up when changing tabs
						set_disabled_callbacks(false);
						m_selected_tab = it;
					}

					render_button(it, start, hovered || it == m_selected_tab);
					start += width;
				}

				if (!m_selected_tab) {
					m_selected_tab = m_items.front();
				}

				m_selected_tab->set_visible(true);
			}
		}

	protected:
		decltype(m_parent) m_selected_tab;
	};
}



namespace ui
{
	template < typename t >
	class c_slider : public base_item {
	public:
		c_slider(int x, int y, int w, t min, t max, const char* text, t* setting, t full, const char* suffix = 0) :
			base_item(x, y, w, 5, text), m_setting(setting), m_suffix(suffix),
			m_min(float(min)), m_max(float(max)), m_full(full), m_has_text(true) { };

		c_slider(int x, int y, int w, t min, t max, const char* text, t* setting, const char* suffix = 0) :
			base_item(x, y, w, 5, text), m_setting(setting), m_suffix(suffix),
			m_min(float(min)), m_max(float(max)), m_full(max), m_has_text(true) { };

		c_slider(int x, int y, int w, t min, t max, t* setting, const char* suffix = 0) :
			base_item(x, y, w, 5, nullptr), m_setting(setting), m_suffix(suffix),
			m_min(float(min)), m_max(float(max)), m_full(max), m_has_text(false) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int x = get_relative_x();
			int y = get_relative_y() + (m_has_text ? 11 : 2);

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);

			if (!m_has_text) {
				x += 4;
				return mouse_x >= x && mouse_x <= x + m_width - 8
					&& mouse_y >= y && mouse_y <= y + m_height + 2;
			}

			return mouse_x >= x - 1 && mouse_x <= x + m_width + 1
				&& mouse_y >= y && mouse_y <= y + m_height + 2;
		}

		void input() {
			bool mouse_clicked = g_input.is_key_pressed(KEYS_MOUSE1);
			bool hovered = is_hovered();
			float progress = 0.f;
			int x = get_relative_x();
			int y = get_relative_y() + 2;
			int mouse_x, mouse_y;

			*m_setting = (t)(std::clamp< float >(*m_setting, m_min, m_max));
			ui_get_cursor_pos(mouse_x, mouse_y);

			if (hovered && mouse_clicked) {

				float progress{ };

				if (m_has_text) {
					progress = std::clamp< float >(float(mouse_x - x) / (m_width - 3), 0.f, 1.0f);
				}
				else {
					progress = std::clamp< float >(float(mouse_x - x - 4) / (m_width - 10), 0.f, 1.0f);
				}

				*m_setting = progress == 1.0f ? m_full : (t)(((m_max - m_min) * progress) + m_min);
			}

			if (!m_has_text) {
				bool y_hover = mouse_y >= y && mouse_y <= y + m_height + 1;

				bool minus_hovered = mouse_x >= x - 1 && mouse_x <= x + 3 && y_hover;
				bool plus_hovered = mouse_x >= x + m_width - 2 && mouse_x <= x + m_width + 2 && y_hover;

				if (mouse_clicked) {
					if (!m_mouse_held) {
						if (!std::is_floating_point< t >::value) {
							if (minus_hovered) *m_setting -= (t)1;
							if (plus_hovered)  *m_setting += (t)1;
						}
						else if (m_max - m_min <= 2.0f) {
							if (minus_hovered) *m_setting -= (t)0.1f;
							if (plus_hovered)  *m_setting += (t)0.1f;
						}
					}

					m_mouse_held = true;
				}
				else {
					m_mouse_held = false;
				}
			}
		};

		void draw_slider(int x, int y) {
			float val = float(*m_setting);
			float progress = (val - m_min) / (m_max - m_min);

			ui_draw_rect(x, y, m_width, m_height, ui_get_disabled_col());

			ui_draw_rect(x, y, (int)((m_width - 3) * progress), m_height, ui_get_accent_col());
			ui_draw_rect(x + (int)((m_width - 3) * progress), y, 3, m_height,
				is_hovered() ? ui_get_text_col() : ui_get_accent_col() * 0.7f);
		}

		void draw_slider_small(int x, int y) {
			float val = float(*m_setting);
			float progress = (val - m_min) / (m_max - m_min);
			x -= 1; //i couldnt be fucked

			ui_draw_rect(x + 5, y, m_width - 8, m_height, ui_get_disabled_col());

			ui_draw_rect(x + 5, y, (int)((m_width - 10) * progress), m_height, ui_get_accent_col());
			ui_draw_rect(x + (int)((m_width - 10) * progress) + 3, y, 3, m_height,
				is_hovered() ? ui_get_text_col() : ui_get_accent_col() * 0.7f);

			ui_draw_string(x, y - 3, false, ui_get_text_col(), "-");
			ui_draw_string(x + m_width - 2, y - 3, false, ui_get_text_col(), "+");

			char val_str[12];
			if (m_suffix)
				sprintf_s(val_str, 12, t(0.1f) == t(0) ? "%d %s" : "%0.2f %s", *m_setting, m_suffix);
			else
				sprintf_s(val_str, 12, t(0.1f) == t(0) ? "%d" : "%0.2f", *m_setting);

			int text_h, text_w;
			ui_get_text_size(text_w, text_h, val_str);
			ui_draw_string(x + (int)((m_width - 8) * progress), y + 2, false, ui_get_text_col(), val_str);
		}

		virtual int get_total_height() const override {
			return m_height + (m_has_text ? 12 : 6);
		}

		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();
			int text_w, text_h;
			char val[12];

			input();

			//weird hacky fix for floating point vars
			if (m_suffix)
				sprintf_s(val, 12, t(0.1f) == t(0) ? "%d %s" : "%0.2f %s", *m_setting, m_suffix);
			else
				sprintf_s(val, 12, t(0.1f) == t(0) ? "%d" : "%0.2f", *m_setting);

			if (m_has_text) {
				ui_draw_string(x + 2, y, false, ui_get_text_col(), m_text);
				ui_get_text_size(text_w, text_h, val);
				ui_draw_string(x + m_width - text_w - 1, y, false, ui_get_text_col(), val);
				draw_slider(x, y + 12);
			}
			else {
				draw_slider_small(x, y + 2);
			}
		}


	protected:
		t* m_setting;
		t			m_full;
		float		m_min;
		float		m_max;
		bool		m_has_text = true;
		const char* m_suffix;
		bool		m_mouse_held = false;
	};
}

namespace ui
{
	namespace dropdowns
	{
		#define xors( s ) ( s )
		template < typename t = int >
		struct dropdown_item_t {
			const char* m_name;
			t			m_value;
		};


		static std::vector< dropdown_item_t< > > autoscope_aimbot_types = {
			{ xors("off"), 0 },
			{ xors("always"), 1 },
			{ xors("hitchance fail"), 2 },
		};






		static std::vector< dropdown_item_t< > > activation_types_aimbot = {
			{ xors("always"), 0 },
			{ xors("on key"), 1 },
		};

		static std::vector< dropdown_item_t< > > silent_aimbot_types = {
			{ xors("none"), 0 },
			{ xors("client-side"), 1 },
			{ xors("server-side"), 2 },
		};

		static std::vector< dropdown_item_t< > > hitboxes = {
			//{ xors("hitscan"), -1 },
			{ xors("head"), 1 },
			{ xors("neck"), 2 },
			{ xors("pelvis"), 3 },
			{ xors("chest"), 4 },
			{ xors("thorax"), 5 },
		};

		static std::vector< dropdown_item_t< > > auto_stop = {
	{ xors("none"), 0 },
	{ xors("hitchance"), 1 },
	{ xors("hitchance + lag"), 4 },
	{ xors("duck"), 5 },
	{ xors("slide"), 3 },
	{ xors("full"), 2 }
		};
		
		static std::vector< dropdown_item_t< > > antiaim_yaw = {
			{ xors("disabled"), 0 },
			{ xors("backwards"), 1 }

		};
		static std::vector< dropdown_item_t< > > desync_type = {
			{ xors("Manual"), 0 },
			{ xors("Peek real"), 1 },
			{ xors("Peek desync"), 2 },
			{ xors("Jitter"), 3}
		};
		static std::vector< dropdown_item_t< > > delay_type = {
			{ xors("none"), 0 },
			{ xors("time"), 2 },
			{ xors("velocity"), 3 },
		};

		static std::vector< dropdown_item_t< > > antiaim_pitch = {
			{ xors("disabled"), 0 },
			{ xors("down"), 1 },
			{ xors("ideal down"), 2 }
		};

		static std::vector< dropdown_item_t< > > antiaim_freestanding = {
			{ xors("disabled"), 0 },
			{ xors("static"), 1 },
			{ xors("narrow angle"), 2 },
			{ xors("wide angle"), 3 },
		};

		static std::vector< dropdown_item_t< > > antiaim_edge = {
			{ xors("off"), 0 },
			{ xors("peek"), 1 },
			{ xors("full"), 2 },
		};

		static std::vector< dropdown_item_t< > > antiaim_edge_type = {
			{ xors("static"), 0 },
			{ xors("narrow angle"), 1 },
			{ xors("wide angle"), 2 },
		};

		static std::vector< dropdown_item_t< > > fakelag_types = {
			{ xors("disabled"), 0 },
			{ xors("static"), 1 },
			{ xors("fluctuate"), 2 },
			{ xors("adaptive"), 3 },
		};

		static std::vector< dropdown_item_t< > > fakelag_fix_types = {
			{ xors("disabled"), 0 },
			{ xors("wait"), 1 },
			{ xors("predict"), 2 }
		};

		static std::vector< dropdown_item_t< > > activation_type_visuals = {
			{ xors("never"), 0 },
			{ xors("always"), 1 },
			{ xors("on hold"), 2 },
			{ xors("toggle"), 3 },
		};

		static std::vector< dropdown_item_t< > > configs = {
			{ xors("legit"), 0 },
			{ xors("closet"), 1 },
			{ xors("rage 1"), 2 },
			{ xors("rage 2"), 3 },
#if _DEBUG
			{ xors("test"), 4 },
#endif
		};

		static std::vector< dropdown_item_t< > > autostrafe_types = {
	{ xors("none"), 0 },
	{ xors("view"), 1 },
	{ xors("view + movement"), 2 }
		};


		static std::vector< dropdown_item_t< > > recorder_angles = {
			{ xors("none"), 0 },
			{ xors("shot"), 1 },
			{ xors("all"), 2 }
		};

		static std::vector< dropdown_item_t< > > baim_types = {
			{ xors("off"), 0 },
			{ xors("basic"), 1 },
			{ xors("smart"), 2 },
			{ xors("aggressive"), 3 },
			{ xors("force"), 4 },
		};

		static std::vector< dropdown_item_t< > > name_changers = {
			{ xors("off"), 0 },
			{ xors("dollar"), 1 },
			{ xors("moneybot.cc"), 2 },
		};

		static std::vector< dropdown_item_t< > > weapon_esp_types = {
			{ xors("none"), 0 },
			{ xors("icon"), 1 },
			{ xors("name"), 2 },
		};

		static std::vector< dropdown_item_t< > > fake_ping_activation = {
			{ xors("off"), 0 },
			{ xors("on hold"), 1 },
			{ xors("toggle"), 2 },
			{ xors("always"), 3 },
			{ xors("auto"), 4 }
		};

		static std::vector< dropdown_item_t< > > world_esp_type = {
			{ xors("off"), 0 },
			{ xors("name"), 1 },
			{ xors("glow"), 2 },
			{ xors("both"), 3 }
		};

		static std::vector< dropdown_item_t< > > multipoint_types = {
			{ xors("off"), 0 },
			{ xors("minimal"), 1 },
			{ xors("vitals"), 2 },
			{ xors("full"), 3 }
		};

		static std::vector< dropdown_item_t< > > edge_priority = {
			{ xors("freestanding"), 0 },
			{ xors("edge"), 1 }
		};

		static std::vector< dropdown_item_t< > > selection_type = {
			{ xors("fov"), 0 },
			{ xors("distance"), 1 },
			{ xors("simulation time"), 2 }
		};

		static std::vector< dropdown_item_t< > > autobuy_main_wep = {
			{ xors("none"), 0 },
			{ xors("autosniper"), 1 },
			{ xors("scout"), 2 },
			{ xors("sniper"), 3 },
			{ xors("ak/m4"), 4 },
			{ xors("aug/sg"), 5 },
			{ xors("mac10/mp9"), 6 },
		};

		static std::vector< dropdown_item_t< > > autobuy_second_wep = {
			{ xors("none"), 0 },
			{ xors("deagle/r8"), 1 },
			{ xors("dualies"), 2 },
		};

		static std::vector< dropdown_item_t< > > mtc = {
			{ xors("default"), 0 },
			{ xors("flat"), 1 },
			{ xors("glass"), 2 },
			{ xors("metallic"), 3 }
		};

		static std::vector< dropdown_item_t< > > skin_knives = {
			{ xors("none"), 0 },
			{ xors("bayonet"), 1 },
			{ xors("flip knife"), 2 },
			{ xors("gut knife"), 3 },
			{ xors("karambit"), 4 },
			{ xors("m9 bayonet"), 5 },
			{ xors("huntsman"), 6 },
		};

		static std::vector< dropdown_item_t< > > weapon_configs = {
			{ xors("default"), 0 }/*,
			{ xors("heavy pistols"), 1 },
			{ xors("snipers"), 2 },
			{ xors("auto snipers"), 3 }*/
		};

		static std::vector< dropdown_item_t< > > tracer_type = {
			{ xors("none"), 0 },
			{ xors("line"), 1 },
			{ xors("beam"), 2 }
		};

		static std::vector< dropdown_item_t< > > tsr = {
			{ xors("off"), 0 },
			{ xors("rainbow"), 1 },
			{ xors("default"), 2 }
		};

		static std::vector< dropdown_item_t< > > gloves = {
			{ xors("none"), 0 },
			{ xors("bloodhound"), 5027 },
			{ xors("sporty"), 5030 },
			{ xors("slick"), 5031 },
			{ xors("handwrap"), 5032 },
			{ xors("motorcycle"), 5033 },
			{ xors("specialist"), 5034 }
		};

		static std::vector< dropdown_item_t< > > glove_skins = {
			{ xors("charred"), 10006 },
		{ xors("snakebite"), 10007 },
		{ xors("bronzed"), 10008 },
		{ xors("leather"), 10009 },
		{ xors("spruce ddpat"), 10010 },
		{ xors("lunar weave"), 10013 },
		{ xors("convoy"), 10015 },
		{ xors("crimson weave"), 10016 },
		{ xors("superconductor"), 10018 },
		{ xors("arid"), 10019 },
		{ xors("slaughter"), 10021 },
		{ xors("eclipse"), 10024 },
		{ xors("spearmint"), 10026 },
		{ xors("boom!"), 10027 },
		{ xors("cool mint"), 10028 },
		{ xors("forest ddpat"), 10030 },
		{ xors("crimson kimono"), 10033 },
		{ xors("emerald web"), 10034 },
		{ xors("foundation"), 10035 },
		{ xors("badlands"), 10036 },
		{ xors("pandora's box"), 10037 },
		{ xors("hedge maze"), 10038 },
		{ xors("guerrilla"), 10039 },
		{ xors("diamondback"), 10040 },
		{ xors("king snake"), 10041 },
		{ xors("imperial plaid"), 10042 },
		{ xors("overtake"), 10043 },
		{ xors("racing green"), 10044 },
		{ xors("amphibious"), 10045 },
		{ xors("bronze morph"), 10046 },
		{ xors("omega"), 10047 },
		{ xors("vice"), 10048 },
		{ xors("pow!"), 10049 },
		{ xors("turtle"), 10050 },
		{ xors("transport"), 10051 },
		{ xors("polygon"), 10052 },
		{ xors("cobalt skulls"), 10053 },
		{ xors("overprint"), 10054 },
		{ xors("duct tape"), 10055 },
		{ xors("arboreal"), 10056 },
		{ xors("emerald"), 10057 },
		{ xors("mangrove"), 10058 },
		{ xors("rattler"), 10059 },
		{ xors("case hardened"), 10060 },
		{ xors("crimson web"), 10061 },
		{ xors("buckshot"), 10062 },
		{ xors("fade"), 10063 },
		{ xors("mogul"), 10064 },
		};

		static std::vector< dropdown_item_t< > > skins = {
			{ xors("none"), -1 },
			{ xors("groundwater"), 2 },
		{ xors("candy apple"), 3 },
		{ xors("forest ddpat"), 5 },
		{ xors("arctic camo"), 6 },
		{ xors("desert storm"), 8 },
		{ xors("bengal tiger"), 9 },
		{ xors("copperhead"), 10 },
		{ xors("skulls"), 11 },
		{ xors("crimson web"), 12 },
		{ xors("blue streak"), 13 },
		{ xors("red laminate"), 14 },
		{ xors("gunsmoke"), 15 },
		{ xors("jungle tiger"), 16 },
		{ xors("urban ddpat"), 17 },
		{ xors("virus"), 20 },
		{ xors("granite marbleized"), 21 },
		{ xors("contrast spray"), 22 },
		{ xors("forest leaves"), 25 },
		{ xors("lichen dashed"), 26 },
		{ xors("bone mask"), 27 },
		{ xors("anodized navy"), 28 },
		{ xors("snake camo"), 30 },
		{ xors("silver"), 32 },
		{ xors("hot rod"), 33 },
		{ xors("metallic ddpat"), 34 },
		{ xors("ossified"), 36 },
		{ xors("blaze"), 37 },
		{ xors("fade"), 38 },
		{ xors("bulldozer"), 39 },
		{ xors("night"), 40 },
		{ xors("copper"), 41 },
		{ xors("blue steel"), 42 },
		{ xors("stained"), 43 },
		{ xors("case hardened"), 44 },
		{ xors("contractor"), 46 },
		{ xors("colony"), 47 },
		{ xors("dragon tattoo"), 48 },
		{ xors("lightning strike"), 51 },
		{ xors("slaughter"), 59 },
		{ xors("dark water"), 60 },
		{ xors("hypnotic"), 61 },
		{ xors("bloomstick"), 62 },
		{ xors("cold blooded"), 67 },
		{ xors("carbon fiber"), 70 },
		{ xors("scorpion"), 71 },
		{ xors("safari mesh"), 72 },
		{ xors("wings"), 73 },
		{ xors("polar camo"), 74 },
		{ xors("blizzard marbleized"), 75 },
		{ xors("winter forest"), 76 },
		{ xors("boreal forest"), 77 },
		{ xors("forest night"), 78 },
		{ xors("orange ddpat"), 83 },
		{ xors("pink ddpat"), 84 },
		{ xors("mudder"), 90 },
		{ xors("cyanospatter"), 92 },
		{ xors("caramel"), 93 },
		{ xors("grassland"), 95 },
		{ xors("blue spruce"), 96 },
		{ xors("ultraviolet"), 98 },
		{ xors("sand dune"), 99 },
		{ xors("storm"), 100 },
		{ xors("tornado"), 101 },
		{ xors("whiteout"), 102 },
		{ xors("grassland leaves"), 104 },
		{ xors("polar mesh"), 107 },
		{ xors("condemned"), 110 },
		{ xors("glacier mesh"), 111 },
		{ xors("sand mesh"), 116 },
		{ xors("sage spray"), 119 },
		{ xors("jungle spray"), 122 },
		{ xors("sand spray"), 124 },
		{ xors("urban perforated"), 135 },
		{ xors("waves perforated"), 136 },
		{ xors("orange peel"), 141 },
		{ xors("urban masked"), 143 },
		{ xors("jungle dashed"), 147 },
		{ xors("sand dashed"), 148 },
		{ xors("urban dashed"), 149 },
		{ xors("jungle"), 151 },
		{ xors("demolition"), 153 },
		{ xors("afterimage"), 154 },
		{ xors("bullet rain"), 155 },
		{ xors("death by kitty"), 156 },
		{ xors("palm"), 157 },
		{ xors("walnut"), 158 },
		{ xors("brass"), 159 },
		{ xors("splash"), 162 },
		{ xors("modern hunter"), 164 },
		{ xors("splash jam"), 165 },
		{ xors("blaze orange"), 166 },
		{ xors("radiation hazard"), 167 },
		{ xors("nuclear threat"), 168 },
		{ xors("fallout warning"), 169 },
		{ xors("predator"), 170 },
		{ xors("irradiated alert"), 171 },
		{ xors("black laminate"), 172 },
		{ xors("boom"), 174 },
		{ xors("scorched"), 175 },
		{ xors("faded zebra"), 176 },
		{ xors("memento"), 177 },
		{ xors("doomkitty"), 178 },
		{ xors("nuclear threat"), 179 },
		{ xors("fire serpent"), 180 },
		{ xors("corticera"), 181 },
		{ xors("emerald dragon"), 182 },
		{ xors("overgrowth"), 183 },
		{ xors("corticera"), 184 },
		{ xors("golden koi"), 185 },
		{ xors("wave spray"), 186 },
		{ xors("zirka"), 187 },
		{ xors("graven"), 188 },
		{ xors("bright water"), 189 },
		{ xors("black limba"), 190 },
		{ xors("tempest"), 191 },
		{ xors("shattered"), 192 },
		{ xors("bone pile"), 193 },
		{ xors("spitfire"), 194 },
		{ xors("demeter"), 195 },
		{ xors("emerald"), 196 },
		{ xors("anodized navy"), 197 },
		{ xors("hazard"), 198 },
		{ xors("dry season"), 199 },
		{ xors("mayan dreams"), 200 },
		{ xors("palm"), 201 },
		{ xors("jungle ddpat"), 202 },
		{ xors("rust coat"), 203 },
		{ xors("mosaico"), 204 },
		{ xors("jungle"), 205 },
		{ xors("tornado"), 206 },
		{ xors("facets"), 207 },
		{ xors("sand dune"), 208 },
		{ xors("groundwater"), 209 },
		{ xors("anodized gunmetal"), 210 },
		{ xors("ocean foam"), 211 },
		{ xors("graphite"), 212 },
		{ xors("ocean foam"), 213 },
		{ xors("graphite"), 214 },
		{ xors("x-ray"), 215 },
		{ xors("blue titanium"), 216 },
		{ xors("blood tiger"), 217 },
		{ xors("hexane"), 218 },
		{ xors("hive"), 219 },
		{ xors("hemoglobin"), 220 },
		{ xors("serum"), 221 },
		{ xors("blood in the water"), 222 },
		{ xors("nightshade"), 223 },
		{ xors("water sigil"), 224 },
		{ xors("ghost camo"), 225 },
		{ xors("blue laminate"), 226 },
		{ xors("electric hive"), 227 },
		{ xors("blind spot"), 228 },
		{ xors("azure zebra"), 229 },
		{ xors("steel disruption"), 230 },
		{ xors("cobalt disruption"), 231 },
		{ xors("crimson web"), 232 },
		{ xors("tropical storm"), 233 },
		{ xors("ash wood"), 234 },
		{ xors("varicamo"), 235 },
		{ xors("night ops"), 236 },
		{ xors("urban rubble"), 237 },
		{ xors("varicamo blue"), 238 },
		{ xors("calicamo"), 240 },
		{ xors("hunting blind"), 241 },
		{ xors("army mesh"), 242 },
		{ xors("gator mesh"), 243 },
		{ xors("teardown"), 244 },
		{ xors("army recon"), 245 },
		{ xors("amber fade"), 246 },
		{ xors("damascus steel"), 247 },
		{ xors("red quartz"), 248 },
		{ xors("cobalt quartz"), 249 },
		{ xors("full stop"), 250 },
		{ xors("pit viper"), 251 },
		{ xors("silver quartz"), 252 },
		{ xors("acid fade"), 253 },
		{ xors("nitro"), 254 },
		{ xors("asiimov"), 255 },
		{ xors("the kraken"), 256 },
		{ xors("guardian"), 257 },
		{ xors("mehndi"), 258 },
		{ xors("redline"), 259 },
		{ xors("pulse"), 260 },
		{ xors("marina"), 261 },
		{ xors("rose iron"), 262 },
		{ xors("rising skull"), 263 },
		{ xors("sandstorm"), 264 },
		{ xors("kami"), 265 },
		{ xors("magma"), 266 },
		{ xors("cobalt halftone"), 267 },
		{ xors("tread plate"), 268 },
		{ xors("the fuschia is now"), 269 },
		{ xors("victoria"), 270 },
		{ xors("undertow"), 271 },
		{ xors("titanium bit"), 272 },
		{ xors("heirloom"), 273 },
		{ xors("copper galaxy"), 274 },
		{ xors("red fragcam"), 275 },
		{ xors("panther"), 276 },
		{ xors("stainless"), 277 },
		{ xors("blue fissure"), 278 },
		{ xors("asiimov"), 279 },
		{ xors("chameleon"), 280 },
		{ xors("corporal"), 281 },
		{ xors("redline"), 282 },
		{ xors("trigon"), 283 },
		{ xors("heat"), 284 },
		{ xors("terrain"), 285 },
		{ xors("antique"), 286 },
		{ xors("pulse"), 287 },
		{ xors("sergeant"), 288 },
		{ xors("sandstorm"), 289 },
		{ xors("guardian"), 290 },
		{ xors("heaven guard"), 291 },
		{ xors("death rattle"), 293 },
		{ xors("green apple"), 294 },
		{ xors("franklin"), 295 },
		{ xors("meteorite"), 296 },
		{ xors("tuxedo"), 297 },
		{ xors("army sheen"), 298 },
		{ xors("caged steel"), 299 },
		{ xors("emerald pinstripe"), 300 },
		{ xors("atomic alloy"), 301 },
		{ xors("vulcan"), 302 },
		{ xors("isaac"), 303 },
		{ xors("slashed"), 304 },
		{ xors("torque"), 305 },
		{ xors("antique"), 306 },
		{ xors("retribution"), 307 },
		{ xors("kami"), 308 },
		{ xors("howl"), 309 },
		{ xors("curse"), 310 },
		{ xors("desert warfare"), 311 },
		{ xors("cyrex"), 312 },
		{ xors("orion"), 313 },
		{ xors("heaven guard"), 314 },
		{ xors("poison dart"), 315 },
		{ xors("jaguar"), 316 },
		{ xors("bratatat"), 317 },
		{ xors("road rash"), 318 },
		{ xors("detour"), 319 },
		{ xors("red python"), 320 },
		{ xors("master piece"), 321 },
		{ xors("nitro"), 322 },
		{ xors("rust coat"), 323 },
		{ xors("chalice"), 325 },
		{ xors("knight"), 326 },
		{ xors("chainmail"), 327 },
		{ xors("hand cannon"), 328 },
		{ xors("dark age"), 329 },
		{ xors("briar"), 330 },
		{ xors("royal blue"), 332 },
		{ xors("indigo"), 333 },
		{ xors("twist"), 334 },
		{ xors("module"), 335 },
		{ xors("desert-strike"), 336 },
		{ xors("tatter"), 337 },
		{ xors("pulse"), 338 },
		{ xors("caiman"), 339 },
		{ xors("jet set"), 340 },
		{ xors("first class"), 341 },
		{ xors("leather"), 342 },
		{ xors("commuter"), 343 },
		{ xors("dragon lore"), 344 },
		{ xors("first class"), 345 },
		{ xors("coach class"), 346 },
		{ xors("pilot"), 347 },
		{ xors("red leather"), 348 },
		{ xors("osiris"), 349 },
		{ xors("tigris"), 350 },
		{ xors("conspiracy"), 351 },
		{ xors("fowl play"), 352 },
		{ xors("water elemental"), 353 },
		{ xors("urban hazard"), 354 },
		{ xors("desert-strike"), 355 },
		{ xors("koi"), 356 },
		{ xors("ivory"), 357 },
		{ xors("supernova"), 358 },
		{ xors("asiimov"), 359 },
		{ xors("cyrex"), 360 },
		{ xors("abyss"), 361 },
		{ xors("labyrinth"), 362 },
		{ xors("traveler"), 363 },
		{ xors("business class"), 364 },
		{ xors("olive plaid"), 365 },
		{ xors("green plaid"), 366 },
		{ xors("reactor"), 367 },
		{ xors("setting sun"), 368 },
		{ xors("nuclear waste"), 369 },
		{ xors("bone machine"), 370 },
		{ xors("styx"), 371 },
		{ xors("nuclear garden"), 372 },
		{ xors("contamination"), 373 },
		{ xors("toxic"), 374 },
		{ xors("radiation hazard"), 375 },
		{ xors("chemical green"), 376 },
		{ xors("hot shot"), 377 },
		{ xors("fallout warning"), 378 },
		{ xors("cerberus"), 379 },
		{ xors("wasteland rebel"), 380 },
		{ xors("grinder"), 381 },
		{ xors("murky"), 382 },
		{ xors("basilisk"), 383 },
		{ xors("griffin"), 384 },
		{ xors("firestarter"), 385 },
		{ xors("dart"), 386 },
		{ xors("urban hazard"), 387 },
		{ xors("cartel"), 388 },
		{ xors("fire elemental"), 389 },
		{ xors("highwayman"), 390 },
		{ xors("cardiac"), 391 },
		{ xors("delusion"), 392 },
		{ xors("tranquility"), 393 },
		{ xors("cartel"), 394 },
		{ xors("man-o'-war"), 395 },
		{ xors("urban shock"), 396 },
		{ xors("naga"), 397 },
		{ xors("chatterbox"), 398 },
		{ xors("catacombs"), 399 },
		{ xors("dragon king"), 400 },
		{ xors("system lock"), 401 },
		{ xors("malachite"), 402 },
		{ xors("deadly poison"), 403 },
		{ xors("muertos"), 404 },
		{ xors("serenity"), 405 },
		{ xors("grotto"), 406 },
		{ xors("quicksilver"), 407 },
		{ xors("tiger tooth"), 409 },
		{ xors("damascus steel"), 410 },
		{ xors("damascus steel"), 411 },
		{ xors("marble fade"), 413 },
		{ xors("rust coat"), 414 },
		{ xors("doppler"), 415 },
		{ xors("doppler"), 416 },
		{ xors("doppler"), 417 },
		{ xors("doppler"), 418 },
		{ xors("doppler"), 419 },
		{ xors("doppler"), 420 },
		{ xors("doppler"), 421 },
		{ xors("elite build"), 422 },
		{ xors("armor core"), 423 },
		{ xors("worm god"), 424 },
		{ xors("bronze deco"), 425 },
		{ xors("valence"), 426 },
		{ xors("monkey business"), 427 },
		{ xors("eco"), 428 },
		{ xors("djinn"), 429 },
		{ xors("hyper beast"), 430 },
		{ xors("heat"), 431 },
		{ xors("man-o'-war"), 432 },
		{ xors("neon rider"), 433 },
		{ xors("origami"), 434 },
		{ xors("pole position"), 435 },
		{ xors("grand prix"), 436 },
		{ xors("twilight galaxy"), 437 },
		{ xors("chronos"), 438 },
		{ xors("hades"), 439 },
		{ xors("icarus fell"), 440 },
		{ xors("minotaur's labyrinth"), 441 },
		{ xors("asterion"), 442 },
		{ xors("pathfinder"), 443 },
		{ xors("daedalus"), 444 },
		{ xors("hot rod"), 445 },
		{ xors("medusa"), 446 },
		{ xors("duelist"), 447 },
		{ xors("pandora's box"), 448 },
		{ xors("poseidon"), 449 },
		{ xors("moon in libra"), 450 },
		{ xors("sun in leo"), 451 },
		{ xors("shipping forecast"), 452 },
		{ xors("emerald"), 453 },
		{ xors("para green"), 454 },
		{ xors("akihabara accept"), 455 },
		{ xors("hydroponic"), 456 },
		{ xors("bamboo print"), 457 },
		{ xors("bamboo shadow"), 458 },
		{ xors("bamboo forest"), 459 },
		{ xors("aqua terrace"), 460 },
		{ xors("counter terrace"), 462 },
		{ xors("terrace"), 463 },
		{ xors("neon kimono"), 464 },
		{ xors("orange kimono"), 465 },
		{ xors("crimson kimono"), 466 },
		{ xors("mint kimono"), 467 },
		{ xors("midnight storm"), 468 },
		{ xors("sunset storm"), 469 },
		{ xors("sunset storm"), 470 },
		{ xors("daybreak"), 471 },
		{ xors("impact drill"), 472 },
		{ xors("seabird"), 473 },
		{ xors("aquamarine revenge"), 474 },
		{ xors("hyper beast"), 475 },
		{ xors("yellow jacket"), 476 },
		{ xors("neural net"), 477 },
		{ xors("rocket pop"), 478 },
		{ xors("bunsen burner"), 479 },
		{ xors("evil daimyo"), 480 },
		{ xors("nemesis"), 481 },
		{ xors("ruby poison dart"), 482 },
		{ xors("loudmouth"), 483 },
		{ xors("ranger"), 484 },
		{ xors("handgun"), 485 },
		{ xors("elite build"), 486 },
		{ xors("cyrex"), 487 },
		{ xors("riot"), 488 },
		{ xors("torque"), 489 },
		{ xors("frontside misty"), 490 },
		{ xors("dualing dragons"), 491 },
		{ xors("survivor z"), 492 },
		{ xors("flux"), 493 },
		{ xors("stone cold"), 494 },
		{ xors("wraiths"), 495 },
		{ xors("nebula crusader"), 496 },
		{ xors("golden coil"), 497 },
		{ xors("rangeen"), 498 },
		{ xors("cobalt core"), 499 },
		{ xors("special delivery"), 500 },
		{ xors("wingshot"), 501 },
		{ xors("green marine"), 502 },
		{ xors("big iron"), 503 },
		{ xors("kill confirmed"), 504 },
		{ xors("scumbria"), 505 },
		{ xors("point disarray"), 506 },
		{ xors("ricochet"), 507 },
		{ xors("fuel rod"), 508 },
		{ xors("corinthian"), 509 },
		{ xors("retrobution"), 510 },
		{ xors("the executioner"), 511 },
		{ xors("royal paladin"), 512 },
		{ xors("power loader"), 514 },
		{ xors("imperial"), 515 },
		{ xors("shapewood"), 516 },
		{ xors("yorick"), 517 },
		{ xors("outbreak"), 518 },
		{ xors("tiger moth"), 519 },
		{ xors("avalanche"), 520 },
		{ xors("teclu burner"), 521 },
		{ xors("fade"), 522 },
		{ xors("amber fade"), 523 },
		{ xors("fuel injector"), 524 },
		{ xors("elite build"), 525 },
		{ xors("photic zone"), 526 },
		{ xors("kumicho dragon"), 527 },
		{ xors("cartel"), 528 },
		{ xors("valence"), 529 },
		{ xors("triumvirate"), 530 },
		{ xors("royal legion"), 532 },
		{ xors("the battlestar"), 533 },
		{ xors("lapis gator"), 534 },
		{ xors("praetorian"), 535 },
		{ xors("impire"), 536 },
		{ xors("hyper beast"), 537 },
		{ xors("necropos"), 538 },
		{ xors("jambiya"), 539 },
		{ xors("lead conduit"), 540 },
		{ xors("fleet flock"), 541 },
		{ xors("judgement of anubis"), 542 },
		{ xors("red astor"), 543 },
		{ xors("ventilators"), 544 },
		{ xors("orange crash"), 545 },
		{ xors("firefight"), 546 },
		{ xors("spectre"), 547 },
		{ xors("chantico's fire"), 548 },
		{ xors("bioleak"), 549 },
		{ xors("oceanic"), 550 },
		{ xors("asiimov"), 551 },
		{ xors("fubar"), 552 },
		{ xors("atlas"), 553 },
		{ xors("ghost crusader"), 554 },
		{ xors("re-entry"), 555 },
		{ xors("primal saber"), 556 },
		{ xors("black tie"), 557 },
		{ xors("lore"), 558 },
		{ xors("lore"), 559 },
		{ xors("lore"), 560 },
		{ xors("lore"), 561 },
		{ xors("lore"), 562 },
		{ xors("black laminate"), 563 },
		{ xors("black laminate"), 564 },
		{ xors("black laminate"), 565 },
		{ xors("black laminate"), 566 },
		{ xors("black laminate"), 567 },
		{ xors("gamma doppler"), 568 },
		{ xors("gamma doppler"), 569 },
		{ xors("gamma doppler"), 570 },
		{ xors("gamma doppler"), 571 },
		{ xors("gamma doppler"), 572 },
		{ xors("autotronic"), 573 },
		{ xors("autotronic"), 574 },
		{ xors("autotronic"), 575 },
		{ xors("autotronic"), 576 },
		{ xors("autotronic"), 577 },
		{ xors("bright water"), 578 },
		{ xors("bright water"), 579 },
		{ xors("freehand"), 580 },
		{ xors("freehand"), 581 },
		{ xors("freehand"), 582 },
		{ xors("aristocrat"), 583 },
		{ xors("phobos"), 584 },
		{ xors("violent daimyo"), 585 },
		{ xors("wasteland rebel"), 586 },
		{ xors("mecha industries"), 587 },
		{ xors("desolate space"), 588 },
		{ xors("carnivore"), 589 },
		{ xors("exo"), 590 },
		{ xors("imperial dragon"), 591 },
		{ xors("iron clad"), 592 },
		{ xors("chopper"), 593 },
		{ xors("harvester"), 594 },
		{ xors("reboot"), 595 },
		{ xors("limelight"), 596 },
		{ xors("bloodsport"), 597 },
		{ xors("aerial"), 598 },
		{ xors("ice cap"), 599 },
		{ xors("neon revolution"), 600 },
		{ xors("syd mead"), 601 },
		{ xors("imprint"), 602 },
		{ xors("directive"), 603 },
		{ xors("roll cage"), 604 },
		{ xors("scumbria"), 605 },
		{ xors("ventilator"), 606 },
		{ xors("weasel"), 607 },
		{ xors("petroglyph"), 608 },
		{ xors("airlock"), 609 },
		{ xors("dazzle"), 610 },
		{ xors("grim"), 611 },
		{ xors("powercore"), 612 },
		{ xors("triarch"), 613 },
		{ xors("fuel injector"), 614 },
		{ xors("briefing"), 615 },
		{ xors("slipstream"), 616 },
		{ xors("doppler"), 617 },
		{ xors("doppler"), 618 },
		{ xors("doppler"), 619 },
		{ xors("ultraviolet"), 620 },
		{ xors("ultraviolet"), 621 },
		{ xors("polymer"), 622 },
		{ xors("ironwork"), 623 },
		{ xors("dragonfire"), 624 },
		{ xors("royal consorts"), 625 },
		{ xors("mecha industries"), 626 },
		{ xors("cirrus"), 627 },
		{ xors("stinger"), 628 },
		{ xors("black sand"), 629 },
		{ xors("sand scale"), 630 },
		{ xors("flashback"), 631 },
		{ xors("buzz kill"), 632 },
		{ xors("sonar"), 633 },
		{ xors("gila"), 634 },
		{ xors("turf"), 635 },
		{ xors("shallow grave"), 636 },
		{ xors("cyrex"), 637 },
		{ xors("wasteland princess"), 638 },
		{ xors("bloodsport"), 639 },
		{ xors("fever dream"), 640 },
		{ xors("jungle slipstream"), 641 },
		{ xors("blueprint"), 642 },
		{ xors("xiangliu"), 643 },
		{ xors("decimator"), 644 },
		{ xors("oxide blaze"), 645 },
		{ xors("capillary"), 646 },
		{ xors("crimson tsunami"), 647 },
		{ xors("emerald poison dart"), 648 },
		{ xors("akoben"), 649 },
		{ xors("ripple"), 650 },
		{ xors("last dive"), 651 },
		{ xors("scaffold"), 652 },
		{ xors("neo-noir"), 653 },
		{ xors("seasons"), 654 },
		};
	}
}


namespace ui
{
	template < typename t = int >
	class c_dropdown : public base_item {
	public:
		c_dropdown(int x, int y, int w, const char* name, t* setting,
			std::vector< dropdowns::dropdown_item_t< t > >* items, size_t max_items = 8) :
			base_item(x, y, w, 16, name), m_dropdown_items(items), m_setting(setting),
			m_max_items(max_items) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int x = get_relative_x();
			int y = get_relative_y() + 12;
			int h = m_height;

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);


			return mouse_x >= x && mouse_x <= x + m_width
				&& mouse_y >= y && mouse_y <= y + h;
		}

		inline bool is_any_item_hovered() {
			if (m_disabled || !m_active) return false;

			int x = get_relative_x();
			int y = get_relative_y() + m_height + 12;
			int h = m_height * (std::min< size_t >(
				m_dropdown_items->size(), m_max_items));

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);


			return mouse_x >= x && mouse_x <= x + m_width
				&& mouse_y >= y && mouse_y <= y + h;
		}

		virtual int get_total_height() const override {
			return m_height + 13;
		}

		void draw_box(const int& x, const int& y, const char* str, bool hovered = false) {
			ui_draw_rect(x, y, m_width, m_height, hovered ? ui_get_disabled_col() : ui_get_bg_col());
			ui_draw_line(x, y + m_height, x + m_width, y + m_height, ui_get_accent_col());

			ui_draw_string(x + m_width / 2, y + 2, true, ui_get_text_col(), str);
		}

		void update_value() {
			for (auto& it : *m_dropdown_items) {
				if (it.m_value == *m_setting) {
					m_selected_item = &it;
				}
			}
		}

		void draw_items(const int& x, const int& y) {
			auto& items = *m_dropdown_items;
			auto it = &items.front();
			int offset = m_height + 1;
			int hovered = 0;
			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);

			auto is_hovered = [&](int y_offset) {
				return mouse_x >= x && mouse_x <= x + m_width
					&& mouse_y >= y + y_offset && mouse_y <= y + y_offset + m_height;
			};


			for (size_t i = items.size() > m_max_items ? m_curr_scroll : 0;
				i < std::min< size_t >(m_dropdown_items->size(), m_max_items + m_curr_scroll);
				++i, offset += m_height + 1
				) {
				it = &items.at(i);

				draw_box(x, y + offset, it->m_name);

				if (is_hovered(offset)) {
					hovered = offset;
					if (g_input.is_key_pressed(KEYS_MOUSE1)) {
						m_selected_item = it;
						*m_setting = it->m_value;
						m_active = false;
						m_has_clicked_item = true;
					}
				}
			}

			if (hovered) {
				ui_draw_outlined_rect(x - 1, y - 1 + hovered,
					m_width + 1, m_height + 1, ui_get_text_col());
			}
		}

		void input() {
			static float enable_time{ };
			static bool  needs_reenable{ };
			bool active_backup = m_active;
			bool active_changed = false;

			if (is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				if (!m_mouse_held) {
					m_active = !m_active;
				}
				m_mouse_held = true;
			}
			else if (!is_any_item_hovered()) {
				m_mouse_held = false;
			}

			if (!is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1) && !is_any_item_hovered()) {
				m_active = false;
			}

			if (GetTickCount() * 0.001f > enable_time && needs_reenable) {
				set_disabled_callbacks(false);
				needs_reenable = false;
			}


			active_changed = m_active != active_backup;
			//disable input on all items
			if (active_changed || m_has_clicked_item) {
				if (!m_active) {
					enable_time = GetTickCount() * 0.001f + 0.220f;
					needs_reenable = true;
				}
				else {
					set_disabled_callbacks(true);
				}

				m_has_clicked_item = false;
			}

			if (m_selected_item) {
				*m_setting = m_selected_item->m_value;
			}

			if (m_active && m_dropdown_items->size() > m_max_items) {
				int scroll_input = g_input.get_scroll_state();

				if (m_curr_scroll > 0 || scroll_input < 0) //we dont want scroll to loop around from 0 to max
					m_curr_scroll -= scroll_input; //because positive is scroll up, we gotta flip it

				if (m_curr_scroll > m_dropdown_items->size() - m_max_items)
					m_curr_scroll = m_dropdown_items->size() - m_max_items;
			}
		}

		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();

			bool restore = false;
			RECT prev_rect{ };

			if (m_active) {
				restore = true;
				g_d3d.get_device()->GetScissorRect(&prev_rect);

				RECT new_rect{
					prev_rect.left,
					prev_rect.top,
					g_d3d.m_width,
					g_d3d.m_height,
				};

				g_d3d.get_device()->SetScissorRect(&new_rect);

				draw_items(x, y + 11);


				//draw scrollbar
				size_t total_items = m_dropdown_items->size();
				if (total_items > m_max_items) {
					const size_t height = (m_height + 1) * m_max_items;
					const float slider_step = (float)(height) / float(total_items - m_max_items + 1);

					size_t slider_pos = static_cast<size_t>(slider_step * m_curr_scroll);
					ui_draw_rect(x + m_width - 1, y + slider_pos + m_height + 13, 2, (int)slider_step, ui_get_accent_col());
				}
			}

			update_value();
			input();

			ui_draw_string(x + 2, y, false, ui_get_text_col(), m_text);
			ui_draw_rect(x, y + 13, m_width, m_height, ui_get_disabled_col());
			ui_draw_outlined_rect(x - 1, y + 12, m_width + 1, m_height + 1,
				is_hovered() || m_active ? ui_get_text_col() : ui_get_accent_col());

			if (m_selected_item) {
				ui_draw_string(x + m_width / 2, y + 14, true, ui_get_text_col(), m_selected_item->m_name);
			}

			if (restore) {
				g_d3d.get_device()->SetScissorRect(&prev_rect);
			}
		}

	protected:
		std::vector< dropdowns::dropdown_item_t< t > >* m_dropdown_items{ };
		dropdowns::dropdown_item_t< t >* m_selected_item{ };
		bool m_active = false;
		bool m_mouse_held = false;
		bool m_has_clicked_item = true;
		t* m_setting{ };
		size_t m_max_items{ };
		size_t m_curr_scroll{ };
	};
}



namespace ui
{
	class c_key_picker : public base_item {
	public:
		c_key_picker(int x, int y, int w, const char* name, int* setting) :
			base_item(x, y, w, 16, name), m_setting(setting) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int x = get_relative_x();
			int y = get_relative_y() + 12;
			int h = m_height;

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);


			return mouse_x >= x && mouse_x <= x + m_width
				&& mouse_y >= y && mouse_y <= y + h;
		}

		virtual int get_total_height() const override {
			return m_height + 12;
		}

		void input() {
			bool active_backup = m_active;
			bool active_changed = false;

			if (is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				if (!m_mouse_held) {
					m_active = true;
				}
				m_mouse_held = true;
			}

			if (g_input.is_key_pressed(KEYS_ESCAPE)) {
				m_active = false;
				*m_setting = KEYS_NONE;
			}

			if (m_active && !m_mouse_held) {
				int key = g_input.is_any_key_pressed();
				if (key != KEYS_NONE) {
					*m_setting = key;
					m_active = false;
				}
			}

			active_changed = active_backup != m_active;
			if (active_changed) {
				set_disabled_callbacks(m_active);
			}
		}

		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();

			input();

			ui_draw_string(x + 2, y, false, ui_get_text_col(), m_text);
			ui_draw_rect(x, y + 13, m_width, m_height, ui_get_disabled_col());
			ui_draw_outlined_rect(x - 1, y + 12, m_width + 1, m_height + 1,
				is_hovered() || m_active ? ui_get_text_col() : ui_get_accent_col());

			ui_draw_string(x + m_width / 2, y + 14, true, ui_get_text_col(),
				g_input.get_key_name((VirtualKeys_t)*m_setting));
		}

	protected:
		int* m_setting{ };
		bool m_active{ };
		bool m_mouse_held{ };
	};

	//skEeT PiCkErS
	class c_key_picker_small : public base_item {
	public:
		c_key_picker_small(int x, int y, int* setting) :
			base_item(x, y, 0, 0, ("KEY_PICKER")), m_setting(setting) { }

		virtual int get_total_height() const override {
			const char* name = g_input.get_short_name((VirtualKeys_t)*m_setting);
			int w, h;
			ui_get_text_size(w, h, name);

			return h;
		}

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			const char* name = g_input.get_short_name((VirtualKeys_t)*m_setting);
			int x = get_relative_x();
			int y = get_relative_y();
			int mouse_x, mouse_y;
			int w, h;

			ui_get_text_size(w, h, "[%s]", name);
			ui_get_cursor_pos(mouse_x, mouse_y);

			return mouse_x >= x - w && mouse_x <= x
				&& mouse_y >= y && mouse_y <= y + h;
		}

		void input() {

			bool active_backup = m_active;
			bool active_changed = false;

			if (g_input.is_key_pressed(KEYS_ESCAPE)) {
				m_active = false;
				*m_setting = KEYS_NONE;
			}

			if (m_active && !m_mouse_held) {
				int key = g_input.is_any_key_pressed();
				if (key != KEYS_NONE) {
					*m_setting = key;
					m_active = false;
				}
			}

			if (is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				if (!m_mouse_held) {
					m_active = true;
				}
				m_mouse_held = true;
			}
			else {
				m_mouse_held = false;
			}

			active_changed = active_backup != m_active;
			if (active_changed) {
				set_disabled_callbacks(m_active);
			}
		}

		virtual void render() override {
			const char* name = g_input.get_short_name((VirtualKeys_t)*m_setting);
			int x = get_relative_x();
			int y = get_relative_y();

			int w, h;
			ui_get_text_size(w, h, "[%s]", name);

			input();

			ui_draw_string(x - w, y, false, is_hovered() || m_active ?
				ui_get_accent_col() : ui_get_text_col(), "[%s]", name);
		}

	protected:
		int* m_setting;
		bool m_active{ };
		bool m_mouse_held{ };
	};
}




namespace ui
{
	class c_button : public base_item {
	public:
		c_button(int x, int y, int w, int h, const char* name, std::function< void() > fn) :
			base_item(x, y, w, h, name), m_fn(fn) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int x = get_relative_x();
			int y = get_relative_y();
			int h = m_height;

			int mouse_x, mouse_y;
			ui_get_cursor_pos(mouse_x, mouse_y);


			return mouse_x >= x && mouse_x <= x + m_width
				&& mouse_y >= y && mouse_y <= y + h;
		}


		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();

			ui_draw_rect(x, y, m_width, m_height, ui_get_disabled_col());
			ui_draw_outlined_rect(x - 1, y - 1, m_width + 1, m_height + 1,
				is_hovered() ? ui_get_text_col() : ui_get_accent_col());

			if (is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				ui_draw_rect(x, y, m_width, m_height, ui_get_bg_col());
				if (!m_mouse_held) {
					m_fn();
				}
				m_mouse_held = true;
			}
			else {
				m_mouse_held = false;
			}

			ui_draw_string(x + m_width / 2, y + 2, true, ui_get_text_col(), m_text);
		}

	protected:
		std::function< void() > m_fn;
		bool m_mouse_held{ };
	};
}







namespace ui
{
	class c_color_picker : public base_item {
		static constexpr int BOX_WIDTH = 106;
		static constexpr int BOX_HEIGHT = 125;
	public:
		c_color_picker(int x, int y, int w, const char* name, color_t* setting) :
			base_item(x, y, w, 4, name), m_setting(setting),
			m_has_text(true) { }

		c_color_picker(int x, int y, int w, color_t* setting) :
			base_item(x, y, w, 8, xors("COLOR_PICKER")), m_setting(setting),
			m_has_text(false) { }

		virtual bool is_hovered() override {
			if (m_disabled) return false;

			int x = get_relative_x();
			int y = get_relative_y();

			int cursor_x, cursor_y;
			ui_get_cursor_pos(cursor_x, cursor_y);

			if (m_has_text) y += 12;

			if (!m_active) {
				return cursor_x >= x && cursor_x <= x + m_width
					&& cursor_y >= y && cursor_y <= y + m_height;
			}

			return cursor_x >= x && cursor_x <= x + BOX_WIDTH + 23
				&& cursor_y >= y && cursor_y <= y + BOX_HEIGHT + 2;
		}

		virtual int get_total_height() const override {
			return m_has_text ? (m_height + 12) : m_height;
		}

		void input() {
			bool active_backup = m_active;
			bool active_changed = false;

			if (is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				if (!m_active) {
					m_mouse_held = true;
				}

				m_active = true;
			}
			else if (m_active && !is_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				m_active = false;
			}
			else {
				m_mouse_held = false;
			}

			active_changed = active_backup != m_active;
			if (active_changed) {
				set_disabled_callbacks(m_active);
			}

			m_hue = m_setting->hue();
			if (m_hue > 1.0f) {
				m_hue -= 359.f;
			}

			m_saturation = m_setting->saturation();
			m_brightness = m_setting->brightness() / 255.f;
			m_alpha = m_setting->a();
		}

		void output() {
			*m_setting = color_t::from_hsb(m_hue, m_saturation, m_brightness);
			m_setting->a() = m_alpha;
		}

		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();

			if (m_has_text) {
				ui_draw_string(x + 2, y, false, ui_get_text_col(), m_text);
				y += 12;
			}

			input();

			if (m_active) {
				RECT old_rect;
				g_d3d.get_device()->GetScissorRect(&old_rect);

				RECT new_rect{
					x - 1, y - 1,
					x + BOX_WIDTH + 22,
					y + BOX_HEIGHT + 2
				};

				g_d3d.get_device()->SetScissorRect(&new_rect);

				int mouse_x, mouse_y;
				ui_get_cursor_pos(mouse_x, mouse_y);

				color_t bg_col(0, 0, 0, 90);
				ui_draw_rect(x, y, BOX_WIDTH + 20, BOX_HEIGHT, bg_col);
				ui_draw_outlined_rect(x - 1, y - 1, BOX_WIDTH + 22, BOX_HEIGHT + 2, ui_get_accent_col());

				for (int i{ }; i < 100; i += 3) {
					for (int i2{ }; i2 < 100; i2 += 3) {
						ui_draw_rect(x + i + 1, y + i2 + 1, 3, 3,
							color_t::from_hsb(m_hue, float(i2) * 0.01f, float(i) * 0.01f));
					}
				}

				if (g_input.is_key_pressed(KEYS_MOUSE1) && !m_mouse_held
					&& mouse_x > x && mouse_x <= x + 100
					&& mouse_y > y && mouse_y <= y + 100) {

					int mouse_x_c = std::clamp(mouse_x, x, x + 100);
					int mouse_y_c = std::clamp(mouse_y, y, y + 100);

					int delta_y = std::clamp(mouse_y_c - y, 0, 100);
					int delta_x = std::clamp(mouse_x_c - x, 0, 100);

					m_saturation = float(delta_y) * 0.01f;
					m_brightness = float(delta_x) * 0.01f;
				}

				auto is_hue_slider_hovered = [&]() -> bool {
					return mouse_x > x + 110 && mouse_x < x + 122
						&& mouse_y > y && mouse_y < y + 100;
				};

				auto draw_slider_hue = [&]() {
					for (int i{ }; i < 100; ++i) {
						auto cur_col = color_t::from_hsb(float(i) * 0.01f, m_saturation, m_brightness);

						ui_draw_rect(x + 110, y + i + 1, 12, 2, cur_col);
					}

					ui_draw_outlined_rect(x + 109, y + int(m_hue * 100.f) + 1, 14, 3,
						is_hue_slider_hovered() ? ui_get_text_col() : ui_get_disabled_col());
				};

				auto is_alpha_slider_hovered = [&]() -> bool {
					return mouse_x > x + 1 && mouse_x < x + 122
						&& mouse_y > y + 110 && mouse_y < y + 124;
				};

				auto draw_slider_alpha = [&]() {
					for (int i{ 121 }; i >= 0; --i) {
						auto col = *m_setting;
						col.a() = (int)(float(i) * 255.f / 121.f);

						ui_draw_rect(x + i + 1, y + 110, 1, 12, col);
					}

					int a_pos = (int)(float(m_alpha) * 121.f / 255.f);
					ui_draw_outlined_rect(x + 1 + a_pos, y + 109, 3, 14,
						is_alpha_slider_hovered() ? ui_get_text_col() : ui_get_disabled_col());
				};

				draw_slider_hue();
				if (is_hue_slider_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
					int delta = std::clamp(mouse_y - y, 0, 100);
					m_hue = float(delta) * 0.01f;
				}

				draw_slider_alpha();
				if (is_alpha_slider_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
					int delta = std::clamp(mouse_x - x, 0, 121);
					m_alpha = (int)(float(delta) * 255.f / 121.f);
				}

				output();

				g_d3d.get_device()->SetScissorRect(&old_rect);
			}
			else {
				if (is_hovered()) {
					ui_draw_rect(x - 1, y - 1, m_width + 2, m_height + 2, ui_get_text_col());
				}
				ui_draw_rect(x, y, m_width, m_height, *m_setting);
			}
		}

	protected:
		color_t* m_setting = nullptr;
		bool m_active = false;
		bool m_mouse_held = false;
		float m_saturation = 1.0f;
		float m_brightness = 1.0f;
		float m_hue = 0.f;
		uint8_t m_alpha = 255;
		bool m_has_text = false;
	};

}







namespace ui
{
	class c_label : public base_item {
	public:
		c_label(int x, int y, const char* text) :
			base_item(x, y, 0, 16, text) { }

		virtual void render() override {
			int x = get_relative_x();
			int y = get_relative_y();

			ui_draw_string(x + 2, y + 2, false, ui_get_text_col(), m_text);
		}
	};
}



namespace ui
{
	template < typename t, size_t size >
	class c_itemlist : public base_item {
	public:
		static constexpr size_t ITEM_SIZE = 18;

		c_itemlist(int x, int y, int width, const char* name, std::vector< dropdowns::dropdown_item_t< t > >* dropdowns, t* setting)
			: base_item(x, y, width, 20, name), m_items(dropdowns), m_setting(setting) { }

		void text_input();
		bool is_textbox_hovered() {
			if (m_disabled)
				return false;

			int x = get_relative_x();
			int y = get_relative_y() + 12;

			int mx, my;
			ui_get_cursor_pos(mx, my);

			return mx >= x && my >= y
				&& mx <= x + m_width && my <= y + ITEM_SIZE;
		}

		void input() {
			text_input();
		}

		virtual int get_total_height() const override {
			size_t total_items = 0;

			if (strlen(m_search_text)) {
				for (size_t i{ }; i < m_items->size(); ++i) {
					if (!strstr(m_items->at(i).m_name, m_search_text))
						continue;

					++total_items;
				}
			}
			else
				total_items = m_items->size();

			return 12 + 20 + total_items * 20;
		}

		bool is_item_hovered(int x, int y) {
			if (m_disabled)
				return false;

			int mx, my;
			ui_get_cursor_pos(mx, my);

			return mx >= x && mx <= x + m_width
				&& my >= y && my <= y + ITEM_SIZE;
		}

		void draw_items(int x, int y) {
			int max_y = INT_MAX;

			auto parent = get_parent();
			if (parent->is_form()) {
				max_y = parent->get_relative_y() + parent->get_total_height() + ITEM_SIZE;
			}

			std::string search_str(m_search_text);
			std::transform(search_str.begin(), search_str.end(), search_str.begin(), [](char c) { return tolower(c); });

			int text_len = strlen(m_search_text);
			for (size_t i{ }; i < m_items->size(); ++i) {
				if (y > max_y)
					break;

				auto& item = m_items->at(i);

				if (!strstr(item.m_name, m_search_text))
					continue;

				ui_draw_rect(x - 1, y - 1, m_width + 2, ITEM_SIZE + 2,
					is_item_hovered(x, y) || *m_setting == item.m_value
					? ui_get_text_col() : ui_get_accent_col());

				ui_draw_rect(x, y, m_width, ITEM_SIZE, ui_get_disabled_col());

				ui_draw_string(x + 2, y + 3, false, ui_get_text_col(), item.m_name);

				if (g_input.is_key_pressed(KEYS_MOUSE1) && is_item_hovered(x, y))
					*m_setting = item.m_value;

				y += ITEM_SIZE + 2;
			}
		}

		virtual void render() override {
			input();

			int x = get_relative_x();
			int y = get_relative_y();

			ui_draw_string(x + 2, y, false, ui_get_text_col(), m_text);

			ui_draw_rect(x - 1, y + 12, m_width + 2, ITEM_SIZE + 2, is_textbox_hovered() || m_box_active ? ui_get_text_col() : ui_get_accent_col());
			if (is_textbox_hovered()) {
				ui_draw_rect(x, y + 13, m_width, ITEM_SIZE, ui_get_text_col());
			}
			ui_draw_rect(x + 1, y + 14, m_width - 2, ITEM_SIZE - 2, ui_get_disabled_col());
			ui_draw_string(x + 3, y + 17, false, ui_get_text_col(), m_box_active ? "%s_" : "%s", m_search_text);

			draw_items(x, y + 14 + ITEM_SIZE);
		}

	private:
		std::vector< dropdowns::dropdown_item_t< t > >* m_items;
		bool m_box_active{ };
		bool m_was_held{ };
		char m_search_text[size]{ };
		float m_last_key_input[KEYS_LAST]{ };
		uint8_t m_key_states[256]{ };
		t* m_setting{ };
	};
}




namespace ui
{
	template < typename t, size_t size >
	void c_itemlist< t, size >::text_input() {
		if (is_textbox_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
			if (!m_was_held) {
				m_box_active ^= 1;
			}
			m_was_held = true;
		}
		else {
			m_was_held = false;
		}

		if (m_box_active) {
			if (!is_textbox_hovered() && g_input.is_key_pressed(KEYS_MOUSE1)) {
				m_box_active = false;
			}

			float current_time = GetTickCount() * 0.001f;
			size_t length = strlen(m_search_text);

			for (size_t i{ }; i < 0xfe; ++i) {
				if (g_input.is_key_pressed(i)) {
					float delta_time = current_time - m_last_key_input[i];
					if (fabs(delta_time) > 0.2f) {
						if (i == KEYS_BACK) {
							m_search_text[length - 1] = 0;
							m_last_key_input[i] = current_time;
							continue;
						}

						m_key_states[i] = 0xf0;
						wchar_t pressed_char;
						const auto scan = MapVirtualKeyA(i, 2);
						auto ret = ToAscii(i, scan, (BYTE*)m_key_states, (LPWORD)&pressed_char, 1);

						if (ret == 1) {
							if (length < size) {
								m_search_text[length] = (char)(pressed_char);
								m_search_text[length + 1] = 0;
							}
						}
						m_last_key_input[i] = current_time;
					}
				}
				else {
					m_last_key_input[i] = 0.f;
					m_key_states[i] = 0;
				}
			}

			if (g_input.is_key_pressed(KEYS_RETURN)) {
				m_box_active = false;
			}
		}
	}
}





namespace ui
{
	extern int current_category;
	#define xors( s ) ( s )
	static std::string date_str = __DATE__;
	inline auto menu = std::make_shared< ui::c_menu >(10, 10, 580, 470, xors("quebot"),
		__DATE__);

	extern void render();

}


namespace keybind_shonax_fx
{
	inline int double_tap_fx;
	inline int hideshot;

	inline int fsp;
	inline int ba;
	inline int od;
	inline int inverter;


	inline int l;
	inline int r;
	inline int b;
	inline int f;

	inline int fd;
	inline int sw;
	inline int tp;
}
