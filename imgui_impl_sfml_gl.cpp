#include <SFML/System.hpp>
#include <imgui.h>
#include "imgui_impl_sfml_gl.h"
#include "gl_imgui_gl_core_3_2.hpp"

// Data
static bool g_DeviceObjectsCreated = false;
static GLuint g_FontTexture = 0;
static bool g_MousePressed[5] = { false, false, false, false, false };

// OpenGL objects
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

namespace gl = imgui::gl;

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplSfmlGL_RenderDrawLists( ImDrawData* draw_data )
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLint last_program; gl::GetIntegerv(gl::CURRENT_PROGRAM, &last_program);
    GLint last_texture; gl::GetIntegerv(gl::TEXTURE_BINDING_2D, &last_texture);
    GLint last_active_texture; gl::GetIntegerv(gl::ACTIVE_TEXTURE, &last_active_texture);
    GLint last_array_buffer; gl::GetIntegerv(gl::ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; gl::GetIntegerv(gl::ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; gl::GetIntegerv(gl::VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_blend_src; gl::GetIntegerv(gl::BLEND_SRC, &last_blend_src);
    GLint last_blend_dst; gl::GetIntegerv(gl::BLEND_DST, &last_blend_dst);
    GLint last_blend_equation_rgb; gl::GetIntegerv(gl::BLEND_EQUATION_RGB, &last_blend_equation_rgb);
    GLint last_blend_equation_alpha; gl::GetIntegerv(gl::BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
    GLint last_viewport[4]; gl::GetIntegerv(gl::VIEWPORT, last_viewport);
    GLboolean last_enable_blend = gl::IsEnabled(gl::BLEND);
    GLboolean last_enable_cull_face = gl::IsEnabled(gl::CULL_FACE);
    GLboolean last_enable_depth_test = gl::IsEnabled(gl::DEPTH_TEST);
    GLboolean last_enable_scissor_test = gl::IsEnabled(gl::SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    gl::Enable(gl::BLEND);
    gl::BlendEquation(gl::FUNC_ADD);
    gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
    gl::Disable(gl::CULL_FACE);
    gl::Disable(gl::DEPTH_TEST);
    gl::Enable(gl::SCISSOR_TEST);
    gl::ActiveTexture(gl::TEXTURE0);

    // Setup orthographic projection matrix
    gl::Viewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float ortho_projection[4][4] =
            {
                    { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
                    { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
                    { 0.0f,                  0.0f,                  -1.0f, 0.0f },
                    {-1.0f,                  1.0f,                   0.0f, 1.0f },
            };
    gl::UseProgram(g_ShaderHandle);
    gl::Uniform1i(g_AttribLocationTex, 0);
    gl::UniformMatrix4fv(g_AttribLocationProjMtx, 1, gl::FALSE_, &ortho_projection[0][0]);
    gl::BindVertexArray(g_VaoHandle);

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
        gl::BufferData(gl::ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), gl::STREAM_DRAW);

        gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), gl::STREAM_DRAW);

        for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
        {
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                gl::BindTexture(gl::TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                gl::Scissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                gl::DrawElements(gl::TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? gl::UNSIGNED_SHORT : gl::UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state
    gl::UseProgram(last_program);
    gl::ActiveTexture(last_active_texture);
    gl::BindTexture(gl::TEXTURE_2D, last_texture);
    gl::BindVertexArray(last_vertex_array);
    gl::BindBuffer(gl::ARRAY_BUFFER, last_array_buffer);
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    gl::BlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    gl::BlendFunc(last_blend_src, last_blend_dst);
    if (last_enable_blend) gl::Enable(gl::BLEND); else gl::Disable(gl::BLEND);
    if (last_enable_cull_face) gl::Enable(gl::CULL_FACE); else gl::Disable(gl::CULL_FACE);
    if (last_enable_depth_test) gl::Enable(gl::DEPTH_TEST); else gl::Disable(gl::DEPTH_TEST);
    if (last_enable_scissor_test) gl::Enable(gl::SCISSOR_TEST); else gl::Disable(gl::SCISSOR_TEST);
    gl::Viewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

bool ImGui_ImplSfmlGL_Init()
{
    ImGuiIO& io = ImGui::GetIO();

    io.KeyMap[ ImGuiKey_Tab ] = sf::Keyboard::Tab;
    io.KeyMap[ ImGuiKey_LeftArrow ] = sf::Keyboard::Left;
    io.KeyMap[ ImGuiKey_RightArrow ] = sf::Keyboard::Right;
    io.KeyMap[ ImGuiKey_UpArrow ] = sf::Keyboard::Up;
    io.KeyMap[ ImGuiKey_DownArrow ] = sf::Keyboard::Down;
    io.KeyMap[ ImGuiKey_PageUp ] = sf::Keyboard::PageUp;
    io.KeyMap[ ImGuiKey_PageDown ] = sf::Keyboard::PageDown;
    io.KeyMap[ ImGuiKey_Home ] = sf::Keyboard::Home;
    io.KeyMap[ ImGuiKey_End ] = sf::Keyboard::End;
    io.KeyMap[ ImGuiKey_Delete ] = sf::Keyboard::Delete;
    io.KeyMap[ ImGuiKey_Backspace ] = sf::Keyboard::BackSpace;
    io.KeyMap[ ImGuiKey_Enter ] = sf::Keyboard::Return;
    io.KeyMap[ ImGuiKey_Escape ] = sf::Keyboard::Escape;
    io.KeyMap[ ImGuiKey_A ] = sf::Keyboard::A;
    io.KeyMap[ ImGuiKey_C ] = sf::Keyboard::C;
    io.KeyMap[ ImGuiKey_V ] = sf::Keyboard::V;
    io.KeyMap[ ImGuiKey_X ] = sf::Keyboard::X;
    io.KeyMap[ ImGuiKey_Y ] = sf::Keyboard::Y;
    io.KeyMap[ ImGuiKey_Z ] = sf::Keyboard::Z;

    io.RenderDrawListsFn = ImGui_ImplSfmlGL_RenderDrawLists;

    gl::sys::LoadFunctions();

    return true;
}

void ImGui_ImplSfmlGL_Shutdown()
{
    ImGui_ImplSfmlGL_InvalidateDeviceObjects();
    ImGui::Shutdown();
}

void ImGui_ImplSfmlGL_NewFrame( sf::Window& window, const sf::Time& dt )
{
    if ( !g_DeviceObjectsCreated )
        ImGui_ImplSfmlGL_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    sf::Vector2u size = window.getSize();
    io.DisplaySize = ImVec2((float)size.x, (float)size.y);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.DeltaTime = dt.asSeconds();

    window.setMouseCursorVisible(!io.MouseDrawCursor);

    // Get mouse state (again)
    io.MouseDown[ 0 ] = g_MousePressed[ 0 ] || sf::Mouse::isButtonPressed( sf::Mouse::Button::Left );
    io.MouseDown[ 1 ] = g_MousePressed[ 1 ] || sf::Mouse::isButtonPressed( sf::Mouse::Button::Middle );
    io.MouseDown[ 2 ] = g_MousePressed[ 2 ] || sf::Mouse::isButtonPressed( sf::Mouse::Button::Right );
    io.MouseDown[ 3 ] = g_MousePressed[ 3 ] || sf::Mouse::isButtonPressed( sf::Mouse::Button::XButton1 );
    io.MouseDown[ 4 ] = g_MousePressed[ 4 ] || sf::Mouse::isButtonPressed( sf::Mouse::Button::XButton2 );
    g_MousePressed[ 0 ] = false;
    g_MousePressed[ 1 ] = false;
    g_MousePressed[ 2 ] = false;
    g_MousePressed[ 3 ] = false;
    g_MousePressed[ 4 ] = false;

    ImGui::NewFrame();
}

bool ImGui_ImplSfmlGL_ProcessEvent( const sf::Event& event )
{
    ImGuiIO& io = ImGui::GetIO();

    switch ( event.type ) {
        case sf::Event::MouseMoved:
            io.MousePos = ImVec2( (float)event.mouseMove.x, (float)event.mouseMove.y );
            return true;
        case sf::Event::LostFocus:
            io.MousePos = ImVec2( -1.0f, -1.0f );
            return true;
        case sf::Event::MouseWheelScrolled:
            io.MouseWheel = event.mouseWheelScroll.delta;
            return true;
        case sf::Event::MouseButtonPressed: {
            const auto& button = event.mouseButton.button;

            if ( button == sf::Mouse::Button::Left )
                g_MousePressed[ 0 ] = true;
            if ( button == sf::Mouse::Button::Middle )
                g_MousePressed[ 1 ] = true;
            if ( button == sf::Mouse::Button::Right )
                g_MousePressed[ 2 ] = true;
            if ( button == sf::Mouse::Button::XButton1 )
                g_MousePressed[ 3 ] = true;
            if ( button == sf::Mouse::Button::XButton2 )
                g_MousePressed[ 4 ] = true;

            return true;
        }
        case sf::Event::TextEntered: {
            char output[6] = { 0, 0, 0, 0, 0, 0 };
            sf::Utf8::encode( event.text.unicode, output );
            io.AddInputCharactersUTF8( output );
            return true;
        }
        case sf::Event::KeyPressed:
        case sf::Event::KeyReleased:
            io.KeysDown[ event.key.code ] = ( event.type == sf::Event::KeyPressed );
            io.KeyShift = event.key.shift;
            io.KeyCtrl = event.key.control;
            io.KeyAlt = event.key.alt;
            io.KeySuper = event.key.system;
            return true;
        default:
            return false;
    }
}

void ImGui_ImplSfmlGL_InvalidateDeviceObjects()
{
    if (g_VaoHandle) gl::DeleteVertexArrays(1, &g_VaoHandle);
    if (g_VboHandle) gl::DeleteBuffers(1, &g_VboHandle);
    if (g_ElementsHandle) gl::DeleteBuffers(1, &g_ElementsHandle);
    g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

    gl::DetachShader(g_ShaderHandle, g_VertHandle);
    gl::DeleteShader(g_VertHandle);
    g_VertHandle = 0;

    gl::DetachShader(g_ShaderHandle, g_FragHandle);
    gl::DeleteShader(g_FragHandle);
    g_FragHandle = 0;

    gl::DeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    if (g_FontTexture)
    {
        gl::DeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

static void ImGui_ImplSfmlGL_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    gl::GetIntegerv(gl::TEXTURE_BINDING_2D, &last_texture);
    gl::GenTextures(1, &g_FontTexture);
    gl::BindTexture(gl::TEXTURE_2D, g_FontTexture);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, width, height, 0, gl::RGBA, gl::UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Restore state
    gl::BindTexture(gl::TEXTURE_2D, last_texture);
}

bool ImGui_ImplSfmlGL_CreateDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    gl::GetIntegerv(gl::TEXTURE_BINDING_2D, &last_texture);
    gl::GetIntegerv(gl::ARRAY_BUFFER_BINDING, &last_array_buffer);
    gl::GetIntegerv(gl::VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar *vertex_shader =
            "#version 150\n"
                    "uniform mat4 ProjMtx;\n"
                    "in vec2 Position;\n"
                    "in vec2 UV;\n"
                    "in vec4 Color;\n"
                    "out vec2 Frag_UV;\n"
                    "out vec4 Frag_Color;\n"
                    "void main()\n"
                    "{\n"
                    "	Frag_UV = UV;\n"
                    "	Frag_Color = Color;\n"
                    "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                    "}\n";

    const GLchar* fragment_shader =
            "#version 150\n"
                    "uniform sampler2D Texture;\n"
                    "in vec2 Frag_UV;\n"
                    "in vec4 Frag_Color;\n"
                    "out vec4 Out_Color;\n"
                    "void main()\n"
                    "{\n"
                    "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
                    "}\n";

    g_ShaderHandle = gl::CreateProgram();
    g_VertHandle = gl::CreateShader(gl::VERTEX_SHADER);
    g_FragHandle = gl::CreateShader(gl::FRAGMENT_SHADER);
    gl::ShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    gl::ShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    gl::CompileShader(g_VertHandle);
    gl::CompileShader(g_FragHandle);
    gl::AttachShader(g_ShaderHandle, g_VertHandle);
    gl::AttachShader(g_ShaderHandle, g_FragHandle);
    gl::LinkProgram(g_ShaderHandle);

    g_AttribLocationTex = gl::GetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = gl::GetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = gl::GetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = gl::GetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = gl::GetAttribLocation(g_ShaderHandle, "Color");

    gl::GenBuffers(1, &g_VboHandle);
    gl::GenBuffers(1, &g_ElementsHandle);

    gl::GenVertexArrays(1, &g_VaoHandle);
    gl::BindVertexArray(g_VaoHandle);
    gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
    gl::EnableVertexAttribArray(g_AttribLocationPosition);
    gl::EnableVertexAttribArray(g_AttribLocationUV);
    gl::EnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    gl::VertexAttribPointer(g_AttribLocationPosition, 2, gl::FLOAT, gl::FALSE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    gl::VertexAttribPointer(g_AttribLocationUV, 2, gl::FLOAT, gl::FALSE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    gl::VertexAttribPointer(g_AttribLocationColor, 4, gl::UNSIGNED_BYTE, gl::TRUE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

    ImGui_ImplSfmlGL_CreateFontsTexture();

    // Restore modified GL state
    gl::BindTexture(gl::TEXTURE_2D, last_texture);
    gl::BindBuffer(gl::ARRAY_BUFFER, last_array_buffer);
    gl::BindVertexArray(last_vertex_array);

    g_DeviceObjectsCreated = true;

    return true;
}
