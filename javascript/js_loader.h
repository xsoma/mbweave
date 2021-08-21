#include "../DLL_MAIN.h"
#include <mutex>

class c_js_loader {
private:
	string get_module_file_path(HMODULE hModule);
	string get_module_base_dir(HMODULE hModule);
public:
	vector<std::string> scripts_list;
	std::map<std::string, bool> loaded_scripts;
	void unload(string);
	void load(string);
	void remove(string);

	void refresh_scripts();
};
typedef void(*LPSEARCHFUNC)(LPCTSTR lpszFileName);
extern BOOL SearchFiles(LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders = FALSE);
extern c_js_loader js_loader;