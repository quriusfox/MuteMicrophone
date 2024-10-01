#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <tchar.h>
#include "mute.h"
#include "tray.h"
#include "toast.h"

#pragma comment(lib, "WindowsApp.lib") // Links with the necessary Windows libraries

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used services from windows headers to speed up build time
#define WND_CLASS_NAME L"MuteMicrophone Hidden Class" // Window class name
#define WND_NAME L"MuteMicrophone Hidden Window" // Window name

//#define DEBUG

// Global microphone instance to manage microphone-related actions
MICROPHONE* mic;

/*
 * Window procedure function to handle hotkey events
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;

    switch (msg)
    {
        case WM_HOTKEY:
            std::wcout << L"WM_HOTKEY received\n";
            hr = mute(); // Call mute/unmute function on hotkey press

            if (SUCCEEDED(hr))
            {
                std::wcout << L"Microphone status changed\n";
            }
            else
            {
                std::wcout << L"Failed to change microphone status\n";
            }
            break;

        case WM_TRAYICON:
            if (LOWORD(lParam) == WM_LBUTTONDOWN)
            {
                // Toggle microphone mute/unmute when left-clicking on the tray icon
                mute();
            }
            break;

        case WM_DESTROY:
            // Remove the tray icon when the window is destroyed
            RemoveTrayIcon();
            PostQuitMessage(0);
            break;
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam); // Default window message handling
}

/*
 * Create a hidden window to capture hotkey events
 */
HWND createWindow()
{
    // Show the console window for debugging purposes
    HWND hConsole = GetConsoleWindow();

    #ifdef DEBUG
        ShowWindow(hConsole, SW_SHOW);
    #else
        ShowWindow(hConsole, SW_HIDE);
    #endif

    WNDCLASS wc = { };

    // Set the window procedure and class name for the hidden window
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = WND_CLASS_NAME;

    // Register the window class
    RegisterClass(&wc);

    // Create a hidden window that will be used to capture hotkey events
    HWND hWnd = CreateWindowEx(
        0,
        WND_CLASS_NAME,
        WND_NAME,
        0,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    // Initialize tray icon
    InitTrayIcon(hWnd, wc.hInstance);

    return hWnd;
}

/*
 * Message loop to listen for hotkey events
 */
VOID msgListenLoop()
{
    MSG msg = { 0 };

    // Enter the message loop to wait for and process messages (like WM_HOTKEY)
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/*
 * Function to mute/unmute the microphone
 */
HRESULT mute()
{
    BOOL wasMuted;
    HRESULT hr = CoInitialize(NULL); // Initialize COM

    if (FAILED(hr))
    {
        return S_FALSE;
    }

    // Get the microphone endpoint
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (LPVOID*)&mic->pEnumerator
    );

    if (FAILED(hr))
    {
        cleanup(mic); // Clean up in case of failure
        return S_FALSE;
    }

    hr = mic->pEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &mic->pMicDevice);

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }

    // Get the audio endpoint volume control interface
    hr = mic->pMicDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (VOID**)&mic->micVolume);

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }

    // Get the current mute status
    hr = mic->micVolume->GetMute(&wasMuted);

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }

    // Toggle mute status
    mic->micVolume->SetMute(!wasMuted, NULL);

    // Show a toast notification indicating whether the microphone is muted or unmuted
    
    /*if (!wasMuted)
    {
        MessageBox(NULL, L"Microphone muted", L"Mute status", MB_OK);
    }
    else
    {
        MessageBox(NULL, L"Microphone unmuted", L"Mute status", MB_OK);
    }*/
    
    if (!wasMuted)
    {
        ShowNotification(L"Mute status", L"Microphone muted");
    }
    else
    {
        ShowNotification(L"Mute status", L"Microphone unmuted");
    }

    // Update the tray icon based on the new mute status
    UpdateTrayIcon(!wasMuted);

    cleanup(mic); // Clean up after muting/unmuting
    return S_OK;
}

/*
 * Cleanup microphone resources
 */
VOID cleanup(MICROPHONE* mic)
{
    if (mic->micVolume) mic->micVolume->Release();
    if (mic->pEnumerator) mic->pEnumerator->Release();
    if (mic->pMicDevice) mic->pMicDevice->Release();

    CoUninitialize(); // Uninitialize COM
}

/*
 * Entry point of the program
 */
DWORD main()
{
    BOOL success;
    HWND hWnd = createWindow(); // Create a hidden window to capture hotkey events

    if (hWnd == NULL)
    {
        std::wcout << L"Failed to create window\n";
        return -1;
    }

    // Register 'ALT+M' as a global hotkey (0x4D is the virtual key code for 'M')
    success = RegisterHotKey(
        hWnd,
        1,
        MOD_ALT | MOD_NOREPEAT,
        0x4D // 0x4D is 'M'
    );

    if (!success)
    {
        std::wcout << L"Failed to register hotkey\n";
        return -1;
    }

    std::wcout << L"Hotkey 'ALT+M' registered\n";

    // Allocate memory for the MICROPHONE struct
    mic = (MICROPHONE*)VirtualAlloc(NULL, sizeof(MICROPHONE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (mic == NULL)
    {
        return S_FALSE;
    }

    // Start listening for messages, including hotkey events
    msgListenLoop();

    return 0;
}
