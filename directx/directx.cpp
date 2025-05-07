#include "directx.hpp"
#include "../darkside.hpp"
#include "../vmt/vmt.hpp"
#include "../render/render.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <C:\Users\тот самый\Downloads\darkside_sdk-fix-master\darkside_sdk-fix-master\darkside_sdk\sdk\includes\imgui\imgui.h>
#include <C:\Users\тот самый\Downloads\darkside_sdk-fix-master\darkside_sdk-fix-master\darkside_sdk\sdk\includes\imgui\imgui_impl_dx11.h>
#include <C:\Users\тот самый\Downloads\darkside_sdk-fix-master\darkside_sdk-fix-master\darkside_sdk\sdk\includes\imgui\imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT hk_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (uMsg == WM_KEYUP && wParam == VK_INSERT)
        g_menu->m_opened = !g_menu->m_opened;

    static auto original = hooks::enable_cursor::m_enable_cursor.get_original<decltype(&hooks::enable_cursor::hk_enable_cursor)>();
    original(g_interfaces->m_input_system, g_menu->m_opened ? false : hooks::enable_cursor::m_enable_cursor_input);

    return CallWindowProc(g_directx->m_window_proc_original, hWnd, uMsg, wParam, lParam);
}

bool c_directx::initialize() {
    if (!g_opcodes || !g_modules) return false;

    uint8_t* ptr = g_opcodes->scan_relative(g_modules->m_modules.rendersystem_dll.get_name(), "66 0F 7F 0D ? ? ? ? 48 8B F7 66 0F 7F 05", 0x0, 0x4, 0x8);

    if (!ptr)
        return false;

    uint8_t* next_ptr = **reinterpret_cast<uint8_t***>(ptr);
    if (!next_ptr)
        return false;

    m_swap_chain = *reinterpret_cast<IDXGISwapChain**>(next_ptr + 0x170);
    if (!m_swap_chain)
        return false;

    m_present_address = reinterpret_cast<void*>(VMT::get_v_method(m_swap_chain, 8));
    return m_present_address != nullptr;
}

void c_directx::unitialize() {
    if (m_window_proc_original) {
        SetWindowLongPtr(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_window_proc_original));
        m_window_proc_original = nullptr;
    }

    if (m_render_target) m_render_target->Release();
    if (m_device_context) m_device_context->Release();
    if (m_device) m_device->Release();
    if (m_swap_chain) m_swap_chain->Release();

    m_render_target = nullptr;
    m_device_context = nullptr;
    m_device = nullptr;
    m_swap_chain = nullptr;
    m_window = nullptr;
}

std::once_flag init_flag;

void c_directx::start_frame(IDXGISwapChain* swap_chain) {
    if (!swap_chain) return;

    ID3D11Texture2D* back_buffer = nullptr;
    DXGI_SWAP_CHAIN_DESC desc;
    if (FAILED(swap_chain->GetDesc(&desc))) return;

    m_window = desc.OutputWindow;

    std::call_once(init_flag, [&]() {
        if (FAILED(swap_chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_device)))) return;
        m_device->GetImmediateContext(&m_device_context);
        if (FAILED(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer)))) return;

        if (FAILED(m_device->CreateRenderTargetView(back_buffer, nullptr, &m_render_target))) {
            back_buffer->Release();
            return;
        }

        if (back_buffer) back_buffer->Release();

        m_window_proc_original = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hk_wnd_proc)));

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(m_window);
        ImGui_ImplDX11_Init(m_device, m_device_context);

        g_render->initialize();
        });
}

void c_directx::new_frame() {
    if (!m_device_context) return;

    ImGuiIO& io = ImGui::GetIO();
    g_render->update_screen_size(io);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void c_directx::end_frame() {
    if (!m_device_context || !m_render_target) return;

    ImGui::Render();

    m_device_context->OMSetRenderTargets(1, &m_render_target, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}