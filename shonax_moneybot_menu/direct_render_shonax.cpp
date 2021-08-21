#include "direct_render_shonax.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "input_state_shonax.h"


d3d::c_renderer g_d3d;
d3d::d3d_fonts_t d3d::fonts;



namespace d3d
{
	void d3d_fonts_t::release() {
		if (f_12) f_12->Release();
		if (f_esp_small) f_esp_small->Release();
		if (f_16) f_16->Release();
		if (f_18) f_18->Release();
		if (f_menu) f_menu->Release();
		if (f_con) f_con->Release();

		f_12 = f_esp_small = f_16 = f_18 = f_menu = f_con = nullptr;
	}

	void d3d_fonts_t::create(IDirect3DDevice9* device) {
		auto create_font = [&](ID3DXFont** font, const char* font_name, int width, int size, int weight) {
			
			auto code = D3DXCreateFontA(device, size, width, weight, 0, false, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, font_name, font);

			if (code < 0) throw ("fuck d3d");
		};

		create_font(&f_12,
#ifndef COMIC_SANS
			("Tahoma")
#else
			xors("Comic Sans MS")
#endif
			, 0, 12, 0); //change this idc
		create_font(&f_esp_small, ("Tahoma"), 0, 11, 500);
		create_font(&f_16, ("Verdana"), 0, 16, 0);
		create_font(&f_18, ("Verdana"), 0, 18, 900);
		create_font(&f_menu,
#ifndef COMIC_SANS
			("Tahoma")
#else
			xors("Comic Sans MS")
#endif
			, 0, 12,
#ifndef COMIC_SANS
			300
#else
			600
#endif
		);
		create_font(&f_con, ("Consolas"), 6, 12, 300);
	}


	c_renderer::c_renderer(IDirect3DDevice9* device) : m_device(device) {
		create_objects();
	}

	bool c_renderer::run_frame(IDirect3DDevice9* device) {
		if (!m_device) {
			m_device = device;
			create_objects();
			return false;
		}

		return true;
	}

	c_renderer::~c_renderer() {
		if (!m_device)
			return;

		invalidate_objects();
	}

	void c_renderer::on_device_lost() {
		if (!m_device)
			return;

		invalidate_objects();
	}

	void c_renderer::on_device_reset() {
		if (!m_device)
			return;

		create_objects();
	}

	void c_renderer::invalidate_objects() {
		if (m_block) m_block->Release();
		fonts.release();
	}

	void c_renderer::create_objects() {
		D3DVIEWPORT9 viewport;

		if (!m_device) return;

		if (m_device->GetViewport(&viewport) < 0) {
			return;
		}

		if (m_device->CreateStateBlock(D3DSBT_ALL, &m_block) < 0) {
			return;
		}

		if (!m_block) {
			return;
		}

		// get display size.
		m_width = viewport.Width;
		m_height = viewport.Height;

		fonts.create(m_device);

	}

