#include "DefaultModules.h"
#include <Cragine.h>

int main() {
    crg::App app{};
    app.addModule(crg::DefaultModules{});
    app.run();
    LOG_TRACE("Log from game project");
    return 0;
}
