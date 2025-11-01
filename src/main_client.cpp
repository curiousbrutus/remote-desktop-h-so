#include "common/network.h"
#include "common/protocol.h"
#include "client/client.h"
#include "client/renderer.h"
#include <iostream>
#include <string>

using namespace RemoteDesktop;

// Global variables
static Client* g_client = nullptr;
static Renderer* g_renderer = nullptr;
static HWND g_hwnd = nullptr;

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (g_renderer) {
                g_renderer->Render(hdc);
            } else {
                // Fill with black if no renderer
                FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE:
            // Trigger redraw on resize
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_ERASEBKGND:
            // Prevent flicker
            return 1;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Frame callback from client
void OnFrameReceived(const std::vector<uint8_t>& bgraData, 
                     uint32_t width, uint32_t height) {
    if (g_renderer) {
        g_renderer->UpdateFrame(bgraData, width, height);
    }
}

int main(int argc, char* argv[]) {
    std::cout << "==================================" << std::endl;
    std::cout << "  Remote Desktop Client" << std::endl;
    std::cout << "==================================" << std::endl;

    // Initialize Winsock
    if (!NetworkSocket::InitializeWinsock()) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }

    // Parse arguments
    std::string host = "127.0.0.1"; // Default to localhost
    uint16_t port = DEFAULT_PORT;

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = static_cast<uint16_t>(std::atoi(argv[2]));
    }

    std::cout << "Connecting to " << host << ":" << port << std::endl;

    // Register window class
    const wchar_t CLASS_NAME[] = L"RemoteDesktopClientWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClassW(&wc);

    // Create window
    g_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Remote Desktop Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720, // Initial size
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!g_hwnd) {
        std::cerr << "Failed to create window" << std::endl;
        NetworkSocket::CleanupWinsock();
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOW);

    // Initialize renderer
    Renderer renderer;
    g_renderer = &renderer;

    if (!renderer.Initialize(g_hwnd)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        NetworkSocket::CleanupWinsock();
        return 1;
    }

    // Create and connect client
    Client client;
    g_client = &client;

    client.SetFrameCallback(OnFrameReceived);

    if (!client.Connect(host, port)) {
        std::cerr << "Failed to connect to server" << std::endl;
        NetworkSocket::CleanupWinsock();
        return 1;
    }

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    client.Disconnect();
    NetworkSocket::CleanupWinsock();

    std::cout << "Client terminated." << std::endl;
    return 0;
}