	void c_renderer::begin() {
		if (!m_device) return;

		D3DVIEWPORT9 vp{ 0, 0, m_width, m_height, 0.f, 1.f };

		m_device->SetViewport(&vp);

		//m_sil_txt->GetSurfaceLevel( 0, &m_surface );

		m_block->Capture();

		// set vertex stream declaration.
		m_device->SetVertexShader(nullptr);
		m_device->SetPixelShader(nullptr);
		m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

		m_device->SetRenderState(D3DRS_LIGHTING, false);
		m_device->SetRenderState(D3DRS_FOGENABLE, false);
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
		m_device->SetRenderState(D3DRS_ZWRITEENABLE, false);
		m_device->SetRenderState(D3DRS_STENCILENABLE, false);

		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);
		m_device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);

		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		m_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);

		m_device->SetTexture(0, nullptr);
		m_device->SetTexture(1, nullptr);
		m_device->SetTexture(2, nullptr);
		m_device->SetTexture(3, nullptr);

		m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		m_device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		m_device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	
	}

	void c_renderer::end() {
		
		m_block->Apply();

	}

	void c_renderer::draw_line(color_t color, int x0, int y0, int x1, int y1) {
		d3d_vertex_t v[2] = {
			d3d_vertex_t(float(x0), float(y0), 1.0f, color), //because fuck you thats why
			d3d_vertex_t(float(x1), float(y1), 1.0f, color)
		}; //edit: do we wanna use z for shit? i mean we could for like menu stuff
		   //so it renders above other stuff

		m_device->DrawPrimitiveUP(D3DPT_LINELIST, 1, v, VERTEX_SIZE);
	}

	void c_renderer::draw_rect(color_t color, int x, int y, int w, int h) {
		d3d_vertex_t v[5] = {
			d3d_vertex_t(float(x), float(y), 1.0f, color),
			d3d_vertex_t(float(x + w), float(y), 1.0f, color),
			d3d_vertex_t(float(x + w), float(y + h), 1.0f, color),
			d3d_vertex_t(float(x), float(y + h), 1.0f, color),
			d3d_vertex_t(float(x), float(y), 1.0f, color)
		};

		m_device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, v, VERTEX_SIZE);
	}

	void c_renderer::draw_filled_rect(color_t color, int x, int y, int w, int h) {
		d3d_vertex_t v[6] = {
			d3d_vertex_t(float(x + w), float(y), 1.0f, color),
			d3d_vertex_t(float(x), float(y + h), 1.0f, color),
			d3d_vertex_t(float(x + w), float(y + h), 1.0f, color),
			d3d_vertex_t(float(x), float(y), 1.0f, color),
			d3d_vertex_t(float(x), float(y + h), 1.0f, color),
			d3d_vertex_t(float(x + w), float(y), 1.0f, color)
		};

		m_device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, v, VERTEX_SIZE);
	}

	void c_renderer::draw_gradient(color_t start, color_t end, int x, int y, int w, int h, GradientType_t type) {
		d3d_vertex_t v[4];

		switch (type) {
		case GRADIENT_VERTICAL:
			v[0] = { float(x), float(y), 1.0f, start };
			v[1] = { float(x + w), float(y), 1.0f, start };
			v[2] = { float(x), float(y + h), 1.0f, end };
			v[3] = { float(x + w), float(y + h), 1.0f, end };
			break;
		case GRADIENT_HORIZONTAL:
			v[0] = { float(x), float(y), 1.0f, start };
			v[1] = { float(x + w), float(y), 1.0f, end };
			v[2] = { float(x), float(y + h), 1.0f, start };
			v[3] = { float(x + w), float(y + h), 1.0f, end };
			break;
		}

		//m_device->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, true );
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &v, VERTEX_SIZE);
		//m_device->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, false );
	}

	void c_renderer::draw_circle(color_t color, int x, int y, int r, int res) {
		constexpr float PI = 3.1415926f;
		const float step = PI * 2.0f / float(res);

		int point_x = x + r,
			point_y = y - r,
			point_x_o{ },
			point_y_o{ };

		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
		for (int i{ }; i <= res; i++) {
			float theta = float(i) * step;

			point_x = x + (int)(r * cos(theta));
			point_y = y - (int)(r * sin(theta));
			if (i) draw_line(color, point_x, point_y, point_x_o, point_y_o);
			point_x_o = point_x;
			point_y_o = point_y;
		}
		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);
	}

	void c_renderer::draw_filled_circle(color_t color, int x, int y, int r, int res) {
		d3d_vertex_t* v = (d3d_vertex_t*)_alloca(VERTEX_SIZE * res);
		const float step = 3.14159265358979323846f * 2.0f / res;

		for (size_t i{ }; i < res; ++i) {
			float theta = i * step;
			float x_off = r * cos(theta);
			float y_off = r * sin(theta);

			v[i] = { float(x + x_off), float(y + y_off), 1.0f, color };
		}

		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, res, v, VERTEX_SIZE);
		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);
	}

	void c_renderer::draw_text(ID3DXFont* font, color_t color,
		int x, int y, FontAlign_t align, long font_flags, const char* msg) {
		if (!msg) return;
		if (!font) return;

		auto d3d_black = D3DCOLOR_RGBA(0, 0, 0, color.get_alpha());
		auto d3d_color = D3DCOLOR_RGBA(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
		auto buf = msg;

		if (align == ALIGN_CENTER) x -= get_text_width(font, font_flags, msg) / 2;
		if (align == ALIGN_RIGHT) x -= get_text_width(font, font_flags, msg);
		RECT rect{ x, y, 1000, 100 };

		ulong_t flags = DT_NOCLIP | DT_LEFT | DT_TOP;

		if (font_flags & D3DFONTFLAG_DROPSHADOW) {
			RECT r{ rect };
			r.left++;
			r.top++;
			font->DrawTextA(0, buf, -1, &r, flags, d3d_black);
		}

		if (font_flags & D3DFONTFLAG_OUTLINE) {
			for (int i = -1; i < 2; i++) {
				if (!i) continue;
				RECT r{ rect };
				r.left += i;
				r.top += i;
				font->DrawTextA(0, buf, -1, &r, flags, d3d_black);
			}
		}

		font->DrawTextA(0, buf, -1, &rect, flags, d3d_color);
	}

	int c_renderer::get_text_width(ID3DXFont* font, long flags, const char* msg, ...) {
		char* buffer = (char*)_alloca(2048);
		va_list list{ };

		memset(buffer, 0, 2048);

		__crt_va_start(list, msg);
		vsprintf_s(buffer, 2048, msg, list);
		__crt_va_end(list);

		RECT temp{ };
		font->DrawTextA(0, buffer, -1, &temp, DT_CALCRECT, 0x0);

		return (temp.right - temp.left);
	}

	int c_renderer::get_text_height(ID3DXFont* font, long flags, const char* msg, ...) {
		char* buffer = (char*)_alloca(2048);
		va_list list{ };

		memset(buffer, 0, 2048);

		__crt_va_start(list, msg);
		vsprintf_s(buffer, 2048, msg, list);
		__crt_va_end(list);

		RECT temp{ };
		font->DrawTextA(0, buffer, -1, &temp, DT_CALCRECT, 0x0);

		return (temp.bottom - temp.top);
	}
}





