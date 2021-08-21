#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "Hooks.h"

enum tab_t : short {
	undefined = -1,
	legit,
	rage,
	antiaims,
	esp,
	misc,
	world,
	skins,
	javascript,
};

class c_window;
class c_child;
class CMenu
{
public:
	void draw_indicators();
	c_child* weapon_cfg = nullptr;
	c_window* window = nullptr;
	bool initialized = false;
	void render();
	void update_binds();


	uintptr_t get_path(uintptr_t window, int tab, std::string subtab, int subtab_selected_item) {

	}

	bool add_checkbox(uintptr_t path, std::string name) {

	}

	// ...

	uintptr_t find_element(uintptr_t path, std::string element_name) {

	}
};

extern float g_Menucolor_t[4];
