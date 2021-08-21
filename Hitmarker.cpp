#include "Hitmarker.h"

void CHitMarker::Paint()
{
	if (!csgo->is_connected || !csgo->is_local_alive)
		return;

	const int size = 8;
	const int gap = 3;
	const float thickness = 1.f;
	const int padding = 4;
	const float expire_time = 0.75f;

	auto r = vars.visuals.hitmarker_color[0],
		g = vars.visuals.hitmarker_color[1],
		b = vars.visuals.hitmarker_color[2];

	// draw

	float w = csgo->w;
	float h = csgo->h;
	w /= 2.f;
	h /= 2.f;

	auto easeOutQuad = [](float x) {
		return 1 - (1 - x) * (1 - x);
	};

	if (vars.visuals.hitmarker & 1)
	{
		float current_alpha = (interfaces.global_vars->realtime - csgo->last_hit_time) / expire_time;
		current_alpha = clamp(current_alpha, 0.f, 1.f);

		if (current_alpha == 1.f)
			csgo->last_hit_time = 0.f;

		float alpha = 255.f * easeOutQuad(current_alpha);

		g_Render->DrawLine(w - padding, h - padding, w - padding - size, h - padding - size, color_t(r, g, b, 255 - alpha), thickness);
		g_Render->DrawLine(w + padding, h + padding, w + padding + size, h + padding + size, color_t(r, g, b, 255 - alpha), thickness);
		g_Render->DrawLine(w + padding, h - padding, w + padding + size, h - padding - size, color_t(r, g, b, 255 - alpha), thickness);
		g_Render->DrawLine(w - padding, h + padding, w - padding - size, h + padding + size, color_t(r, g, b, 255 - alpha), thickness);
	}

	if (!hitmarkers.empty()) {
		for (const auto& hitmarker : hitmarkers)
		{
			Vector pos3D = Vector(hitmarker.position.x, hitmarker.position.y, hitmarker.position.z), pos2D;
			if (!Math::WorldToScreen(pos3D, pos2D))
				continue;

			if (vars.visuals.hitmarker & 2)
			{
				g_Render->DrawLine(pos2D.x - padding, pos2D.y - padding, pos2D.x - padding - size, pos2D.y - padding - size, color_t(r, g, b, hitmarker.alpha), thickness);
				g_Render->DrawLine(pos2D.x + padding, pos2D.y + padding, pos2D.x + padding + size, pos2D.y + padding + size, color_t(r, g, b, hitmarker.alpha), thickness);
				g_Render->DrawLine(pos2D.x + padding, pos2D.y - padding, pos2D.x + padding + size, pos2D.y - padding - size, color_t(r, g, b, hitmarker.alpha), thickness);
				g_Render->DrawLine(pos2D.x - padding, pos2D.y + padding, pos2D.x - padding - size, pos2D.y + padding + size, color_t(r, g, b, hitmarker.alpha), thickness);
			}

			if (vars.visuals.visualize_damage) {

				const auto& modifier = std::clamp(fabsf(csgo->get_absolute_time() - hitmarker.time), 0.f, 1.f);
				switch (hitmarker.seed % 4)
				{
				case 0:
					g_Render->DrawString(pos2D.x - 50 * modifier, pos2D.y - 25.f - 50 * modifier,
						hitmarker.hs ? color_t(255, 35, 35, (1.f - modifier) * 255) : color_t(255, 255, 255, (1.f - modifier) * 255),
						render::dropshadow | render::centered_x | render::centered_y, fonts::lby_indicator, hitmarker.hp.c_str());
					break;
				case 1:
					g_Render->DrawString(pos2D.x - 50 * modifier, pos2D.y - 25.f + 50 * modifier,
						hitmarker.hs ? color_t(255, 35, 35, (1.f - modifier) * 255) : color_t(255, 255, 255, (1.f - modifier) * 255),
						render::dropshadow | render::centered_x | render::centered_y, fonts::lby_indicator, hitmarker.hp.c_str());
					break;
				case 2:
					g_Render->DrawString(pos2D.x + 50 * modifier, pos2D.y - 25.f - 50 * modifier,
						hitmarker.hs ? color_t(255, 35, 35, (1.f - modifier) * 255) : color_t(255, 255, 255, (1.f - modifier) * 255),
						render::dropshadow | render::centered_x | render::centered_y, fonts::lby_indicator, hitmarker.hp.c_str());
					break;
				case 3:
					g_Render->DrawString(pos2D.x + 50 * modifier, pos2D.y - 25.f + 50 * modifier,
						hitmarker.hs ? color_t(255, 35, 35, (1.f - modifier) * 255) : color_t(255, 255, 255, (1.f - modifier) * 255),
						render::dropshadow | render::centered_x | render::centered_y, fonts::lby_indicator, hitmarker.hp.c_str());
					break;
				}
			}
		}
		// proceeed
		for (int i = 0; i < hitmarkers.size(); i++) {
			if (hitmarkers[i].time + 4.25f <= csgo->get_absolute_time()) {
				hitmarkers[i].alpha -= 1;
			}
			if (hitmarkers[i].alpha <= 0)
				hitmarkers.erase(hitmarkers.begin() + i);
		}
	}
}

void CHitMarker::Add(Vector hitpos, bool headshot, std::string damage)
{
	if (!(vars.visuals.hitmarker & 2))
		return;
	
	static int counter = 0;
	counter++;
	hitmarker_info info;
	info.position = hitpos;
	info.alpha = 255;
	info.time = csgo->get_absolute_time();
	info.hp = damage;
	info.hs = headshot;
	info.seed = counter;
	hitmarkers.push_back(info);
	if (counter > 3)
		counter = 0;
}