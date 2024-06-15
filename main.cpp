#include <iostream>
#include "glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include <fstream>
#include "glm/vec3.hpp"
#include <sstream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"
#include <unordered_map>
#include <vector>

#include <string>
#include "imgui_stdlib.h"
#include <cstring>
#include <ranges>
#include <charconv>
#include <algorithm>
#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_images.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <filesystem>
float BlendValue = 1.0f;
struct Vertex
{

    glm::vec3 Position;
    glm::vec2 TextureCoords;
    glm::vec3 Normal;

};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
class Mesh
{


public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> Indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(unsigned int ShaderProgram);
private:
    unsigned int VAO, VBO, EBO;

    void setupMesh();

};

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this-> vertices = vertices;
    this->Indices = indices;
    this->textures = textures;
    setupMesh();
}

void Mesh::Draw(unsigned int ShaderProgram) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std:: string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);


        glUniform1i(glGetUniformLocation(ShaderProgram,("material" + name + number).c_str()),i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int),
                 &Indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));

    glBindVertexArray(0);
}
class Model
{
public:
    Model (char* path)
    {
        loadModel(path);
    }
    void Draw(unsigned int ShaderProgrm);
private:
    // model data
    std:: vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;
    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                              std::string typeName);
};

void Model::Draw(unsigned int ShaderProgrm) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(ShaderProgrm);
}

void Model::loadModel(std::string path) {
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std:: vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TextureCoords = vec;
        }
        else
            vertex.TextureCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                                                aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material,
                                                                 aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return {vertices, indices, textures};
}
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
    return textures;
}


void Model::processNode(aiNode * node,const aiScene *scene) {

// process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
// then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}
void FrameResizingCallback(GLFWwindow* window,int width,int height)
{

    glViewport(0,0,width,height);
}

float degrees = 0;
glm::vec3 CameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 CameraUp = glm::vec3(0.0f,1.0f,0.0f);
glm::vec3 CameraPos = glm::vec3(0.0f,0.0f,3.0f);
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool firstMouse = true;
double lastX;
double lastY;
float pitch;
float yaw;
void MouseMoveCallback(GLFWwindow * window,double xposIn,double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    CameraFront = glm::normalize(direction);
}
void ProcessInput(GLFWwindow* window)
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    const float cameraSpeed = 30.0f * deltaTime; // adjust accordingly

    if (glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {

        glfwSetWindowShouldClose(window,1);
    }
    if (glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS && BlendValue < 1)
    {

        BlendValue += 0.001;
    }

    if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS)
    {
CameraPos += cameraSpeed * CameraFront;

    }
    if (glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS)
    {
        CameraPos -= cameraSpeed * CameraFront;

    }

    if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS)
    {
        CameraPos -= cameraSpeed * glm::normalize(glm::cross(CameraFront,CameraUp));


    }

    if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS)
    {
        CameraPos += cameraSpeed *  glm::normalize(glm::cross(CameraFront,CameraUp));

    }
  //  if (glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS )
   // {

    //    degrees += 0.01;
    //}

    if (glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS && BlendValue > 0)
    {

        BlendValue -= 0.001;

    }

  //  if (glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS)
  //  {

     //   degrees -= 0.01;
   // }
    if (BlendValue < 0)
    {
        BlendValue = 0;
    }
    if (BlendValue > 1)
    {
        BlendValue = 1;
    }
}
float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};

float transparentVertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  -1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  -1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  -1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
};
//


float Vertices2[]
{
        0.0f, -0.5f, 0.0f,
        0.9f, -0.5f, 0.0f,
        0.45f, 0.5f, 0.0f
};

unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
};
glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
};
glm::vec3 LightPositions[]
{

    {-30.0f,50.0f,-10.0f},
    {-60.0f,50.0f,-50.0f},
{-16.0f,50.0f,25.0f}



};
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *msg, const void *data)
{
    std::string _source;
    std::string _type;
  std::string _severity;


    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "UNKNOWN";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;

        default:
            _severity = "UNKNOWN";
            break;
    }
char* _type2 = _type.data();
    char* _severity2 = _severity.data();
    char* _source2 = _source.data();

    printf("%d: %s of %s severity, raised from %s: %s\n",
           id, _type2, _severity2, _source2, msg);
}

// During init, enable debug output

float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
};


int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920,1080,"V2 Yahooo",NULL,NULL);

glm::mat4 TransformMatrix = glm::mat4(1.0f);
    TransformMatrix = glm::translate(TransformMatrix,glm::vec3(0.0f,0.5f,0.0f));