namespace d3d
{






	void c_sprite::init(IDirect3DDevice9* device, const BYTE* file, size_t img_size, size_t width, size_t height)
	{
		m_width = width;
		m_height = height;

		m_device = device;
		m_image = file;
		m_image_size = img_size;
	}


	void c_sprite::begin(IDirect3DDevice9* device) {
		m_device = device;

		if (!m_device) {
			return;
		}

		if (!m_sprite)
			D3DXCreateSprite(m_device, &m_sprite);

		if (!m_texture) {
			auto hr = D3DXCreateTextureFromFileInMemoryEx(
				m_device, m_image, m_image_size,
				m_width, m_height, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_texture);
		}

		if (m_sprite)
			m_sprite->Begin(D3DXSPRITE_DONOTMODIFY_RENDERSTATE);
	}

	void c_sprite::end() {
		if (!m_device || !m_sprite || !m_texture) return;
		m_sprite->End();
	}

	void c_sprite::on_reset() {
		if (m_sprite && m_device && m_texture) {
			m_sprite->OnLostDevice();
			m_texture->Release();
			m_texture = nullptr;
		}
	}

	void c_sprite::on_reset_end() {
		if (m_sprite && m_device)
			m_sprite->OnResetDevice();
	}

	void c_sprite::draw(int x, int y, color_t color) {
		if (!m_device || !m_texture || !m_sprite) {
			return;
		}

		const float scale = 0.8f;

		ulong_t
			fill_mode,
			adaptive_tess,
			dest_blend,
			cull,
			stencil,
			vertex_blend,
			alpha_test;

		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_FILLMODE, &fill_mode);
		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_ENABLEADAPTIVETESSELLATION, &adaptive_tess);
		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, &dest_blend);
		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, &cull);
		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_STENCILENABLE, &stencil);
		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_VERTEXBLEND, &vertex_blend);
		m_device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, &alpha_test);

		ulong_t mag_filter;
		ulong_t min_filter;
		m_device->GetSamplerState(0, D3DSAMPLERSTATETYPE::D3DSAMP_MAGFILTER, &mag_filter);
		m_device->GetSamplerState(0, D3DSAMPLERSTATETYPE::D3DSAMP_MINFILTER, &min_filter);

		m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);

		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FILLMODE, D3DFILL_SOLID);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ENABLEADAPTIVETESSELLATION, false);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL_NONE);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_STENCILENABLE, false);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_VERTEXBLEND, false);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, D3DLINECAPS_ALPHACMP);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_MULTISAMPLEANTIALIAS, D3DMULTISAMPLE_4_SAMPLES);


		ulong_t hr;
		D3DXVECTOR2 center = D3DXVECTOR2((m_width * scale) * 0.5f, (m_height * scale) * 0.5f);
		D3DXVECTOR2 trans = D3DXVECTOR2(x - center.x, y - center.y);
		D3DXMATRIX matrix;
		D3DXVECTOR2 scale_vec(scale, scale);
		D3DXMatrixTransformation2D(&matrix, 0, 0.f, &scale_vec, &center, 0.f, &trans);

		hr = m_sprite->SetTransform(&matrix);

		auto d3dcolor = D3DCOLOR_RGBA(color.get_red(),
			color.get_green(), color.get_blue(), color.get_alpha());
		hr = m_sprite->Draw(m_texture, 0, 0, 0, d3dcolor);

		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_MULTISAMPLEANTIALIAS, false);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FILLMODE, fill_mode);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ENABLEADAPTIVETESSELLATION, adaptive_tess);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, dest_blend);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, cull);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_STENCILENABLE, stencil);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_VERTEXBLEND, vertex_blend);
		m_device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, alpha_test);

		m_device->SetSamplerState(0, D3DSAMPLERSTATETYPE::D3DSAMP_MAGFILTER, mag_filter);
	}
}


