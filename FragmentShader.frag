
#version 330 core
out vec4 FragColor;
in vec3 VertexColor;
in vec2 TextureCoordinates;
uniform sampler2D Texture1;
uniform sampler2D Texture2;
uniform float BlendValue;
in vec3 Normal;
uniform vec3 LightPos;
in vec3 ModelPos;
uniform vec3 CameraPos;
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};
struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float Quadratic;

};
struct DirectionalLight
{
    vec3 direction; 
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
vec3 direction;
    float cutoffangle;
    float outercutoffangle;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float Quadratic;

};
//uniform Light light;
uniform DirectionalLight light2;
uniform SpotLight light;


uniform Material material;

 vec3 calculatespotlight(SpotLight spotlight, vec3 normal, vec3 viewdir)
{
    float distancee = length(light.position - ModelPos);
    float attenuation = 1.0/(light.constant+light.linear * distancee + light.Quadratic * (distancee * distancee));
    float specstrength = 0.5;
    vec3 norm = normalize(Normal);
    vec3 LightDir = normalize(light.position - ModelPos);
    float theta = max(dot(LightDir,normalize(-light.direction)),0.0);
    float epsilon = (light.cutoffangle- light.outercutoffangle);
    float intensity = clamp((theta - light.outercutoffangle) / epsilon, 0.0, 1.0);


    float DiffuseScaler = max(dot(LightDir, norm), 0.0);

    vec3 LightColor = vec3(1.0f, 1.0f, 1.0f);

    vec3 DiffuseColor = light.diffuse * DiffuseScaler * vec3(texture(material.diffuse, TextureCoordinates)); ;

    float AmbientStrength = 0.1f;
    vec3 Ambient = light.ambient * vec3(texture(material.diffuse, TextureCoordinates));
    vec3 CameraDirection = normalize(CameraPos - ModelPos);
    vec3 ReflectDirection = reflect(-LightDir, Normal);
    float SpecScaler = pow(max(dot(CameraDirection, ReflectDirection), 0.0), material.shininess);
    vec3 SpecularColor = light.specular * SpecScaler * vec3(texture(material.specular, TextureCoordinates));
    DiffuseColor  *= intensity;
    SpecularColor *= intensity;
    Ambient *= attenuation;
    DiffuseColor *= attenuation;
    SpecularColor *= attenuation;

    vec3 ResultColor = (Ambient + DiffuseColor + SpecularColor);

    // FragColor = mix(texture(Texture1,TextureCoordinates),texture(Texture2,vec2(-TextureCoordinates.x,TextureCoordinates.y)),BlendValue);
    return ResultColor;


}
#define NR_SPOT_LIGHTS 3
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
#define NR_POINT_LIGHTS 3
uniform Light pointLights[NR_POINT_LIGHTS];
vec3 calculatepointlight(Light pointlight, vec3 normal, vec3 viewdir)
{
    float distancee = length(pointlight.position - ModelPos);
    float attenuation = 1.0/(pointlight.constant+pointlight.linear * distancee + pointlight.Quadratic * (distancee * distancee));
    float specstrength = 0.5;
    vec3 norm = normalize(Normal);
    vec3 LightDir = normalize(pointlight.position - ModelPos);


    float DiffuseScaler = max(dot(LightDir, norm), 0.0);

    vec3 LightColor = vec3(1.0f, 1.0f, 1.0f);

    vec3 DiffuseColor = pointlight.diffuse * DiffuseScaler * vec3(texture(material.diffuse, TextureCoordinates)); ;

    float AmbientStrength = 0.1f;
    vec3 Ambient = pointlight.ambient * vec3(texture(material.diffuse, TextureCoordinates));
    vec3 CameraDirection = normalize(CameraPos - ModelPos);
    vec3 ReflectDirection = reflect(-LightDir, Normal);
    float SpecScaler = pow(max(dot(CameraDirection, ReflectDirection), 0.0), material.shininess);
    vec3 SpecularColor = pointlight.specular * SpecScaler * vec3(texture(material.specular, TextureCoordinates));
    Ambient *= attenuation;
    DiffuseColor *= attenuation;
    SpecularColor *= attenuation;

    vec3 ResultColor = (Ambient + DiffuseColor + SpecularColor);

    // FragColor = mix(texture(Texture1,TextureCoordinates),texture(Texture2,vec2(-TextureCoordinates.x,TextureCoordinates.y)),BlendValue);
    return ResultColor;



}
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TextureCoordinates));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TextureCoordinates));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TextureCoordinates));
    return (ambient + diffuse + specular);
}
void main()
{

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(CameraPos - ModelPos);

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(light2,norm,viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
    result += calculatepointlight(pointLights[i], norm, viewDir);
        }

    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);


}