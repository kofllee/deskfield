#pragma once

#include "CanvasTypes.h"
#include "WindowId.h"
#include "WindowRegistry.h"

#include "windowing/WindowSnapshot.h"

#include <windows.h>

#include <vector>

enum class DeskfieldWindowState {
    Normal,
    CanvasMinimized,
    CanvasFullscreen,
    CaptureOnly,
    NativeInteractive,
    Hidden,
    Unavailable,
    Closed
};

struct NativeSourceState {
    RECT nativeRect{};
    RECT normalNativeRect{};

    bool visible{};
    bool minimized{};
    bool maximized{};
    bool cloaked{};
};

struct CanvasWindow {
    WindowId id{};
    HWND hwnd{};

    std::wstring title{};
    std::wstring className{};
    DWORD processId{};

    CanvasRect canvasRect{};
    CanvasRect savedNormalCanvasRect{};

    DeskfieldWindowState state{DeskfieldWindowState::Normal};
    NativeSourceState native{};

    bool selected{};
    bool captureEnabled{true};
};

class WorkspaceModel {
public:
    void clear();

    void addWindow(
        const WindowSnapshot& snapshot,
        WindowId id,
        const CanvasCamera& camera
    );

    void removeWindow(WindowId id);

    void updateMetadata(
        WindowId id,
        const WindowSnapshot& snapshot
    );

    void updateNativeState(
        WindowId id,
        const WindowSnapshot& snapshot
    );

    void setCanvasRect(WindowId id, const CanvasRect& rect);
    void setState(WindowId id, DeskfieldWindowState state);

    CanvasWindow* findById(WindowId id);
    const CanvasWindow* findById(WindowId id) const;

    CanvasWindow* findByHwnd(HWND hwnd);
    const CanvasWindow* findByHwnd(HWND hwnd) const;

    std::vector<CanvasWindow>& windows();
    const std::vector<CanvasWindow>& windows() const;

private:
    static CanvasRect makeInitialCanvasRect(
        const WindowSnapshot& snapshot,
        const CanvasCamera& camera
    );

    static NativeSourceState makeNativeState(
        const WindowSnapshot& snapshot
    );

private:
    std::vector<CanvasWindow> windows_{};
};