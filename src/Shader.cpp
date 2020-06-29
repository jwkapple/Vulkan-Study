
#include "Shader.h"

std::vector<char> Shader::ReadFile(const std::string & filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file!");
	}

	std::cerr << "Successfuly opend file" << std::endl;

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void Shader::CreateShaderModule(VkDevice device, const std::string & vertexPath, const std::string & fragmentPath)
{
	auto vertexCode = ReadFile(vertexPath);
	auto fragCode = ReadFile(fragmentPath);

	VkShaderModuleCreateInfo vertexCreateInfo{};
	vertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexCreateInfo.codeSize = vertexCode.size();
	vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data());

	if (vkCreateShaderModule(device, &vertexCreateInfo, nullptr, &mVertexShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vertex Shader Module!");
	}

	VkShaderModuleCreateInfo fragCreateInfo{};
	fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragCreateInfo.codeSize = fragCode.size();
	fragCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
	
	if (vkCreateShaderModule(device, &fragCreateInfo, nullptr, &mFragmentShdaerModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Fragment Shader Module!");
	}
}
