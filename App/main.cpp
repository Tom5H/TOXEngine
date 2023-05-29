#include "../Engine/TOXEngine.h"
#include "ExampleApplication.h"

int main() {
  ExampleApplication app;
  TOXEngine engine(app);

  try {
    engine.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
