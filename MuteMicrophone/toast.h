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

// Namespace declarations for WRL and WinRT types
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

/*
 * Utility function to set the value of an XML node (used in toast notifications)
 */
VOID SetNodeValue(IXmlDocument* doc, IXmlNode* node, const wchar_t* value);

/*
 * Function to show a Windows toast notification
 */
VOID ShowNotification(const std::wstring& title, const std::wstring& message);
