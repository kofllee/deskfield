#include "NativeLayoutSynchronizer.h"

void NativeLayoutSynchronizer::synchronize(
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const WindowController& controller,
    SourceWindowHost& sourceWindowHost
) {
    for (const CanvasWindow& window : workspace.windows()) {
        sourceWindowHost.applyNativeLayout(
            window,
            camera,
            workArea,
            mapper,
            controller
        );
    }
}