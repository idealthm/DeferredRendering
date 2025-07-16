#pragma once
#include <cstdarg>
#include <string>
#include <vector>

namespace String
{
	inline std::string Printf(const char* format, ...)
	{
		// 初始化可变参数列表
		va_list args;
		va_start(args, format);
    
		// 第一次调用：确定所需缓冲区大小（包括终止符）
		va_list args_copy;
		va_copy(args_copy, args);
		int length = std::vsnprintf(nullptr, 0, format, args_copy);
		va_end(args_copy);
    
		if (length <= 0) 
			return ""; // 格式化失败
    
		// 创建足够大小的缓冲区
		std::vector<char> buffer(length + 1); // +1 用于终止符
    
		// 第二次调用：实际格式化
		std::vsnprintf(buffer.data(), buffer.size(), format, args);
		va_end(args);
    
		return std::string(buffer.data());
	}
};
