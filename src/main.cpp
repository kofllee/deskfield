#include "app/DeskfieldApp.h"

int main() {
    DeskfieldApp app;

    if (!app.initialize()) {
        return 1;
    }

    return app.run();
}