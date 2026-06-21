#pragma once
#include <string>
#include <vector>

namespace Provider {
	const std::vector<std::string>& GetCommonProlog();
	const std::vector<std::string>& GetDepthVertex();
	const std::vector<std::string>& GetDepthFragment();
	const std::vector<std::string>& GetGBufferVertex();
	const std::vector<std::string>& GetGBufferFragment();
	const std::vector<std::string>& GetLightingVertex();
	const std::vector<std::string>& GetLightingFragment();
	const std::vector<std::string>& GetPostProcessVertex();
	const std::vector<std::string>& GetPostProcessFragment();
}
