#pragma once

class RendererAPI
{
public:
	enum class Type { OpenGL, Vulkan };

	static Type Get() { return s_Type; }
	static void Set(Type type) { s_Type = type; }

private:
	static inline Type s_Type = Type::OpenGL;
};
