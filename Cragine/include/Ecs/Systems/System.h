#pragma once

#include "Ecs/Systems/SystemParam.h"
#include <functional>
#include <tuple>
#include <utility>

namespace crg::ecs {

    class World;

    template<typename... Params>
    struct SystemWrapper;

    template<typename... Params>
    struct SystemWrapper<void(Params...)> {
        using FnType =  std::function<void(Params...)>;
        FnType fn;

        std::tuple<typename SystemParam<Params>::State...> states;


        SystemWrapper(FnType function, World& world) : fn(std::move(function)) {
            states = std::make_tuple(SystemParam<Params>::init(world)...);
        }


        template<std::size_t... Is>
        void invoke(World& world, std::index_sequence<Is...>) {

            auto args = std::make_tuple(
                SystemParam<Params>::fetch(&std::get<Is>(states), world)...
            );

            std::apply(fn, args);
        }

        void run(World& world) {
            invoke(
                world,
                std::make_index_sequence<sizeof...(Params)>{}
            );

        }

    };


    class BaseSystem {
    public:
        virtual ~BaseSystem() = default;

        virtual void run(World& world) = 0;
    };

    template <typename... Params>
    struct System final : public BaseSystem {
        SystemWrapper<void(Params...)> systemImpl;

        System(typename SystemWrapper<void(Params...)>::FnType func, World& world) :
        systemImpl(std::move(func), world) {}

        void run(World& world) override {
            systemImpl.run(world);
        }
    };


}
