#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform LightBuffer {
    vec3 pos;
    vec3 playerPos;
} light;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    
    vec3 LightColor = vec3(1.0, 0.0, 1.0);

    vec3 lightDir = normalize(light.pos - fragPos);
    vec3 norm = normalize(fragNormal);
    vec3 diffuse = max(dot(norm, lightDir), 0.0) * LightColor;

    vec3 viewDir = normalize(light.playerPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 ambient = 0.2 * LightColor;

    float specularS = 0.5;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    vec3 specular = specularS * spec * LightColor;
    outColor = vec4((diffuse + ambient + specular), 1.0);
}