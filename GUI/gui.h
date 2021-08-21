#pragma once
#include "window.h"
#include "child.h"
#include "tab.h"
#include "colorpicker.h"
#include "multicombo.h"
#include "keybind.h"
#include "input_text.h"
#include "listbox.h"
#include "dynamic_text.h"
#include "ragebot_subtabs.h"
#include "tab_selector.h"
#include "separator.h"

class c_gui {
private:
	std::vector<c_window*> windows;
public:
	c_style style;
	void add_window(c_window* window);
	void render();
};

extern c_gui g_gui;