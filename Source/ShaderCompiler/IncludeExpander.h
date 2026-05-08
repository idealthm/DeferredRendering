#pragma once

#include <string>
#include <vector>

/**
 * Expands #include "..." directives in GLSL source code.
 *
 * @param source       The source code containing #include directives
 * @param currentDir   Directory of the current file (for resolving relative includes)
 * @param searchPaths  Additional directories to search for included files
 * @returns            The source with all #include directives expanded inline
 *
 * Throws std::runtime_error on missing files or circular includes.
 */
std::string ExpandIncludes(const std::string& source,
                           const std::string& currentDir,
                           const std::vector<std::string>& searchPaths);

/**
 * Reads a file, strips UTF-8 BOM, normalizes line endings to \n.
 */
std::string ReadSourceFile(const std::string& filePath);
