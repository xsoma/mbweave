#include "js_loader.h"
#include "js_handler.h"
#include "../Hooks.h"
#include "../GUI/gui.h"
#include "cache.h"

c_js_loader js_loader;

void read_scripts(LPCTSTR lpszFileName)
{
	js_loader.scripts_list.push_back(lpszFileName);
}

string c_js_loader::get_module_file_path(HMODULE hModule)
{
	string ModuleName = str("");
	char szFileName[MAX_PATH] = { 0 };

	if (GetModuleFileNameA(hModule, szFileName, MAX_PATH))
		ModuleName = szFileName;

	return ModuleName;
}

string c_js_loader::get_module_base_dir(HMODULE hModule)
{
	string ModulePath = get_module_file_path(hModule);
	return ModulePath.substr(0, ModulePath.find_last_of(str("\\/")));
}

void c_js_loader::remove(string cfg_name) {

	static TCHAR path[256];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + str("\\weave\\scripts\\");
		file = std::string(path) + str("\\weave\\scripts\\") + cfg_name;
	}

	remove(file.c_str());
}

std::vector<uint8_t> read_script_from_file(char const* filename)
{
	std::vector<uint8_t> result;

	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	if (ifs)
	{
		std::ifstream::pos_type pos = ifs.tellg();
		result.resize(pos);

		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)&result[0], pos);
	}

	return result;
}

void c_js_loader::unload(std::string script_name)
{
	run_callback("unload");
	csgo->mtx.lock();

	const auto& script = str(R"(
		for(var cb_type in Callbacks)
			delete Callbacks[cb_type][')") + script_name + str(R"('];
	)");
	if (duk_peval_string(js_handler.ctx, script.c_str()) != 0) {
		Msg(str("JavaScript: Unload error: ") + std::string(duk_safe_to_string(js_handler.ctx, -1)), color_t(255, 35, 35, 255));
		csgo->mtx.unlock();
		return;
	}

	loaded_scripts[script_name] = false;

	auto& script_cache = scripts_cache[script_name];
	for (auto& image : script_cache.images)
		csgo->outdated_textures[image.first] = image.second;
	scripts_cache.erase(script_name);

	auto& menu_elements = js_handler.menu_elements[script_name];
	for (auto el : menu_elements)
	{
		switch (el.second->p_element->type)
		{
		case c_elementtype::checkbox:
			delete (c_checkbox*)el.second->p_element;
			break;
		case c_elementtype::slider:
			delete (c_slider*)el.second->p_element;
			break;
		}
		delete el.second;
	}
	js_handler.menu_elements.erase(script_name);

	csgo->mtx.unlock();
}

static __forceinline void merge_lines(std::string& str)
{
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

void c_js_loader::load(std::string script_name) {
	csgo->mtx.lock();
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + str("\\weave\\scripts\\");
		file = std::string(path) + str("\\weave\\scripts\\") + script_name;
	}

	std::string prefix_script = str(R"(
		(function(){
			const script_name = ')") + script_name + str(R"(',
			
			register_callback = _register_callback.bind(null, script_name);

			for(var cb_type in Callbacks)
				Callbacks[cb_type][script_name] = [];

			ui.add_checkbox = ui._add_checkbox.bind(ui, script_name);
			ui.add_slider = ui._add_slider.bind(ui, script_name);

			vars.get_int = vars._get_int.bind(vars, script_name);
			vars.set_int = vars._set_int.bind(vars, script_name);
			vars.get_uint = vars._get_uint.bind(vars, script_name);
			vars.set_uint = vars._set_uint.bind(vars, script_name);
			vars.get_bool = vars._get_bool.bind(vars, script_name);
			vars.set_bool = vars._set_bool.bind(vars, script_name);

			render.picture = render._picture.bind(render, script_name);
	)"), suffix_script = str(R"(
		})();
	)");

	suffix_script.insert(suffix_script.begin(), '\n');

	const auto& script_raw = read_script_from_file(file.c_str());

	const auto& script = prefix_script + std::string(script_raw.begin(), script_raw.end()) + suffix_script;
	if (duk_peval_string(js_handler.ctx, script.c_str()) != 0) {
		Msg(str("JavaScript: Load error: ") + std::string(duk_safe_to_string(js_handler.ctx, -1)), color_t(255, 35, 35, 255));
		csgo->mtx.unlock();
		return;
	}
	loaded_scripts[script_name] = true;
	scripts_cache[script_name] = script_cache_data();
	csgo->mtx.unlock();
}

void c_js_loader::refresh_scripts()
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		scripts_list.clear();
		string ConfigDir = std::string(path) + str("\\weave\\scripts\\");
		CreateDirectory(ConfigDir.c_str(), 0);
		SearchFiles((ConfigDir + str("*.js")).c_str(), read_scripts, FALSE);
	}
}