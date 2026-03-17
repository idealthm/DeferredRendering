#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>


Math::Transform::Transform(): m_location(0.0f), m_rotation(1.0f, 0.0f, 0.0f, 0.0f), m_scale(1.0f)
{}

Math::Transform::Transform(const glm::vec3& location, const glm::quat& rotation, const glm::vec3& scale): m_location(location), m_rotation(rotation), m_scale(scale)
{}

Math::Transform::Transform(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale): m_location(location), m_rotation(glm::quat(glm::radians(rotation))), m_scale(scale)
{}

void Math::Transform::SetRotation(const glm::vec3& eulerDegrees)
{
	glm::vec3 radians = glm::radians(eulerDegrees);
	// GLM 的 quat(vec3) 构造函数顺序：pitch (X), yaw (Y), roll (Z)
	m_rotation = glm::quat(radians);
}

glm::vec3 Math::Transform::GetRotation() const
{
	glm::vec3 radians = glm::eulerAngles(m_rotation); // 返回弧度
	return glm::degrees(radians);
}

glm::mat4 Math::Transform::GetModelMatrix() const
{
	// 平移矩阵
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_location);
	// 旋转矩阵（从四元数转换）
	glm::mat4 rotation = glm::mat4_cast(m_rotation);
	// 缩放矩阵
	glm::mat4 scaling = glm::scale(glm::mat4(1.0f), m_scale);

	// 通常顺序：先缩放，再旋转，最后平移
	return translation * rotation * scaling;
}

Math::Transform Math::Transform::FromMatrix(const glm::mat4& matrix)
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;

	// 调用 GLM 的分解函数
	if (glm::decompose(matrix, scale, rotation, translation, skew, perspective)) {
		// 可选：检查 skew 和 perspective 是否接近零，以验证是否为纯 TRS 矩阵
		// 如果超出容忍范围，可以警告或返回默认 Transform
		return Transform(translation, rotation, scale);
	} else {
		// 分解失败（例如矩阵含有非仿射变换），返回默认变换
		return Transform();
	}
}
