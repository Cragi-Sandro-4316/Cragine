#pragma once

#include "Ecs/Systems/SystemParam.h"
#include "ISystem.h"
#include <functional>
#include <tuple>
#include <utility>

namespace crg::ecs {

    class World;

    template<typename... Args>
    class System : public ISystem {
    public:
        using FnType = std::function<void(Args...)>;

        // Stores the function pointer and pre caches the param state
        System(FnType function, World& world) :
        fn(std::move(function)),
        m_states(std::make_tuple(SystemParam<Args>::init(world)...)) {}


        // Invokes the system
        virtual void run(World& world) override {
            invoke(world, std::make_index_sequence<sizeof...(Args)>{});
        }

    private:
        std::tuple<typename SystemParam<Args>::State...> m_states;

        FnType fn;


        // Fetches the parameters and invokes the function with std::apply
        template<std::size_t... Is>
        void invoke(World& world, std::index_sequence<Is...>) {
            auto args = std::make_tuple(
                SystemParam<Args>::fetch(&std::get<Is>(m_states), world)...
            );

            std::apply(fn, args);
        }



    };

}
