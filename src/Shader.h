#pragma once

#include <fstream>
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <iostream>

class Shader
{
public:
	Shader() = default;
	
	std::vector<char> ReadFile(const std::string& filename);
	void CreateShaderModule(VkDevice device, const std::string& vertexPath, const std::string& fragmentPath);

	VkShaderModule& GetVertexShaderModule() { return mVertexShaderModule; }
	VkShaderModule& GetFragShaderModule() { return mFragmentShdaerModule; }
private:
	VkShaderModule mVertexShaderModule;
	VkShaderModule mFragmentShdaerModule;
};