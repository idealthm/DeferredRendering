#include "GLProgram.h"

#include "GLDriver.h"
#include "Shader/Program.h"

GLProgram::GLProgram(RHI::GLDriver* driver, Program&& program) noexcept
	: HwProgram(std::move(program.getName()))
{
	driver->CompileShader(program.GetShadersSource(), gl.program);
	InitializeProgramState(driver->GetContext(), gl.program, program.GetDescriptorBindings());
}

bool GLProgram::use(RHI::GLDriver* driver, RHI::OpenGLContext* context)
{
	return false;
}

void GLProgram::InitializeProgramState(RHI::OpenGLContext& context, GLuint program, const Program::DescriptorSetInfo& info)
{
    GLuint binding = 0;
    GLuint tmu = 0;

	for (descriptor_set_t set = 0; set < MAX_DESCRIPTOR_SET_COUNT; set++) {
        for (Program::Descriptor const& entry: info[set]) {
            switch (entry.type)
            {
                case RHI::DescriptorType::UNIFORM_BUFFER:
                case RHI::DescriptorType::SHADER_STORAGE_BUFFER:
                {
                    if (!entry.name.empty()) {
                        GLuint const index = glGetUniformBlockIndex(program,
                                entry.name.c_str());
                        if (index != GL_INVALID_INDEX) {
                            // this can fail if the program doesn't use this descriptor
                            glUniformBlockBinding(program, index, binding);
                            m_BindingMap.insert(set, entry.binding,
                                    { binding, entry.type });
                            ++binding;
                        }
                    }
                    break;
                }
                case RHI::DescriptorType::SAMPLER:
                {
                    if (!entry.name.empty()) {
                        GLint const loc = glGetUniformLocation(program, entry.name.c_str());
                        if (loc >= 0) {
                            // this can fail if the program doesn't use this descriptor
                            m_BindingMap.insert(set, entry.binding, { tmu, entry.type });
                            glUniform1i(loc, GLint(tmu));
                            ++tmu;
                        }
                    }
                    break;
                }
                case RHI::DescriptorType::INPUT_ATTACHMENT:
                    break;
            }
        }
    }
}

GLuint GLProgram::getBufferBinding(descriptor_set_t set, descriptor_binding_t binding) const noexcept
{
	return m_BindingMap.get(set, binding);
}

GLuint GLProgram::getTextureUnit(descriptor_set_t set, descriptor_binding_t binding) const noexcept
{
	return m_BindingMap.get(set, binding);
}

Util::bitset64 GLProgram::getActiveDescriptors(descriptor_set_t set) const
{
	return m_BindingMap.getActiveDescriptors(set);
}
