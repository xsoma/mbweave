#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "../colors.h"
#include <vector>

#include "icons/legit.hpp"
#include "icons/rage.hpp"
#include "icons/visuals.hpp"
#include "icons/misc.hpp"
#include "icons/config.hpp"



#include "../Variables.h"

using ulong_t = unsigned long;
using uword_t = unsigned short;

enum D3DFontFlags_t {
	D3DFONTFLAG_OUTLINE = 0x10,
	D3DFONTFLAG_DROPSHADOW = 0x100,
};

enum GradientType_t {
	GRADIENT_HORIZONTAL,
	GRADIENT_VERTICAL
};

enum FontAlign_t : size_t {
	ALIGN_CENTER,
	ALIGN_LEFT,
	ALIGN_RIGHT
};

namespace d3d
{
	struct d3d_vertex_t {
		d3d_vertex_t(float x, float y, float z, color_t color) :
			m_x(x), m_y(y), m_z(z),
			m_clr(D3DCOLOR_RGBA(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha())) { };

		d3d_vertex_t() : m_x(0.f), m_y(0.f), m_z(0.f),
			m_clr(0) { };

		float m_x;
		float m_y;
		float m_z;
		float m_rhw = 1.f;
		D3DCOLOR m_clr;
	};

	constexpr size_t VERTEX_SIZE = sizeof(d3d_vertex_t);

	struct d3d_fonts_t {
		void release();
		void create(IDirect3DDevice9* device);

		ID3DXFont* f_12;
		ID3DXFont* f_esp_small;
		ID3DXFont* f_16;
		ID3DXFont* f_18;
		ID3DXFont* f_menu;
		ID3DXFont* f_con;
	};

	class c_renderer {
	private:
		IDirect3DDevice9* m_device;
		IDirect3DStateBlock9* m_block;
		IDirect3DTexture9* m_chams;
		IDirect3DTexture9* m_sil_txt;
		IDirect3DVertexBuffer9* m_buffer;
		IDirect3DSurface9* m_surface;
	public:
		ulong_t					m_width;
		ulong_t					m_height;

		c_renderer() { };
		c_renderer(IDirect3DDevice9* device);
		~c_renderer();

		void on_device_lost();
		void on_device_reset();
		auto get_device() {
			return m_device;
		}

		bool run_frame(IDirect3DDevice9* device);
		void begin();
		void end();

		void draw_line(color_t color, int x0, int y0, int x1, int y1);
		void draw_rect(color_t color, int x, int y, int w, int h);
		void draw_filled_rect(color_t color, int x, int y, int w, int h);
		void draw_circle(color_t color, int x, int y, int r, int steps = 48);
		void draw_filled_circle(color_t color, int x, int y, int r, int steps = 48);
		void draw_gradient(color_t start, color_t end, int x, int y, int w, int h, GradientType_t type);

		void draw_text(ID3DXFont* font, color_t color, int x, int y, FontAlign_t align, long font_flags, const char* msg);

		template < FontAlign_t align = ALIGN_CENTER >
		void draw_text(ID3DXFont* font, color_t color, int x, int y, long font_flags, const char* msg, ...) {
			char* buffer = (char*)_alloca(2048);
			va_list list{ };

			memset(buffer, 0, 2048);

			__crt_va_start(list, msg);
			vsprintf_s(buffer, 2048, msg, list);
			__crt_va_end(list);

			draw_text(font, color, x, y, align, font_flags, buffer);
		}

		int get_text_width(ID3DXFont* font, long font_flags, const char* msg, ...);
		int get_text_height(ID3DXFont* font, long font_flags, const char* msg, ...);

	private:
		void invalidate_objects();
		void create_objects();
	};

	extern d3d::d3d_fonts_t fonts;
}

extern d3d::c_renderer g_d3d;













namespace d3d
{
	class c_sprite;

	extern std::vector< c_sprite* > sprites;

	class c_sprite {
	public:
		size_t m_width{ };
		size_t m_height{ };

		IDirect3DDevice9* m_device{ };
		ID3DXSprite* m_sprite{ };
		IDirect3DTexture9* m_texture{ };
		const BYTE* m_image{ };
		size_t			   m_image_size{ };


	public:
		c_sprite() {
			sprites.push_back(this);
		}

		~c_sprite() {
			on_reset();
		}

		void init(IDirect3DDevice9* device, const BYTE* file, size_t img_size, size_t width, size_t height);
		

		void begin(IDirect3DDevice9* device);

		void end();

		void on_reset();

		void on_reset_end();

		void draw(int x, int y, color_t color);
	};
}

namespace icons
{
	extern d3d::c_sprite sprite_legit;
	extern d3d::c_sprite sprite_rage;
	extern d3d::c_sprite sprite_visuals;
	extern d3d::c_sprite sprite_misc;
	extern d3d::c_sprite sprite_config;
}



namespace ui
{
	extern void setup_sprites(IDirect3DDevice9* device);
	extern float anim_time;
	extern float m_frametime;
	extern color_t ui_get_accent_col();
	extern void set_animtime(float animtime);
	extern color_t& ui_get_disabled_col();
	extern color_t& ui_get_bg_col();
	extern color_t& ui_get_text_col();
	extern float ui_get_frametime();
	extern void ui_get_cursor_pos(int& x, int& y);
	extern void ui_draw_gradient(int x, int y, int w, int h, color_t start,
		color_t end, GradientType_t type = GRADIENT_HORIZONTAL);

	extern void ui_draw_line(int x, int y, int x1, int y1, color_t color);

	extern void ui_draw_rect(int x, int y, int w, int h, color_t color);


	extern void ui_draw_outlined_rect(int x, int y, int w, int h, color_t color);


	extern void ui_draw_cursor();


	extern void ui_get_text_size(int& w, int& h, const char* text, ...);

	extern void ui_draw_outlined_rect(int x, int y, int w, int h, color_t color);


	extern void ui_draw_circle(int x, int y, int r, color_t color, int res = 48);
	extern void ui_draw_filled_circle(int x, int y, int r, color_t color, int res = 48);


	extern void ui_draw_string(int x, int y, bool center, color_t color, const char* str, ...);
}