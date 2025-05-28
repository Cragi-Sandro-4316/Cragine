#include <Cragine.h>

class Game : public Cragine::Application {
public: 
    Game(){

    }

    ~Game(){

    }

};

Cragine::Application* Cragine::createApplication() {
    return new Game;
}