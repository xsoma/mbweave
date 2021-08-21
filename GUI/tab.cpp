#include "tab.h"
#include "child.h"
#include "../Math.h"

#define icons_size 22
void c_tab::draw(Vector2D pos, Vector2D size) {
	c_child* c = (c_child*)child;
	auto alpha = (int)(c->get_transparency() * 2.55f);
	bool hovered = g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
	if (hovered && !c->g_active_element && !c->is_holding_menu()) {
		c->g_hovered_element = this;
		if (c->is_click()) {
			c->set_active_tab_index(this->index);
			c->reset_mouse();
		}
		
		if (animation < 1.f) animation += animation_speed / 1.5f;
	}
	else {
		if (animation > 0.f) animation -= animation_speed / 1.5f;
	}

	animation = std::clamp(animation, 0.f, 1.f);
	float r_animation = sin(DEG2RAD(animation * 90.f));
	auto clr = color_t(38 + 10 * animation, 41 + 10 * animation, 54 + 10 * animation, alpha);

	/*
	
	Red: 27
	Green: 31
	Blue: 40
	
	*/

	g_Render->FilledRect(pos.x + 2, pos.y + size.y / 2 - 13, size.x - 4, 26, clr, 8.f * (1.f * animation));

	auto t_clr = c->get_active_tab_index() == this->index ? 
		main_color.manage_alpha(alpha - 25.f * r_animation):
		color_t(200 + 55.f * r_animation, 200 + 55.f * r_animation, 200 + 55.f * r_animation, alpha);
	auto base_pos = Vector2D(pos.x + size.x / 2, pos.y + size.y / 2);
	g_Render->_drawList->AddImage(this->texture, ImVec2(base_pos.x - icons_size / 2, base_pos.y - icons_size / 2),
		ImVec2(base_pos.x + icons_size / 2, base_pos.y + icons_size / 2), ImVec2(0, 0), ImVec2(1, 1), t_clr.manage_alpha(255.f - alpha + 255.f * (1.f - animation)).u32());
	g_Render->DrawString(pos.x + size.x / 2, pos.y + size.y / 2, t_clr.manage_alpha(255.f * animation),
		render::centered_x | render::centered_y, fonts::menu_main, get_title().c_str());
}

void c_tab::special_render() {

}

void c_tab::render() {

}