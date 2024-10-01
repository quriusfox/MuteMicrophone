#include "toast.h"

// Namespace declarations for WRL and WinRT types
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

/*
 * Utility function to set the value of an XML node (used in toast notifications)
 */
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

/*
 * Function to show a Windows toast notification
 */
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
