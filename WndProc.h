#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_impl_win32.h"
#include "interfaces.h"
#include "shonax_moneybot_menu/input_state_shonax.h"



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int wheelDelta = 0;
	
	const auto getButtonToggle = [uMsg, wParam](bool& bButton, int vKey)
	{
		if (wParam != vKey) return;
	
		if (uMsg == WM_KEYUP)
			bButton = !bButton;
	};
	
	if (uMsg == WM_MOUSEWHEEL) {
		wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
		for (; wheelDelta > WHEEL_DELTA; wheelDelta -= WHEEL_DELTA)
			csgo->scroll_amount += 1;
		for (; wheelDelta < 0; wheelDelta += WHEEL_DELTA)
			csgo->scroll_amount -= 1;
	}
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		csgo->key_pressed[VK_LBUTTON] = true;
		break;
	case WM_LBUTTONUP:
		csgo->key_pressed[VK_LBUTTON] = false;
		break;
	case WM_RBUTTONDOWN:
		csgo->key_pressed[VK_RBUTTON] = true;
		break;
	case WM_RBUTTONUP:
		csgo->key_pressed[VK_RBUTTON] = false;
		break;
	case WM_MBUTTONDOWN:
		csgo->key_pressed[VK_MBUTTON] = true;
		break;
	case WM_MBUTTONUP:
		csgo->key_pressed[VK_MBUTTON] = false;
		break;
	case WM_MOUSEMOVE:
		break;
	case WM_XBUTTONDOWN:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
		{
			csgo->key_pressed[VK_XBUTTON1] = true;
		}
		else if (button == XBUTTON2)
		{
			csgo->key_pressed[VK_XBUTTON2] = true;
		}
		break;
	}
	case WM_XBUTTONUP:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
		{
			csgo->key_pressed[VK_XBUTTON1] = false;
		}
		else if (button == XBUTTON2)
		{
			csgo->key_pressed[VK_XBUTTON2] = false;
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		break;
	}
	case WM_SYSKEYDOWN:
		csgo->key_pressed[18] = true;
		break;
	case WM_SYSKEYUP:
		csgo->key_pressed[18] = false;
		break;
	case WM_KEYDOWN:
		csgo->key_pressed[wParam] = true;
		break;
	case WM_KEYUP:
		csgo->key_pressed[wParam] = false;
		break;
	default: break;
	}
	
	//window->update_keystates();
	
	for (int i = 0; i < 256; i++) {
		if (csgo->key_pressed[i])
			csgo->key_down[i] = csgo->key_pressedticks[i]++ == 0;
		else
			csgo->key_pressedticks[i] = 0;
	}

	//getButtonToggle(vars.menu.open, VK_INSERT);

	//if (vars.menu.open) {
	//	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	//
	//	if (ImGui::GetIO().WantTextInput)
	//		return true;
	//}

	//KEYS_HOME
	//KEYS_INSERT

	if (uMsg == WM_ACTIVATE && wParam == WA_INACTIVE) {
		g_input.clear_keys();
	}

	if (uMsg == WM_MOUSEMOVE) {
		g_input.capture_mouse_move(lParam);
		//g_con->get_input()->capture_mouse_move(lparam);
	}

	static auto con_pressed = false;
	if (g_input.is_key_pressed(KEYS_INSERT)) {
		if (!con_pressed) {
			vars.menu.open ^= 1;

			//ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			//
			//if (ImGui::GetIO().WantTextInput)
			//	return true;

		}

		con_pressed = true;
	}
	else con_pressed = false;

	if (g_input.register_key_press(VirtualKeyEvents_t(uMsg), VirtualKeys_t(wParam))) {
		if (vars.menu.open) return false;
	}

	if (!vars.menu.open) {
		static bool was_pressed = false;
		if (g_input.is_key_pressed(KEYS_HOME)) {
			if (!was_pressed) {
				vars.menu.open ^= 1;

				was_pressed = true;
			}
		}
		else {
			was_pressed = false;
		}

		if (g_input.register_key_press(VirtualKeyEvents_t(uMsg), VirtualKeys_t(wParam))
			&& vars.menu.open) {
			return false;
		}
	}




	return CallWindowProc(csgo->Init.OldWindow, hWnd, uMsg, wParam, lParam);
}
