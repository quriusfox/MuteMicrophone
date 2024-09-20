#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <tchar.h>
#include "mute.h"
#include <wrl.h>
#include <windows.ui.notifications.h>
#include <NotificationActivationCallback.h>
#include <wrl/wrappers/corewrappers.h> // For HStringReference
#include <windows.foundation.h> // For Windows::Foundation
#include <shobjidl_core.h>

#pragma comment(lib, "WindowsApp.lib") // Links with the necessary Windows libraries

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used services from windows headers to speed up build time
#define WND_CLASS_NAME L"MuteMicrophone Hidden Class" // Window class name
#define WND_NAME L"MuteMicrophone Hidden Window" // Window name

//#define DEBUG

// Namespace declarations for WRL and WinRT types
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

MICROPHONE* mic; // Global microphone instance to manage microphone-related actions

// Utility function to set the value of an XML node (used in toast notifications)
VOID SetNodeValue(IXmlDocument* doc, IXmlNode* node, const wchar_t* value)
{
    // Create a text node with the given value
    ComPtr<IXmlText> textNode;
    doc->CreateTextNode(HStringReference(value).Get(), &textNode);

    // Append the text node to the given XML node
    ComPtr<IXmlNode> text;
    textNode.As(&text);

    ComPtr<IXmlNode> appendedChild;
    node->AppendChild(text.Get(), &appendedChild);
}

// Function to show a Windows toast notification
VOID ShowNotification(const std::wstring& title, const std::wstring& message)
{
    // Initialize COM in single-threaded apartment (STA) mode
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    if (FAILED(hr))
    {
        std::wcout << L"COM initialization failed: " << hr << std::endl;
        return;
    }

    // Retrieve the toast notification manager
    ComPtr<IToastNotificationManagerStatics> toastManager;

    hr = Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
        &toastManager);

    if (FAILED(hr))
    {
        std::wcout << L"Failed to get ToastNotificationManager: " << hr << std::endl;
        CoUninitialize();
        return;
    }

    // Create a toast notification template (type ToastText02: Title + message)
    ComPtr<IXmlDocument> toastXml;
    
    hr = toastManager->GetTemplateContent(ToastTemplateType_ToastText02, &toastXml);
    
    if (FAILED(hr))
    {
        std::wcout << L"Failed to get template content: " << hr << std::endl;
        CoUninitialize();
        return;
    }

    // Get the text nodes where we will set the title and message
    ComPtr<IXmlNodeList> textNodes;
    toastXml->GetElementsByTagName(HStringReference(L"text").Get(), &textNodes);

    // Set the notification title
    ComPtr<IXmlNode> titleNode;
    
    textNodes->Item(0, &titleNode); // 0 is ID for title
    SetNodeValue(toastXml.Get(), titleNode.Get(), title.c_str());

    // Set the notification message
    ComPtr<IXmlNode> messageNode;
    
    textNodes->Item(1, &messageNode); // 1 is ID for message
    SetNodeValue(toastXml.Get(), messageNode.Get(), message.c_str());

    // Create a toast notification from the XML
    ComPtr<IToastNotificationFactory> toastFactory;
    
    hr = Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(),
        &toastFactory);

    if (FAILED(hr))
    {
        std::wcout << L"Failed to get ToastNotification factory: " << hr << std::endl;
        CoUninitialize();
        return;
    }

    ComPtr<IToastNotification> toast;
    
    hr = toastFactory->CreateToastNotification(toastXml.Get(), &toast);
    
    if (FAILED(hr))
    {
        std::wcout << L"Failed to create ToastNotification: " << hr << std::endl;
        CoUninitialize();
        return;
    }

    // Get the toast notifier and show the notification
    ComPtr<IToastNotifier> notifier;
    
    hr = toastManager->CreateToastNotifierWithId(
        HStringReference(L"MuteMicrophone").Get(), &notifier);

    if (SUCCEEDED(hr))
    {
        notifier->Show(toast.Get());
    }
    else
    {
        std::wcout << L"Failed to show ToastNotification: " << hr << std::endl;
    }

    // Clean up COM
    CoUninitialize();
}

// Window procedure function to handle hotkey events
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_HOTKEY)
    {
        std::wcout << L"WM_HOTKEY received\n";
        HRESULT hr = mute(); // Call mute/unmute function on hotkey press

        if (SUCCEEDED(hr))
        {
            std::wcout << L"Microphone status changed\n";
        }
        else
        {
            std::wcout << L"Failed to change microphone status\n";
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam); // Default window message handling
}

// Create a hidden window to capture hotkey events
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

    return hWnd;
}

// Message loop to listen for hotkey events
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

// Function to mute/unmute the microphone
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

    cleanup(mic); // Clean up after muting/unmuting
    return S_OK;
}

// Cleanup microphone resources
VOID cleanup(MICROPHONE* mic)
{
    if (mic->micVolume) mic->micVolume->Release();
    if (mic->pEnumerator) mic->pEnumerator->Release();
    if (mic->pMicDevice) mic->pMicDevice->Release();

    CoUninitialize(); // Uninitialize COM
}

// Entry point of the program
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
