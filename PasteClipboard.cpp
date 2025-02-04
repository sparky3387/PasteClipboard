#include <windows.h>
#include <string>
#include <thread>
#include <chrono>

// Registry key path to store delay value
#define REGISTRY_PATH L"Software\\PasteClipboard"

// Global variables for UI elements
HWND hTextBox, hTimerBox, hButton, hTextLabel, hTimerLabel;

// Function to save the delay value in the registry
void SaveDelayToRegistry(int delay) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_PATH, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, L"Delay", 0, REG_DWORD, (BYTE*)&delay, sizeof(delay));
        RegCloseKey(hKey);
    }
}

// Function to retrieve the delay value from the registry
int LoadDelayFromRegistry() {
    HKEY hKey;
    DWORD delay = 0;
    DWORD dataSize = sizeof(delay);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, L"Delay", nullptr, nullptr, (LPBYTE)&delay, &dataSize);
        RegCloseKey(hKey);
    }
    return delay;
}

// Function to simulate keystrokes
void SimulateKeystrokes(const std::wstring &text) {
    for (auto ch : text) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;             // Virtual key code
        input.ki.wScan = ch;          // Unicode character
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        SendInput(1, &input, sizeof(INPUT));

        // Key release event
        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}

// Callback function for the window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if ((HWND)lParam == hButton) {
            // Get text from the multiline text box
            wchar_t textBuffer[1024];
            GetWindowText(hTextBox, textBuffer, 1024);

            // Get timer value from the timer box
            wchar_t timerBuffer[256];
            GetWindowText(hTimerBox, timerBuffer, 256);

            // Convert timer value to an integer
            int delaySeconds = _wtoi(timerBuffer);

            // Save the delay value to the registry
            SaveDelayToRegistry(delaySeconds);

            // Wait for the specified delay before simulating keystrokes
            std::thread([textBuffer, delaySeconds]() {
                std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
                SimulateKeystrokes(textBuffer);
            }).detach();
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"PasteClipboard";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // Create the main window
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"PasteClipboard", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 350, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) return 0;

    // Create a label for the text box
    hTextLabel = CreateWindow(L"STATIC", L"Enter Text:", WS_VISIBLE | WS_CHILD,
        50, 20, 100, 20, hwnd, nullptr, hInstance, nullptr);

    // Create a multiline text box for input text
    hTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        50, 40, 380, 100, hwnd, nullptr, hInstance, nullptr);

    // Create a label for the timer box
    hTimerLabel = CreateWindow(L"STATIC", L"Delay (seconds):", WS_VISIBLE | WS_CHILD,
        50, 150, 120, 20, hwnd, nullptr, hInstance, nullptr);

    // Load the delay value from the registry
    int savedDelay = LoadDelayFromRegistry();
    wchar_t delayStr[10];
    _itow_s(savedDelay, delayStr, 10);

    // Create a text box for the timer
    hTimerBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", delayStr, WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        50, 170, 300, 20, hwnd, nullptr, hInstance, nullptr);

    // Create a button
    hButton = CreateWindow(L"BUTTON", L"Paste Clipboard", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        150, 210, 150, 30, hwnd, nullptr, hInstance, nullptr);

    // Show the window
    ShowWindow(hwnd, nCmdShow);

    // Message loop
    MSG msg = { };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