std::vector< d3d::c_sprite* > d3d::sprites;

namespace icons
{
	d3d::c_sprite sprite_legit;
	d3d::c_sprite sprite_rage;
	d3d::c_sprite sprite_visuals;
	d3d::c_sprite sprite_misc;
	d3d::c_sprite sprite_config;
}






namespace ui
{
	float anim_time;
	float m_frametime;
	void setup_sprites(IDirect3DDevice9* device) {
		//fuck msvc
		icons::sprite_legit.init(device, icons::legit_icon, icons::legit_size, 66, 66);
		icons::sprite_rage.init(device, icons::rage_icon, icons::rage_size, 66, 66);
		icons::sprite_visuals.init(device, icons::raw::visuals, icons::visuals_size, 66, 66);
		icons::sprite_misc.init(device, icons::misc_icon, icons::misc_size, 66, 66);
		icons::sprite_config.init(device, icons::config_icon, icons::config_size, 66, 66);
	}
	void set_animtime(float animtime) {
		anim_time = animtime;
	}
	color_t ui_get_accent_col() {
		//con_var< uint8_t > menu_color_1r{ &holder_, fnv( "menu_color_1r" ), 231 };
		//con_var< uint8_t > menu_color_1g{ &holder_, fnv("menu_color_1g"), 105 };
		//con_var< uint8_t > menu_color_1b{ &holder_, fnv("menu_color_1b"), 105 };
		//
		//con_var< uint8_t > menu_color_2r{ &holder_, fnv("menu_color_2r"), 0xf4 };
		//con_var< uint8_t > menu_color_2g{ &holder_, fnv("menu_color_2g"), 0x7c };
		//con_var< uint8_t > menu_color_2b{ &holder_, fnv("menu_color_2b"), 0xa8 };


		color_t col_start = {
			231,
			105,
			105
		};

		color_t col_end = {
			0xf4,
			0x7c,
			0xa8
		};

		//color_t col_start = {
		//	vars.menu.color.get_red(),
		//	vars.menu.color.get_green(),
		//	vars.menu.color.get_blue()
		//};
		//
		//color_t col_end = {
		//	vars.menu.color.get_red(),
		//	vars.menu.color.get_green(),
		//	vars.menu.color.get_blue()
		//};

		color_t col = color_t::blend(col_start, col_end, anim_time);

		return col;
	}