glm::mat4 ProjectionMatrix;
ProjectionMatrix = glm::perspective(glm::radians(120.0f),float(1920)/float(1080),0.1f,1000.0f);
    if (window == NULL)
    {

        std::cout << "Failed to create GLFW window" << std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    {
        std::cout << "Failed to create GLAD" << std::endl;
        return -1;
    }

    std::string Path = "../Survival_BackPack_2.fbx";


    std::ifstream VertexShaderGrassFile("../QuadVertex.vert");
    std::string VertexShaderGrassSource;
    std::stringstream streamgrassshader;
    streamgrassshader << VertexShaderGrassFile.rdbuf();
    VertexShaderGrassSource = streamgrassshader.str();
    const char* VertexShaderGrassSourceCSTR = VertexShaderGrassSource.c_str();
    unsigned int VertexShaderGrass;
    VertexShaderGrass = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShaderGrass,1,&VertexShaderGrassSourceCSTR,NULL);
    glCompileShader(VertexShaderGrass);

    std::ifstream FragmentShaderFileGrass("../FragmentShaderQuad.frag");
    std::string FragmentShaderSourceGrass;
    std::stringstream  strstreamFragmentGrass;
    strstreamFragmentGrass << FragmentShaderFileGrass.rdbuf();
    FragmentShaderSourceGrass = strstreamFragmentGrass.str();
    const char* FragmentShaderSourceCSTRGrass = FragmentShaderSourceGrass.c_str();
    unsigned int FragmentShaderGrass;
    FragmentShaderGrass = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(FragmentShaderGrass,1,&FragmentShaderSourceCSTRGrass,NULL);
    glCompileShader(FragmentShaderGrass);



    glfwSetFramebufferSizeCallback(window,FrameResizingCallback);
    std::ifstream VertexShaderFile("../Ve.vert");
    std::string VertexShaderSource;
    std::stringstream  strstream2;
    strstream2 << VertexShaderFile.rdbuf();
    VertexShaderSource = strstream2.str();
    const char* VertexShaderCourseCstr =  VertexShaderSource.c_str();
    unsigned int VertexShader;
    VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader,1,&VertexShaderCourseCstr,NULL);
    glCompileShader(VertexShader);
    std::ifstream FragmentShaderFile("../FragmentShader.frag");
    std::string FragmentShaderSource;
   std::stringstream  strstream;
   strstream << FragmentShaderFile.rdbuf();
   FragmentShaderSource = strstream.str();
    const char* FragmentShaderSourceCSTR = FragmentShaderSource.c_str();
    unsigned int FragmentShader;
    FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(FragmentShader,1,&FragmentShaderSourceCSTR,NULL);
    glCompileShader(FragmentShader);

    std::ifstream FragmentShaderFile2("../FragmentShader2.frag");
    std::string FragmentShaderSource2;
    std::stringstream  strstream3;
    strstream3 << FragmentShaderFile2.rdbuf();
    FragmentShaderSource = strstream3.str();
    const char* FragmentShaderSourceCSTR2 = FragmentShaderSource.c_str();
    unsigned int FragmentShader2;
    FragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(FragmentShader2,1,&FragmentShaderSourceCSTR2,NULL);
    glCompileShader(FragmentShader2);
    stbi_set_flip_vertically_on_load(true);
    GLint isCompiled = 0;
    glGetShaderiv(VertexShaderGrass, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(VertexShaderGrass, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(VertexShaderGrass, maxLength, &maxLength, &errorLog[0]);

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        printf("%s\n", &errorLog[0]);
        glDeleteShader(VertexShaderGrass); // Don't leak the shader.
        return -1 ;
    }
    //Basically Creates the shader program the shader program attaches all fragment and vertex shaders and is what makes it work all shaders attached to a program when used will be executed
    glfwSetCursorPosCallback(window,MouseMoveCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    unsigned int ShaderProgramGrass;
    ShaderProgramGrass= glCreateProgram();
    glAttachShader(ShaderProgramGrass,VertexShaderGrass);
    glAttachShader(ShaderProgramGrass,FragmentShaderGrass);
    glLinkProgram(ShaderProgramGrass);

    glDeleteShader(VertexShaderGrass);
    glDeleteShader(FragmentShaderGrass);
    unsigned int ShaderProgram;
    ShaderProgram = glCreateProgram();

    glAttachShader(ShaderProgram,VertexShader);
    glAttachShader(ShaderProgram,FragmentShader);
    glLinkProgram(ShaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ShaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    unsigned int ShaderProgram2;
    ShaderProgram2 = glCreateProgram();
    glAttachShader(ShaderProgram2,VertexShader);
    glAttachShader(ShaderProgram2,FragmentShader2);
    glLinkProgram(ShaderProgram2);

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader2);

    unsigned int VAO;
    unsigned int VBO;
    unsigned int VAO2;
    unsigned int VBO2;


    unsigned int VAOQuad;
    unsigned int VBOQuad;
    glGenVertexArrays(1,&VAOQuad);
    glGenVertexArrays(1,&VBOQuad);
    glBindVertexArray(VAOQuad);
    glBindBuffer(GL_ARRAY_BUFFER,VBOQuad);

    glBufferData(GL_ARRAY_BUFFER,sizeof(quadVertices),quadVertices,GL_STATIC_DRAW);


    glVertexAttribPointer(0,2 ,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2 ,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1,3 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(3*sizeof(float)));
    // glEnableVertexAttribArray(1);

    glVertexAttribPointer(1,3 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
glGenVertexArrays(1,&VAO2);
glGenBuffers(1,&VBO2);
glBindVertexArray(VAO2);
glBindBuffer(GL_ARRAY_BUFFER,VBO2);
glBufferData(GL_ARRAY_BUFFER,sizeof(Vertices2),Vertices2,GL_STATIC_DRAW);
glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
unsigned int EBO;
glGenBuffers(1,&EBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
unsigned int Texture;
glGenTextures(1,&Texture);
glBindTexture(GL_TEXTURE_2D,Texture);
int height;
int width;
int colorchannels;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
unsigned char* Data = stbi_load("../container2.png",&width,&height,&colorchannels,0);
if (Data)
{

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
    glGenerateMipmap(GL_TEXTURE_2D);
}
    stbi_image_free(Data);
    unsigned int Texture2;
    glGenTextures(1,&Texture2);
    glBindTexture(GL_TEXTURE_2D,Texture2);

    int height2;
    int width2;
    int colorchannels2;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* Data2 = stbi_load("../container2_specular.png",&width2,&height2,&colorchannels2,0);
    if (Data2)
    {

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(Data2);
    glUseProgram(ShaderProgram);



    unsigned int TextureGrass;
    glGenTextures(1,&TextureGrass);
    glBindTexture(GL_TEXTURE_2D,TextureGrass);

    int heightGrass;
    int widthGrass;
    int colorchannelsGrass;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* DataGrass = stbi_load("../grass (1).png",&widthGrass,&heightGrass,&colorchannelsGrass,0);
    if (DataGrass)
    {

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthGrass, heightGrass, 0, GL_RGBA, GL_UNSIGNED_BYTE, DataGrass);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(DataGrass);
    glUniform3f(glGetUniformLocation(ShaderProgram,"LightPos"),0.5f,0.0f,-1.0f);
    glUniform1i(glGetUniformLocation(ShaderProgram, "Texture1"), 0);
    glUniform1i(glGetUniformLocation(ShaderProgram, "Texture2"), 1);
TransformMatrix = glm::translate(TransformMatrix,glm::vec3(5.0f,0.0f,0.0f));
;

    glEnable(GL_DEPTH_TEST);
    std::string stringg = "../Models/Sponza/sponza.obj";

    Model sponza(stringg.data());
unsigned int FBO;
glGenFramebuffers(1,&FBO);
glBindFramebuffer(GL_FRAMEBUFFER,FBO);
    unsigned int textureFBO;
    glGenTextures(1, &textureFBO);
    glBindTexture(GL_TEXTURE_2D, textureFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFBO, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);



    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        //VICTORY!
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    while (!glfwWindowShouldClose(window))
    {

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);


        glDebugMessageCallback(GLDebugMessageCallback, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glEnable(GL_DEPTH_TEST);

        ProcessInput(window);
        glClearColor(0.529, 0.808, 0.922,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 ViewMatrix  = glm::mat4(1.0f);
        //     ViewMatrix = glm::translate(ViewMatrix,CameraPos);
        // ViewMatrix = glm::rotate(ViewMatrix,glm::radians(degrees),glm::vec3(1.0f,0.0f,0.0f));
        ViewMatrix = glm::lookAt(CameraPos,CameraPos+CameraFront,CameraUp);

    //  float  angle2 = glfwGetTime() * 25.0f;
      //  TransformMatrix = glm::rotate(TransformMatrix, glm::radians(angle2), glm::vec3(0.5f, 1.0f, 0.0f));


        glUseProgram(ShaderProgram);
        glUniform3f(glGetUniformLocation(ShaderProgram,"CameraPos"),CameraPos.x,CameraPos.y,CameraPos.z);

        glUniform1f(glGetUniformLocation(ShaderProgram,"material.diffuse"),0);
        glUniform1f(glGetUniformLocation(ShaderProgram,"material.specular"),1);
        glUniform3f(glGetUniformLocation(ShaderProgram,"material.specular"),0.5f,0.5f,0.5f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"material.shininess"),32.0f);

    glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[0].position"),LightPositions[0].x,LightPositions[0].y,LightPositions[0].z);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[1].position"),LightPositions[1].x,LightPositions[1].y,LightPositions[1].z);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[2].position"),LightPositions[2].x,LightPositions[2].y,LightPositions[2].z);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[0].ambient"),0.5f,0.3f,0.2f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[1].ambient"),0.5f,0.3f,0.2f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[2].ambient"),0.5f,0.3f,0.2f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[0].Quadratic"),0.0028f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[1].Quadratic"),0.032f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[2].Quadratic"),0.032f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[0].linear"),0.027);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[1].linear"),0.09f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[2].linear"),0.09f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[0].constant"),1.0f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[1].constant"),1.0f);
        glUniform1f(glGetUniformLocation(ShaderProgram,"pointLights[2].constant"),1.0f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[0].diffuse"), 0.5f,0.3f,0.2f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[1].diffuse"), 0.5f,0.3f,0.2f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[2].diffuse"), 0.5f,0.3f,0.2f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram,"ModelMatrix"),1,GL_FALSE,glm::value_ptr(TransformMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram,"View"),1,GL_FALSE,glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram,"Proj"),1,GL_FALSE,glm::value_ptr(ProjectionMatrix));
        std::cout << BlendValue  << std::endl;
        glUniform1f(glGetUniformLocation(ShaderProgram,"BlendValue"),BlendValue);
        float Timevalue = glfwGetTime();
        float greenvalue = (sin(Timevalue)/2.0f) + 0.5f;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture2);

        glUniform4f(glGetUniformLocation(ShaderProgram,"FragmentColorUniform"),0.0f,greenvalue,0.0f,1.0f);
        glUniform3f(glGetUniformLocation(ShaderProgram,"Offset"),0.0,0.0f,0.0f);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBindVertexArray(VAO);


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);







        TransformMatrix = glm::mat4(1.0f);
        TransformMatrix = glm::scale(TransformMatrix,glm::vec3(0.1f,0.1f,0.1f));
        TransformMatrix = glm::translate(TransformMatrix,glm::vec3(5.0f,-300.0f,0.0f));

        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram,"ModelMatrix"),1,GL_FALSE,glm::value_ptr(TransformMatrix));
        sponza.Draw(ShaderProgram);
        glUseProgram(ShaderProgram2);
        glBindVertexArray(VAO2);
        glVertexAttribPointer(0,3 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1,3 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(3*sizeof(float)));
        // glEnableVertexAttribArray(1);

        glVertexAttribPointer(1,3 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2,2 ,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);
        glm::mat4 LightMatrix = glm::mat4(1.0f);
        LightMatrix = glm::translate(LightMatrix,LightPositions[0]);
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"ModelMatrix"),1,GL_FALSE,glm::value_ptr(LightMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"View"),1,GL_FALSE,glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"Proj"),1,GL_FALSE,glm::value_ptr(ProjectionMatrix));
        glDrawArrays(GL_TRIANGLES,0,36);
        LightMatrix = glm::mat4(1.0f);
        LightMatrix = glm::translate(LightMatrix,LightPositions[1]);

        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"ModelMatrix"),1,GL_FALSE,glm::value_ptr(LightMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"View"),1,GL_FALSE,glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"Proj"),1,GL_FALSE,glm::value_ptr(ProjectionMatrix));
        glDrawArrays(GL_TRIANGLES,0,36);
        LightMatrix = glm::mat4(1.0f);
        LightMatrix = glm::translate(LightMatrix,LightPositions[2]);
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"ModelMatrix"),1,GL_FALSE,glm::value_ptr(LightMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"View"),1,GL_FALSE,glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2,"Proj"),1,GL_FALSE,glm::value_ptr(ProjectionMatrix));
        glDrawArrays(GL_TRIANGLES,0,36);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.529, 0.808, 0.922,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(ShaderProgramGrass);
        glBindTexture(GL_TEXTURE_2D,textureFBO);
        glUniform1i(glGetUniformLocation(ShaderProgramGrass,"Texture"),0);
        glActiveTexture(GL_TEXTURE0);
        glBindBuffer(GL_ARRAY_BUFFER,VBOQuad);
        glBindVertexArray(VAOQuad);

        glDrawArrays(GL_TRIANGLES,0,6);






        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(ShaderProgram);

    glfwTerminate();
    return 0;
}