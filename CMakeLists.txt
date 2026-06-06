cmake_minimum_required(VERSION 3.24)

project(Deskfield LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Deskfield
        src/main.cpp

        src/windowing/WindowEnumerator.cpp
        src/windowing/WindowEnumerator.h
        src/windowing/WindowController.cpp
        src/windowing/WindowController.h

        src/workspace/CanvasTypes.h
        src/workspace/WorkspaceModel.cpp
        src/workspace/WorkspaceModel.h
        src/workspace/ViewportMapper.cpp
        src/workspace/ViewportMapper.h

        src/features/NativeLayoutSynchronizer.cpp
        src/features/NativeLayoutSynchronizer.h

        src/canvas/CanvasHostWindow.cpp
        src/canvas/CanvasHostWindow.h
        src/app/DeskfieldMode.h
        src/workspace/CanvasCameraController.cpp
        src/workspace/CanvasCameraController.h
        src/app/DeskfieldApp.cpp
        src/app/DeskfieldApp.h
        src/windowing/WindowStateTracker.cpp
        src/windowing/WindowStateTracker.h
        src/workspace/WindowId.h
        src/workspace/WindowRegistry.cpp
        src/workspace/WindowRegistry.h
        src/rendering/ICanvasRenderer.h
        src/rendering/GdiDebugCanvasRenderer.cpp
        src/rendering/GdiDebugCanvasRenderer.h
        src/windowing/SourceWindowHost.cpp
        src/windowing/SourceWindowHost.h
        src/capture/CapturedWindow.cpp
        src/capture/CapturedWindow.h
        src/capture/GraphicsCaptureManager.cpp
        src/capture/GraphicsCaptureManager.h
        src/rendering/D3DDevice.cpp
        src/rendering/D3DDevice.h
        src/rendering/D3DCanvasRenderer.cpp
        src/rendering/D3DCanvasRenderer.h
        src/rendering/CanvasGridRenderer.cpp
        src/rendering/CanvasGridRenderer.h
        src/rendering/CapturedWindowRenderer.cpp
        src/rendering/CapturedWindowRenderer.h
        src/platform/Win32Dpi.cpp
        src/platform/Win32Dpi.h
        src/rendering/VisualWindowDrawItem.h
)

target_include_directories(Deskfield PRIVATE
        src
)

target_compile_definitions(Deskfield PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
)

target_link_libraries(Deskfield PRIVATE
        user32
        dwmapi
        d3d11
        dxgi
)