	color_t& ui_get_disabled_col() {
		static color_t col = color_t(61, 61, 61, 255);
		return col;
	}

	color_t& ui_get_bg_col() {
		static color_t col = color_t(24, 25, 27, 255);
		return col;
	}

	color_t& ui_get_text_col() {
		static color_t col = color_t(221, 221, 221, 255);
		return col;
	}

	void ui_draw_gradient(int x, int y, int w, int h, color_t start,
		color_t end, GradientType_t type) {

		g_d3d.draw_gradient(start, end, x, y, w, h, type);
	}

	void ui_draw_line(int x, int y, int x1, int y1, color_t color) {
		g_d3d.draw_line(color, x, y, x1, y1);
	}

	void ui_draw_rect(int x, int y, int w, int h, color_t color) {
		g_d3d.draw_filled_rect(color, x, y, w, h);
	}

	void ui_draw_outlined_rect(int x, int y, int w, int h, color_t color) {
		g_d3d.draw_rect(color, x, y, w, h);
	}

	void ui_draw_circle(int x, int y, int r, color_t color, int res) {
		g_d3d.draw_circle(color, x, y, r, res);
	}

	void ui_draw_filled_circle(int x, int y, int r, color_t color, int res) {
		g_d3d.draw_filled_circle(color, x, y, r, res);
	}

	void ui_draw_string(int x, int y, bool center, color_t color, const char* str, ...) {
		char buf[2048]{ };
		va_list list{ };

		__crt_va_start(list, str);
		vsprintf_s(buf, 2048, str, list);
		__crt_va_end(list);

		g_d3d.draw_text(d3d::fonts.f_menu, color, x, y,
			center ? ALIGN_CENTER : ALIGN_LEFT, D3DFONTFLAG_DROPSHADOW, buf);
	}

	void ui_get_text_size(int& w, int& h, const char* text, ...) {
		char* buf = (char*)_alloca(2048);
		va_list list{ };

		__crt_va_start(list, text);
		vsprintf_s(buf, 2048, text, list);
		__crt_va_end(list);

		w = g_d3d.get_text_width(d3d::fonts.f_menu, 0, buf);
		h = g_d3d.get_text_height(d3d::fonts.f_menu, 0, buf);
	}

	void ui_get_cursor_pos(int& x, int& y) {
		//auto fx = ImGui::GetMousePos();
		//x = fx.x;
		//y = fx.y;
		g_input.get_cursor_pos(x, y);
	}

	float ui_get_frametime() {
		return m_frametime;
	}

	void ui_draw_cursor() {
		const color_t black(0, 0, 0, 255), accent(ui_get_accent_col());
		int x, y;
		ui_get_cursor_pos(x, y);


		for (int i{ }; i <= 9; ++i) {
			ui_draw_line(x, y, x + i, y + 11, accent);
		}

		for (int i{ }; i <= 7; ++i) {
			ui_draw_line(x, y + 9 + i, x + i, y + 9, accent);
		}

		for (int i{ }; i <= 3; ++i) {
			ui_draw_line(x + 6 + i, y + 11, x, y + i, accent);
		}

		ui_draw_line(x + 5, y + 11, x + 8, y + 18, accent);
		ui_draw_line(x + 4, y + 11, x + 7, y + 18, accent);

		ui_draw_line(x, y, x, y + 17, black);
		ui_draw_line(x, y + 17, x + 3, y + 14, black);
		ui_draw_line(x + 4, y + 14, x + 7, y + 19, black);
		ui_draw_line(x + 7, y + 18, x + 9, y + 18, black);
		ui_draw_line(x + 10, y + 18, x + 7, y + 12, black);
		ui_draw_line(x + 7, y + 12, x + 11, y + 12, black);
		ui_draw_line(x + 11, y + 12, x, y, black);
	}
}