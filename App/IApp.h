#ifndef TOXENGINE_APP_IAPP_H_
#define TOXENGINE_APP_IAPP_H_

#include <string>

class ITOXEngine;

class IApp {
public:
  IApp() = default;
  virtual ~IApp() = default;

  // called once at engine startup
  // load models here
  virtual void start(ITOXEngine *const engine) = 0;

  // called once every frame
  virtual void update(ITOXEngine *const engine, void *const uniformBufferMapped,
                      const uint32_t width, const uint32_t height) = 0;
};

#endif // TOXENGINE_APP_IAPP_H_
