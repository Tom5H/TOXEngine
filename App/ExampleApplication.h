#ifndef EXAMPLE_APPLICATION_H_
#define EXAMPLE_APPLICATION_H_

#include "IApp.h"

class ExampleApplication : public IApp {
public:
  void start(ITOXEngine *const engine) override;
  void update(ITOXEngine *const engine, void *const uniformBufferMapped,
              const uint32_t width, const uint32_t height) override;
};

#endif // EXAMPLE_APPLICATION_H_
