#pragma once
#include <list>
#include <set>
#include <string>

/**
 * Command line preprocessor settings.
 * On the command line these are configured by -D, -U, -I, --include, -std
 */
struct DUI {
	DUI() = default;
	std::string prefix = "#version 460 core\n#extension GL_ARB_bindless_texture : require\n";
	std::list<std::string> defines;
	std::set<std::string> undefined;
	std::list<std::string> includePaths;
	std::list<std::string> includes;
	bool clearIncludeCache{};
	bool removeComments{}; /** remove comment tokens from included files */
};