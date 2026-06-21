#pragma once

#include <memory>

#define ToDegree(X) (glm::pi<float>() / 180.f * (X))

template <typename T> using Ref = std::shared_ptr<T>;
template <typename T, typename ... Args>
constexpr auto CreateRef(Args&& ... args) 
	-> decltype(std::make_shared<T>(std::forward<Args>(args)...))
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T> using Scope = std::unique_ptr<T>;
template <typename T, typename ... Args>
constexpr auto CreateScope(Args&& ... args)
	-> decltype(std::make_unique<T>(std::forward<Args>(args)...))
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using UniquePtr = std::unique_ptr<T>;

#define BIT(x) (1 << (x))
#define ASSERT(x) if(!(x)) __debugbreak();

#ifdef __GNUC__
#define UTILS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define UTILS_UNLIKELY(x) (x)
#endif

#include <iostream>

#ifdef DR_DEBUG
#define GL_CALL_DEBUG_HEAD		GLClearError();
#define GL_CALL_DEBUG_END(x)	ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define GL_CALL_DEBUG_HEAD
#define GL_CALL_DEBUG_END(x)
#endif

#define BIND_FUNCTION_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define GLCall(x) GL_CALL_DEBUG_HEAD \
x; \
GL_CALL_DEBUG_END(x)