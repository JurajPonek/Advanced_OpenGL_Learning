#include "window.hpp"
#include <cstdint>
#include <hidusage.h>
#include "windowsx.h"
#include "auto_release.hpp"
#include <gl/gl.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <optional>
#include <queue>
#include <windef.h>
#include <wingdi.h>
#include <winuser.h>
#include <stdexcept>
#include <print>
#include "event.hpp"
#include "exception.hpp"
#include "key.hpp"
#include "key_event.hpp"
#include "log.hpp"
#include "mouse_button_evet.hpp"
#include "mouse_event.hpp"
#include "opengl.hpp"
#include "stop_event.hpp"
#include "vendor/opengl/glext.h"
#include "vendor/opengl/wglext.h"
#include "error.hpp"


namespace 
{
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB{};
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB{};
    auto g_event_queqe = std::queue<game::Event>{};

    void APIENTRY opengl_debug_callback(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity, 
        GLsizei,
        const GLchar *message,
        const void *)
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        {
            return;
        }
        if (type == GL_DEBUG_TYPE_ERROR)
        {
            throw game::Exception("{} {} {} {} {}", source, type, id, severity, message);
        }
        game::log::info("{} {} {} {} {}", source, type, id, severity, message);
    }


    LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg) 
        {
            case WM_CLOSE : g_event_queqe.emplace(game::StopEvent{}); break;
            case WM_KEYUP : 
            {
                g_event_queqe.emplace(game::KeyEvent{static_cast<game::Key>(wparam), game::KeyState::UP});
                break;
            }
            case WM_KEYDOWN : 
            {
                g_event_queqe.emplace(game::KeyEvent{static_cast<game::Key>(wparam), game::KeyState::DOWN});
                break;
            } 
            case WM_INPUT:
            {
                auto raw = ::RAWINPUT{};
                auto dwSize = ::UINT{sizeof(::RAWINPUT)};
                game::ensure(::GetRawInputData(reinterpret_cast<::HRAWINPUT>(lparam), RID_INPUT, &raw, &dwSize, sizeof(::RAWINPUTHEADER)) != static_cast<::UINT>(-1), "Failed to get raw input");
                if (raw.header.dwType == RIM_TYPEMOUSE)
                {
                    const auto x= raw.data.mouse.lLastX;
                    const auto y= raw.data.mouse.lLastY;
                    g_event_queqe.emplace(game::MouseEvent{static_cast<float>(x), static_cast<float>(y)});
                }
                break;
            }
            case WM_LBUTTONUP:
            {
                g_event_queqe.emplace(game::MouseButtonEvent{static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)), game::MouseButtonState::UP});
                break;
            }
            case WM_LBUTTONDOWN:
            {
                g_event_queqe.emplace(
                    game::MouseButtonEvent{static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)), game::MouseButtonState::DOWN});
                break;
            }
        }
        return ::DefWindowProcA(hwnd, msg, wparam
            , lparam);
    }

    template<typename T>
    void resolve_gl_function(T& function, const std::string& name)
    {
        const auto address = ::wglGetProcAddress(name.c_str());
        game::ensure(address, "could not resolve function address");
        function = reinterpret_cast<T>(address);
        
    }
    
    void resolve_wgl_functions(HINSTANCE instance)
    {
        ::WNDCLASS wc = ::WNDCLASS
        {
            .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
            .lpfnWndProc = ::DefWindowProcA,
            .hInstance = instance,
            .lpszClassName = "dummy window",
        };
        game::ensure(::RegisterClassA(&wc) != 0, "failed create dummy window");
        auto dummy_window = game::AutoRelease<::HWND>
        {
            ::CreateWindowExA(
                0,
                wc.lpszClassName,
                wc.lpszClassName,
                CW_USEDEFAULT, 
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                nullptr,
                wc.hInstance,
                0
                ),
            ::DestroyWindow
        

        };
        auto dc = game::AutoRelease<::HDC>{::GetDC(dummy_window), [&dummy_window](auto dc) {::ReleaseDC(dummy_window, dc);}};
        game::ensure(dc, "Could not get dummy dc");
        auto pfd = ::PIXELFORMATDESCRIPTOR
        {
            .nSize = sizeof(::PIXELFORMATDESCRIPTOR),
            .nVersion = 1,
            .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            .iPixelType = PFD_TYPE_RGBA,
            .cColorBits = 32,
            .cAlphaBits = 8,
            .cDepthBits = 24,
            .cStencilBits = 8,
            .iLayerType = PFD_MAIN_PLANE, 
        };
        auto pixel_format = ::ChoosePixelFormat(dc, &pfd);
        game::ensure(pixel_format != 0, "Failed to choose pixel format");
        game::ensure(::SetPixelFormat(dc, pixel_format, &pfd) == TRUE, "Failed to set pixel format");
        const auto context = game::AutoRelease<::HGLRC>{::wglCreateContext(dc), ::wglDeleteContext};
        game::ensure(context ,"Failed to create wgl context");
        game::ensure(::wglMakeCurrent(dc, context) == TRUE ,"Failed to make current context");

        resolve_gl_function(wglCreateContextAttribsARB, "wglCreateContextAttribsARB");
        resolve_gl_function(wglChoosePixelFormatARB, "wglChoosePixelFormatARB");

        game::ensure(::wglMakeCurrent(dc, 0) == TRUE,"Failed to unbind current context");

        

        
    }

    void init_opengl(HDC dc)
    {
        int pixel_format_attribs[]
        {
            WGL_DRAW_TO_WINDOW_ARB,
            GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, 
            GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,
            GL_TRUE,
            WGL_ACCELERATION_ARB,
            WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB,
            WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,
            32,
            WGL_DEPTH_BITS_ARB,
            24,
            WGL_STENCIL_BITS_ARB,
            8,
            0
        };
        auto pixel_format = 0;
        auto num_formats = UINT{};
        ::wglChoosePixelFormatARB(dc, pixel_format_attribs, 0,1,&pixel_format, &num_formats);
        game::ensure(num_formats != 0u, "Could not choose pixel format");
        auto pfd = ::PIXELFORMATDESCRIPTOR{};
        game::ensure(::DescribePixelFormat(dc, pixel_format, sizeof(pfd), &pfd) != 0,"failed to describe pixel format");
        game::ensure(::SetPixelFormat(dc, pixel_format, &pfd) == TRUE,"fialed to set pixel format");
        int gl_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
};

        auto context = ::wglCreateContextAttribsARB(dc, nullptr, gl_attribs);
        game::ensure(context,"failed to create wgl context");

        game::ensure(::wglMakeCurrent(dc, context) == TRUE,"failed to make current context");


    }
    void resolve_global_opengl_functions()
    {
        #define RESOLVE(TYPE, NAME) resolve_gl_function(NAME, #NAME);
        FOR_OPENGL_FUNCTIONS(RESOLVE);
    }

    void  setup_opengl_debug()
    {
        ::glEnable(GL_DEBUG_OUTPUT);
        ::glDebugMessageCallback(opengl_debug_callback, nullptr);

    }
};



