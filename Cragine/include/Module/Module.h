#pragma once

namespace crg {

    class App;

    class Module {
    public:
        virtual ~Module() = default;
        virtual void build(App& app) = 0;
    };
}
