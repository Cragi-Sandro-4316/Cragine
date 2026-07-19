#pragma once

#include <tuple>
#include <type_traits>
namespace crg::ecs {


    template<typename Component>
    struct With{using type = Component; };

    template<typename Component>
    struct is_with : std::false_type{};

    template<typename Component>
    struct is_with<With<Component>> : std::true_type{};

    template<typename Component>
    struct Without{using type = Component; };

    template<typename Component>
    struct is_without : std::false_type{};

    template<typename Component>
    struct is_without<Without<Component>> : std::true_type{};

    template<typename... Ts>
    struct filter_all;

    template<typename... Ts>
    struct filter_all<std::tuple<Ts...>> {
        using type = decltype(std::tuple_cat(
            std::conditional_t<
                !is_with<Ts>::value && !is_without<Ts>::value,
                std::tuple<Ts>,
                std::tuple<>
            >{}...
        ));
    };

}
