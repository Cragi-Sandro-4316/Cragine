#include <Cragine.h>

class Game : public Cragine::Application {
public: 
    Game(){

    }

    ~Game(){

    }

};

Cragine::Application* Cragine::createApplication() {
    printf("porco dio!\n");
    return new Game;
}