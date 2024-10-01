#pragma once

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
#include "resource.h"

#define WM_TRAYICON (WM_USER + 1) // Custom message for tray icon

/*
 * Function to initialize the tray icon
 */
VOID InitTrayIcon(HWND hWnd, HINSTANCE hInstance);


/*
 * Function to update the tray icon based on microphone status
 */
VOID UpdateTrayIcon(BOOL isMuted);


/*
 * Function to remove the tray icon
 */
VOID RemoveTrayIcon();

