#ifndef APP_H_
#define APP_H_

#include <cstdint>

class TOXEngine;

class App {
public:
  App(TOXEngine *engine) : engine(engine) {}
  
  void start();
  void update(void *uniformBufferMapped, uint32_t width, uint32_t height);
private:
  TOXEngine *engine;
};

#endif // APP_H_
