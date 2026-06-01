#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace glm
{
    // vec2
    inline std::ostream& operator<<(std::ostream& os, const vec2& v)
    {
        os << "vec2(" << v.x << ", " << v.y << ")";
        return os;
    }

    // vec3
    inline std::ostream& operator<<(std::ostream& os, const vec3& v)
    {
        os << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    // vec4
    inline std::ostream& operator<<(std::ostream& os, const vec4& v)
    {
        os << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    // dvec2
    inline std::ostream& operator<<(std::ostream& os, const dvec2& v)
    {
        os << "dvec2(" << v.x << ", " << v.y << ")";
        return os;
    }

    // dvec3
    inline std::ostream& operator<<(std::ostream& os, const dvec3& v)
    {
        os << "dvec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    // dvec4
    inline std::ostream& operator<<(std::ostream& os, const dvec4& v)
    {
        os << "dvec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    // ivec2
    inline std::ostream& operator<<(std::ostream& os, const ivec2& v)
    {
        os << "ivec2(" << v.x << ", " << v.y << ")";
        return os;
    }

    // ivec3
    inline std::ostream& operator<<(std::ostream& os, const ivec3& v)
    {
        os << "ivec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    // ivec4
    inline std::ostream& operator<<(std::ostream& os, const ivec4& v)
    {
        os << "ivec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    // uvec2
    inline std::ostream& operator<<(std::ostream& os, const uvec2& v)
    {
        os << "uvec2(" << v.x << ", " << v.y << ")";
        return os;
    }

    // uvec3
    inline std::ostream& operator<<(std::ostream& os, const uvec3& v)
    {
        os << "uvec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    // uvec4
    inline std::ostream& operator<<(std::ostream& os, const uvec4& v)
    {
        os << "uvec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    // mat2
    inline std::ostream& operator<<(std::ostream& os, const mat2& m)
    {
        os << "mat2(\n";
        os << "  " << m[0] << "\n";
        os << "  " << m[1] << "\n";
        os << ")";
        return os;
    }

    // mat3
    inline std::ostream& operator<<(std::ostream& os, const mat3& m)
    {
        os << "mat3(\n";
        os << "  " << m[0] << "\n";
        os << "  " << m[1] << "\n";
        os << "  " << m[2] << "\n";
        os << ")";
        return os;
    }

    // mat4
    inline std::ostream& operator<<(std::ostream& os, const mat4& m)
    {
        os << "mat4(\n";
        os << "  " << m[0] << "\n";
        os << "  " << m[1] << "\n";
        os << "  " << m[2] << "\n";
        os << "  " << m[3] << "\n";
        os << ")";
        return os;
    }

    // quat
    inline std::ostream& operator<<(std::ostream& os, const quat& q)
    {
        os << "quat(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
        return os;
    }
}
