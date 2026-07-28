#include "screenRender.hpp"
/*

#include <gr-base/core/geometry/vertex2D.hpp>

#include <gr-base/managers/assetsManager.hpp>

#include "settings.h"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gError.h>

#include <gr-base/core/console.hpp>
#include <gr-base/core/window.hpp>
#include <gr-base/core/clock.hpp>

#include <gr-base/script/camera.hpp>

#include <gr-base/core/file/file.hpp>

gr::gVertexArray *ScreenRender::m_vertexArray = nullptr;

gr::Shader *ScreenRender::m_shader = nullptr;

u32 ScreenRender::m_vbo = 0;

void ScreenRender::Initialize() {
    if (!m_shader)
    {
        ShaderParams shaderParams;
        shaderParams.name = "screen";
        shaderParams.base = gfile::get_data_directory("/assets/shader/screen.glsl");
        shaderParams.flags = ShaderOptimize_static;

        AssetsManager::LoadShaderAsync(shaderParams,
            [](gr::Shader *shader) { ScreenRender::m_shader = shader; }
        );
    }
    CreateMesh();
}

void ScreenRender::Shutdown() {
    if (m_vertexArray && !Settings::IsEngineRunning()) {
        delete m_vertexArray;
        m_vertexArray = nullptr;
    }
}

#include <gr-render/gl.h>
void ScreenRender::Render(Camera *camera)
{
    auto rendertarget = camera->getRenderTarget();

    if (!m_shader) {
        return;
    }

    auto rect = camera->getRect();

    auto windowsize = Window::GetWindowSize();

    Matrix4x4 model = 
                    Math::translate<Matrix4x4>(Vector3{rect.x, rect.y, 0.0f}) *
                    Math::rotate<Matrix4x4>(Quaternion::identity) *
                    Math::scale<Matrix4x4>(Vector3{rect.w, rect.h, 0.0f});

    auto orth = Math::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // m_shader->setUniform("u_projection", &orth);
    // m_shader->setUniform("u_model", &model);

    int textureID = 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendertarget->GetTextureID());

    // m_shader->setUniform("u_texture", (void *)&textureID);

    // m_shader->bind();

    m_vertexArray->bind();
    
    gr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, 6);

    m_vertexArray->unbind();
    // texture->unbind();
    // m_shader->unbind();
}

void ScreenRender::CreateMesh() {
    if (m_vertexArray) {
        return;
    }

    std::vector<Vertex2D> vertices = {
        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},

        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}}
    };

    m_vbo = gr::gVertexArray::CreateBuffer(BufferType::VBO);
    if (!m_vbo)
    {
        return Console::LogError("[ ScreenRender ] Falied to create vbo.");
    }

    m_vertexArray = new gr::gVertexArray();
    if (!m_vertexArray)
    {
        return Console::LogError("[ ScreenRender ] Falied to create vao.");
    }

    m_vertexArray->bind();

    gr::gVertexArray::Bind(m_vbo);
    gr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex2D), BufferUsage::STATIC);
    gr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex2D), vertices.data());

    gr::gVertexArray::SetAttrib(0, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, position)));
    gr::gVertexArray::SetAttrib(1, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, uv)));

    m_vertexArray->unbind();
}
*/

