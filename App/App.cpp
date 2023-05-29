#include "App.h"

#include "../Engine/TOXEngine.h"

#include <chrono>
#include <cstring>

void App::start() {
  // todo generate geomety here (in child class)
  engine->loadModel("../resources/models/viking_room.obj", "../resources/textures/viking_room.png");
  engine->loadRTXModel("../resources/models/CornellBox-Original.obj");
}

void App::update(void *uniformBufferMapped, uint32_t width, uint32_t height) {
  static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
        glm::radians(45.0f),
        width / (float)height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}
