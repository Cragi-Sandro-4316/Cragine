#pragma once
#include "Ecs/Ecs.h"
#include "Ecs/SystemParams/QueryParam.h"
#include "utils/Logger.h"

namespace crg {

    struct Foo {
        int x = 0;
    };
    struct Marker {};

    static void spawn(Commands commands) {

        commands.spawn(
            Foo {
                .x = 0
            },
            Marker{}
        );

        for (int i = 1; i < 11; i++) {
            commands.spawn(Foo {
                .x = i
            });
        }
    }

    static void despawn(
        Commands commands,
        Query<Entity> q
    ) {
        int i = 0;
        for (auto [ent] : q) {
            i++;
            if (i == 5) {
                commands.despawn(ent);
            }
        }
    }

    static void print(Query<Foo> q) {
        LOG_CORE_WARNING("Printing...");

        int i = 0;
        for (auto [foo] : q) {
            i++;
            LOG_CORE_TRACE("Foo[{}]: {}", i, foo.x);
        }
    }

}