namespace game
{ 
    Window::Window(std::uint32_t width, std::uint32_t height) 
    : m_handle(),m_dc({}), m_windowClass({}), m_width{width}, m_height{height}
    {
        
        m_windowClass = {
            .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
            .lpfnWndProc = window_proc,
            .hInstance = ::GetModuleHandleA(nullptr),
            .lpszClassName = "window class"
            
        };

        ensure(::RegisterClassA(&m_windowClass) != 0 ,"failed to register class");

        ::RECT rect{
            .left = {},
            .top = {},
            .right = static_cast<int>(width),
            .bottom = static_cast<int>(height)
        };
        ensure(::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false) != 0, "Could not resize window");

        m_handle = { ::CreateWindowExA(
                0,
                m_windowClass.lpszClassName,
                "Window",
                WS_OVERLAPPEDWINDOW, 
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                rect.right - rect.left,
                rect.bottom - rect.top,
                nullptr,
                nullptr,
                m_windowClass.hInstance,
                nullptr
                ),
            ::DestroyWindow
        };
        m_dc = game::AutoRelease<::HDC>{::GetDC(m_handle), [this](auto dc) {::ReleaseDC(m_handle, dc);}};

        ::ShowWindow(m_handle, SW_SHOW);
        ::UpdateWindow(m_handle);
        
        const auto rid = ::RAWINPUTDEVICE
        {
            .usUsagePage = HID_USAGE_PAGE_GENERIC,
            .usUsage = HID_USAGE_GENERIC_MOUSE,
            .dwFlags = RIDEV_INPUTSINK,    
            .hwndTarget = m_handle
        };
       ensure(::RegisterRawInputDevices(&rid, 1, sizeof(rid)) == TRUE, "Failed to register input devices");

        resolve_wgl_functions(m_windowClass.hInstance);
        init_opengl(m_dc);
        resolve_global_opengl_functions();
        setup_opengl_debug();

        //::glEnable(GL_CULL_FACE);
        ::glEnable(GL_DEPTH_TEST);
        


    }

    std::optional<Event> Window::pump_event() const
    {
        ::MSG message{};
        while(::PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE) != 0   )
        {
            ::TranslateMessage(&message);
            ::DispatchMessageA(&message);
        }
        if (!g_event_queqe.empty())
        {

            const auto event = g_event_queqe.front();
            g_event_queqe.pop();
            return event;
        }
        return {};
    }

    void Window::swap() const
    {
        ::SwapBuffers(m_dc); 
    }

    ::HWND Window::get_native_handle() const
    {
        return m_handle;
    }

    std::uint32_t Window::get_width() const
    {
        return m_width;
    }
    std::uint32_t Window::get_height() const
    {
        return m_height;
    }
};