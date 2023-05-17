#include "Shader.h"

#include "TOXEngine.h"
#include "Utils.h"
#include <vector>

Shader::Shader(TOXEngine *engine, const std::string path) : engine(engine) {
  const std::vector<char> code = readFile(path);

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(engine->getDevice()->get(), &createInfo, nullptr,
                           &shader) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}

Shader::~Shader() {
  vkDestroyShaderModule(engine->getDevice()->get(), shader, nullptr);
}
