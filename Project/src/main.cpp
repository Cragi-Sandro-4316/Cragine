#include "DefaultModules.h"
#include "utils/Logger.h"
#include <Cragine.h>

void tryInputs() {

}

int main() {
    crg::App app{};
    app
        .addModule(crg::DefaultModules{})
    .run();

    LOG_TRACE("Log from game project");
    return 0;
}
