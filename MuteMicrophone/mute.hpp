#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>

typedef struct AudioEndpoint {
    IAudioEndpointVolume* micVolume = nullptr;
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pMicDevice = nullptr;
} MICROPHONE;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createWindow();
VOID msgListenLoop();
VOID cleanup(MICROPHONE* mic);
HRESULT mute();
