#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexturePos;
uniform vec3 Offset;
out vec3 VertexColor;
out  vec2 TextureCoordinates;
uniform mat4 ModelMatrix;
uniform mat4 View;
uniform mat4 Proj;
out vec3 Normal;
out vec3 ModelPos;
void main()
{
  gl_Position = Proj* View * ModelMatrix * vec4(aPos, 1.0);
  VertexColor = aPos;
  ModelPos = vec3(ModelMatrix * vec4(aPos,1.0));
TextureCoordinates = aTexturePos;
  Normal = aNormal;
};

