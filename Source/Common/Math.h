#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Math
{
	class Transform {
	public:
		// 默认构造函数：位置(0,0,0)，单位四元数，缩放(1,1,1)
		Transform();

		// 带参数的构造函数
		Transform(const glm::vec3& location, const glm::quat& rotation, const glm::vec3& scale);

		Transform(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale);

		// --- 位置 ---
		void SetLocation(const glm::vec3& location) { m_location = location; }
		glm::vec3 GetLocation() const { return m_location; }

		// --- 旋转（欧拉角，角度制）---
		// 设置旋转：输入欧拉角（角度），内部转换为四元数存储
		void SetRotation(const glm::vec3& eulerDegrees);

		// 获取旋转：从四元数转换回欧拉角（角度）
		glm::vec3 GetRotation() const;

		// --- 旋转（四元数直接操作）---
		void SetRotationQuat(const glm::quat& rotation) { m_rotation = rotation; }
		glm::quat GetRotationQuat() const { return m_rotation; }

		// --- 缩放 ---
		void SetScale(const glm::vec3& scale) { m_scale = scale; }
		glm::vec3 GetScale() const { return m_scale; }

		// --- 构建模型矩阵 ---
		glm::mat4 GetModelMatrix() const;

		static Transform FromMatrix(const glm::mat4& matrix);

	private:
		glm::vec3 m_location;
		glm::quat m_rotation;
		glm::vec3 m_scale;
	};
}
