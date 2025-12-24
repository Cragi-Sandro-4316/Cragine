#include "DefaultModules.h"
#include "InputModule/InputManager.h"
#include "InputModule/KeyCode.h"
#include "Resources/ResourceParam.h"
#include "utils/Logger.h"
#include <Cragine.h>

void tryInputs(crg::ecs::Res<crg::InputManager> res) {
   auto& input = res.get();

   if (input.keyJustPressed(crg::KeyCode::KeyA)) {
       LOG_CORE_INFO("KEY A JUST PRESSED");
   }
}

int main() {
    crg::App app{};
    app.addModule(crg::DefaultModules{});
    app.addSystem(crg::ecs::Schedule::Update, tryInputs);
    app.run();
    LOG_TRACE("Log from game project");
    return 0;
}
