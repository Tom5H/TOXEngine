#ifndef APP_H_
#define APP_H_

#include <cstdint>

class TOXEngine;

class App {
public:
  App() = default;
  virtual ~App() = default;

  virtual void start(TOXEngine *const engine);
  virtual void update(TOXEngine *const engine, void *const uniformBufferMapped,
                      const uint32_t width, const uint32_t height);
};

#endif // APP_H_
