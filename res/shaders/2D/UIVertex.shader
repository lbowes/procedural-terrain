#version 330 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec3 a_Colour;
layout (location = 2) in vec2 a_TexCoord;

out vec3 ourColour;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main(){
	ourColour = a_Colour;
	vec4 worldPosition = modelMatrix * vec4(a_Position, 0.0f, 1.0f);	
	gl_Position = projectionMatrix * worldPosition;
	texCoord = a_TexCoord;
}