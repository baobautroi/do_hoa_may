/*
 * blinn_phong.frag
 * Fragment Shader for Blinn-Phong with Normal Mapping, Shadow Mapping PCF, and Fog
 */

#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 FragPosLightSpace;
    mat3 TBN;
} fs_in;

// Material properties
struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D normalMap;
    float shininess;
    bool hasNormalMap;
};

// Directional light
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform sampler2D shadowMap;

uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogGradient;

// Shadow calculation with PCF (Percentage-Closer Filtering)
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get closest depth value from light's perspective
    float currentDepth = projCoords.z;
    
    // Check if outside shadow map
    if(projCoords.z > 1.0)
        return 0.0;
    
    // Calculate bias to prevent shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    // PCF (Percentage-Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0; // 5x5 kernel
    
    return shadow;
}

// Calculate directional light with Blinn-Phong
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Blinn-Phong specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // Combine results
    vec3 ambient = light.ambient * diffuseTex;
    vec3 diffuse = light.diffuse * diff * diffuseTex;
    vec3 specular = light.specular * spec * specularTex;
    
    // Shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
    
    // Apply shadow only to diffuse and specular, not ambient
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

void main()
{
    // Sample textures
    vec3 diffuseTex = texture(material.diffuseMap, fs_in.TexCoords).rgb;
    vec3 specularTex = texture(material.specularMap, fs_in.TexCoords).rgb;
    
    // Obtain normal from normal map or use vertex normal
    vec3 normal;
    if(material.hasNormalMap) {
        normal = texture(material.normalMap, fs_in.TexCoords).rgb;
        normal = normal * 2.0 - 1.0; // Transform from [0,1] to [-1,1]
        normal = normalize(fs_in.TBN * normal);
    } else {
        normal = normalize(fs_in.Normal);
    }
    
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    
    // Calculate lighting
    vec3 result = CalcDirLight(dirLight, normal, viewDir, diffuseTex, specularTex);
    
    // Apply fog
    float distanceToCamera = length(viewPos - fs_in.FragPos);
    float fogFactor = exp(-pow(distanceToCamera * fogDensity, fogGradient));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    result = mix(fogColor, result, fogFactor);
    
    FragColor = vec4(result, 1.0);
}
