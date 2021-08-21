#pragma once
#include "Hooks.h"
#include "GUI/gui.h"

class CMessage
{
public:
	CMessage(std::string str, float time, color_t clr)
	{
		this->str = str;
		this->time = csgo->get_absolute_time();
		this->clr = clr;
		alpha = 255.f;
	}
	std::string str;
	float time;
	color_t clr;
	float alpha;
};
extern float animation_speed;

class CEventlog
{
public:
	std::vector<CMessage> messages;
	void Draw()
	{
		constexpr float showtime = 5.f;
		constexpr float animation_time = 0.175f;

		if (messages.empty())
			return;

		while (messages.size() > 10)
			messages.erase(messages.begin());

		static const auto easeOutQuad = [](float x) {
			return 1 - (1 - x) * (1 - x);
		};

		for (int i = messages.size() - 1; i >= 0; i--)
		{
			float in_anim = std::clamp((csgo->get_absolute_time() - messages[i].time) / animation_time, 0.01f, 1.f);
			float out_anim = std::clamp(((csgo->get_absolute_time() - messages[i].time) - showtime) / animation_time, 0.f, FLT_MAX);

			messages[i].alpha = in_anim * (1.f - out_anim) * 255.f;

			if (out_anim > 1.f)
				messages[i].alpha = 0.f;
			if (in_anim > 1.f)
				messages[i].alpha = 255.f;

			in_anim = easeOutQuad(in_anim);
			out_anim = easeOutQuad(out_anim);

			if (messages[i].alpha > 0.f) {

				ImGui::PushFont(fonts::menu_desc);
				auto text_size = ImGui::CalcTextSize(messages[i].str.c_str());
				ImGui::PopFont();

				const float x_position = in_anim * 15.f - out_anim * 15.f;
				const float y_position = 15.f + (30.f * i);

				g_Render->FilledRectGradient(x_position, y_position, text_size.x + 25, 25.f,
					style.get_color(c_style::window_background).manage_alpha(messages[i].alpha * 0.7f), color_t(0, 0, 0, 0),
					color_t(0, 0, 0, 0), style.get_color(c_style::window_background).manage_alpha(messages[i].alpha * 0.7f));

				g_Render->FilledRect(x_position, y_position, 4.f, 25.f, style.get_color(c_style::accented_color).manage_alpha(messages[i].alpha));

				g_Render->DrawString(x_position + 17, y_position + 12.5f,
					messages[i].clr.manage_alpha(messages[i].alpha),
					render::centered_y, fonts::menu_desc, messages[i].str.c_str());
			}
		}

		for (int i = messages.size() - 1; i >= 0; i--) {
			if (messages[i].alpha <= 0.f) {
				messages.erase(messages.begin() + i);
				break;
			}
		}
	}
};