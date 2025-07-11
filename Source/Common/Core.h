#pragma once

#include <memory>
#include "glm/glm.hpp"

typedef signed char        int8;
typedef short              int16;
typedef int                int32;
typedef long long          int64;
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef signed char        int_least8;
typedef short              int_least16;
typedef int                int_least32;
typedef long long          int_least64;
typedef unsigned char      uint_least8;
typedef unsigned short     uint_least16;
typedef unsigned int       uint_least32;
typedef unsigned long long uint_least64;

typedef signed char        int_fast8;
typedef int                int_fast16;
typedef int                int_fast32;
typedef long long          int_fast64;
typedef unsigned char      uint_fast8;
typedef unsigned int       uint_fast16;
typedef unsigned int       uint_fast32;
typedef unsigned long long uint_fast64;

typedef long long          intmax;
typedef unsigned long long uintmax;

#define ToDegree(X) (glm::pi<float>() / 180.f * (X))

template <typename T> using Ref = std::shared_ptr<T>;
template <typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T> using Scope = std::unique_ptr<T>;
template <typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

#define BIT(x) (1 << (x))
#define ASSERT(x) if(!(x)) __debugbreak();

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

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