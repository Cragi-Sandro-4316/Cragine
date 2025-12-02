#pragma once

#include "Ecs/Components/ComponentSignature.h"

namespace crg::ecs {

    class QueryHelper {
    public:
        template <typename... Components>
        static void createSignature(
            ComponentSignature& components,
            ComponentSignature& withFilter,
            ComponentSignature& withoutFilter
        ) {

            auto lambda = [&](auto t) {
                using T = decltype(t);
                std::type_index tt = typeid(T);


                if constexpr (is_with<T>::value) {
                    using Inner = typename T::type;

                    LOG_CORE_TRACE("Found WITH filter {}", tt.name());
                    withFilter.emplace_back(
                        typeid(Inner),
                        sizeof(Inner),
                        alignof(Inner)
                    );
                }
                else if constexpr (is_without<T>::value) {
                    using Inner = typename T::type;

                    LOG_CORE_TRACE("Found WITHOUT filter {}", tt.name());
                    withoutFilter.emplace_back(
                        typeid(Inner),
                        sizeof(Inner),
                        alignof(Inner)
                    );
                }
                else {
                    LOG_CORE_TRACE("Queried COMPONENT: {}", tt.name());

                    components.emplace_back(
                        typeid(T),
                        sizeof(T),
                        alignof(T)
                    );
                }

            };

            (lambda(Components{}), ...);

        }

    };

}
