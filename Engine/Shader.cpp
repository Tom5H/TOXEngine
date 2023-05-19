#include "Shader.h"

#include "TOXEngine.h"
#include "Utils.h"
#include <vector>

Shader::Shader(Context &context, const std::string path) : context(context) {
  const std::vector<char> code = readFile(path);

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(context.device->get(), &createInfo, nullptr,
                           &shader) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}

Shader::~Shader() {
  vkDestroyShaderModule(context.device->get(), shader, nullptr);
}
