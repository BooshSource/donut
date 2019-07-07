#include <iostream>
#include <string>
#include <Game.h>
#include <Pure3D/Pure3D.h>
#include <SDL.h>
#include <Window.h>
#include <glad/glad.h>

namespace Donut {

const std::string kWindowTitle = "donut";

Game::Game(int argc, char** argv) {
    const int windowWidth = 1280, windowHeight = 1024;
    _window = std::make_unique<Window>(kWindowTitle, windowWidth, windowHeight);

    if (std::filesystem::exists("wrench.p3d")) {
        File file("wrench.p3d", FileMode::Read);
        Pure3D::Pure3D p3d;
        p3d.LoadFromFile(file);
        file.Close();

        const auto& chunks = p3d.GetRoot().GetChildren();
        for (const auto& chunk : chunks) {
            std::cout << chunk->GetType() << std::endl;
        }
    }
}

Game::~Game() {
    SDL_Quit();
}

void Game::Run() {
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _window->Swap();
    }
}

} // namespace Donut
