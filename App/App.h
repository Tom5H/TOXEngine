#ifndef APP_H_
#define APP_H_

#include <cstdint>

class ITOXEngine;

class App {
public:
  App() = default;
  virtual ~App() = default;

  virtual void start(ITOXEngine *const engine);
  virtual void update(ITOXEngine *const engine, void *const uniformBufferMapped,
                      const uint32_t width, const uint32_t height);
};

#endif // APP_H_
