#pragma once
#include "Hooks.h"

void Thirdperson()
{
	if (!interfaces.engine->IsInGame() || !csgo->local)
		return;

//	static float cur_dist = vars.visuals.thirdperson_dist;

	if (!g_Binds[bind_third_person].active)
	{
		enabledtp = false;
		//cur_dist = vars.visuals.thirdperson_dist;
		return;
	}

	if (csgo->local->isAlive() && enabledtp && csgo->client_state->iDeltaTick > 0)
	{
		if (!interfaces.input->m_fCameraInThirdPerson)
			interfaces.input->m_fCameraInThirdPerson = true;

		if (interfaces.input->m_fCameraInThirdPerson)
		{
			/*if (cur_dist > 0.f)
				cur_dist -= 1.f * sin(animation_speed * 10.f);*/

			auto distance = vars.visuals.thirdperson_dist /*- cur_dist*/;

			Vector angles;
			interfaces.engine->GetViewAngles(angles);

			Vector inverse_angles;
			interfaces.engine->GetViewAngles(inverse_angles);

			inverse_angles.z = distance;

			Vector forward, right, up;
			Math::AngleVector4(inverse_angles, forward, right, up);

			Ray_t ray;
			CTraceFilterWorldAndPropsOnly filter;
			trace_t trace;

			auto eye_pos = csgo->fake_duck
				? csgo->local->GetRenderOrigin() + interfaces.game_movement->GetPlayerViewOffset(false)
				: csgo->local->GetRenderOrigin() + csgo->local->GetVecViewOffset();
			auto offset = eye_pos + forward * -distance + right + up;

			ray.Init(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f));
			interfaces.trace->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);

			trace.fraction = clamp(trace.fraction, 0.f, 1.f);

			angles.z = distance * trace.fraction;

			interfaces.input->m_vecCameraOffset = angles;
		}
		/*else
			cur_dist = vars.visuals.thirdperson_dist;*/
	}
	else {
		//cur_dist = vars.visuals.thirdperson_dist;
		interfaces.input->m_fCameraInThirdPerson = false;
		interfaces.input->m_vecCameraOffset.z = 0.f;
	}

	static auto b_once = false;

	if (csgo->local->isAlive())
	{
		b_once = false;
		return;
	}

	if (b_once) {
		csgo->local->SetObserverMode(5);
		b_once = false;
	}

	if (csgo->local->GetObserverMode() == 4)
		b_once = true;
}

void __stdcall Hooked_OverrideView(void* pSetup)
{
	static auto OverRideView = g_pClientModeHook->GetOriginal< OverRideViewFn >(g_HookIndices[fnva1(hs::Hooked_OverrideView.s().c_str())]);

	if (csgo->DoUnload)
		return 	OverRideView(pSetup);

	auto Setup = reinterpret_cast<CViewSetup*>(pSetup);

	if (csgo->is_connected) {
		float fov_addition = vars.misc.worldfov;
		fov_addition = clamp(fov_addition, 0.f, 50.f);

		if (csgo->is_local_alive) {
			if (vars.visuals.remove & 1)
				Setup->angles -= csgo->local->GetPunchAngle() * 0.9f + csgo->local->GetViewPunchAngle();

			g_GrenadePrediction->View(Setup);

			if (csgo->zoom_level > 0)
				Setup->fov = (90.f + fov_addition - (50.f * (vars.misc.zoomfov / 100.f))) / csgo->zoom_level;
			else
				Setup->fov = (90.f + fov_addition);
		}
		else
			Setup->fov = (90.f + fov_addition);

		Thirdperson();

		OverRideView(pSetup);

		if (csgo->is_local_alive && csgo->fake_duck)
		{
			Setup->origin = csgo->local->GetRenderOrigin() + Vector(0.0f, 0.0f, interfaces.game_movement->GetPlayerViewOffset(false).z + 0.064f);

			if (interfaces.input->m_fCameraInThirdPerson)
			{
				auto camera_angles = Vector(interfaces.input->m_vecCameraOffset.x, interfaces.input->m_vecCameraOffset.y, 0.0f); //-V807
				auto camera_forward = Vector();

				Math::AngleVectors(camera_angles, camera_forward);
				Math::VectorMA(Setup->origin, -interfaces.input->m_vecCameraOffset.z, camera_forward, Setup->origin);
			}
		}
	}
	else
		OverRideView(pSetup);
}

float __stdcall Hooked_ViewModel()
{
	return vars.misc.viewmodelfov == 0 ? 68 : vars.misc.viewmodelfov;
}
