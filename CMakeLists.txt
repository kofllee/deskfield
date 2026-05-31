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

        src/overlay/OverlayWindow.cpp
        src/overlay/OverlayWindow.h
        src/app/DeskfieldMode.h
        src/workspace/CanvasCameraController.cpp
        src/workspace/CanvasCameraController.h
        src/app/DeskfieldApp.cpp
        src/app/DeskfieldApp.h
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
)