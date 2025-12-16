#include "InputModule/InputModule.h"
#include <Cragine.h>

int main() {
    crg::App app{};
    app.addModule(crg::InputModule{});
    app.run();
    LOG_TRACE("Log from game project");
    return 0;
}
