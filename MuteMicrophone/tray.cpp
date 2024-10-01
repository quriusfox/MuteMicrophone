#include "tray.h"

// Global variables for tray icon
NOTIFYICONDATA nid = {}; 
HICON hIconMuted;
HICON hIconUnmuted;

/*
 * Function to initialize the tray icon
 */
VOID InitTrayIcon(HWND hWnd, HINSTANCE hInstance)
{
    hIconMuted = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MUTED));   // Load your muted icon resource
    hIconUnmuted = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_UNMUTED)); // Load your unmuted icon resource

    // Fill the NOTIFYICONDATA structure
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1001;  // Unique ID for the tray icon
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;  // Custom message for tray icon interaction

    // Initially set the icon as unmuted
    nid.hIcon = hIconUnmuted;
    wcscpy_s(nid.szTip, L"Microphone Unmuted");

    // Add the icon to the system tray
    Shell_NotifyIcon(NIM_ADD, &nid);
}

/*
 * Function to update the tray icon based on microphone status
 */
VOID UpdateTrayIcon(BOOL isMuted)
{
    if (isMuted)
    {
        nid.hIcon = hIconMuted;
        wcscpy_s(nid.szTip, L"Microphone Muted");
    }
    else
    {
        nid.hIcon = hIconUnmuted;
        wcscpy_s(nid.szTip, L"Microphone Unmuted");
    }

    // Update the icon in the tray
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

/*
 * Function to remove the tray icon
 */
VOID RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}
