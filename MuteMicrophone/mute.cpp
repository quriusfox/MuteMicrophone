#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <tchar.h>
#include "mute.h"

#define WIN32_LEAN_AND_MEAN
#define WND_CLASS_NAME L"MuteMicrophone Hidden Class"
#define WND_NAME L"MuteMicrophone Hidden Window"

//#define DEBUG

MICROPHONE* mic;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_HOTKEY)
    {
        std::wcout << L"WM_HOTKEY received\n";

        HRESULT hr = mute();

        if (SUCCEEDED(hr))
        {
            std::wcout << L"Operation performed successfully\n";
        }
        else
        {
            std::wcout << L"Operation failed\n";
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND createWindow()
{
    HWND hConsole = GetConsoleWindow();

    #ifdef DEBUG
        ShowWindow(hConsole, SW_SHOW);
    #else
        ShowWindow(hConsole, SW_HIDE);
    #endif

    WNDCLASS wc = { };

    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = WND_CLASS_NAME;

    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0,
        WND_CLASS_NAME,
        WND_NAME,
        0,  // No window styles
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    return hWnd;
}

VOID msgListenLoop()
{
    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT mute()
{
    BOOL wasMuted;
    HRESULT hr;

    hr = CoInitialize(NULL);

    if (FAILED(hr))
    {
        return S_FALSE;
    }

    // Get the device enumerator
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (LPVOID*)&mic->pEnumerator
    );

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }

    // Get pointer to the current microphone device
    hr = mic->pEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &mic->pMicDevice);

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }

    // Activate the audio endpoint volume control:
    hr = mic->pMicDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&mic->micVolume);

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }

    // Check if the microphone is muted
    hr = mic->micVolume->GetMute(&wasMuted);

    if (FAILED(hr))
    {
        cleanup(mic);
        return S_FALSE;
    }
    
    // Change the state of the microphone based on it previous state
    mic->micVolume->SetMute(!wasMuted, NULL);

    // Cleanup
    cleanup(mic);

    return S_OK;
}

VOID cleanup(MICROPHONE* mic)
{
    if (mic->micVolume) mic->micVolume->Release();
    if (mic->pEnumerator) mic->pEnumerator->Release();
    if (mic->pMicDevice) mic->pMicDevice->Release();

    CoUninitialize();
}

DWORD main()
{
    BOOL success;
    HWND hWnd;

    hWnd = createWindow();

    if (hWnd == NULL)
    {
        return -1;
    }

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

    std::wcout << L"Hotkey 'ALT+M' registered, using MOD_NOREPEAT flag\n";

    // We do not need to free the memory since the only scenario is that the
    // process will be forcibly closed in which the OS performs the cleanup automatically
    mic = (MICROPHONE*)VirtualAlloc(NULL, sizeof(MICROPHONE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (mic == NULL)
    {
        return S_FALSE;
    }

    msgListenLoop();

    return 0;
}
