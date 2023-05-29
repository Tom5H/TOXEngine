#include "../Engine/TOXEngine.h"
#include "App.h"

int main() {
  App app;
  TOXEngine engine(app);

  try {
    engine.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
