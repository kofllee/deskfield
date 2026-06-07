#pragma once

#include <windows.h>

#include <winrt/Windows.Graphics.Capture.h>

winrt::Windows::Graphics::Capture::GraphicsCaptureItem createGraphicsCaptureItemForWindow(HWND hwnd);
