#pragma once

// #include "AssetManager/AssetManager.h"
#include "AssetManager/AssetManager.h"
#include "Core/App.h"
#include "Module/Module.h"

namespace crg {
    class AssetManagerModule : public Module {
    public:

        virtual void build(App& app) {
            app.addResource<AssetManager>();
        }
    private:
    };
}
