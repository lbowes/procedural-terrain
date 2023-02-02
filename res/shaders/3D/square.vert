#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;

out vec2 texCoord;
out vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){	
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_Position, 1.0f);
	texCoord = a_TexCoord;
 	normal = a_Normal;
}