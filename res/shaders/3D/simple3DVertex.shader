#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Colour;
layout (location = 2) in vec2 a_TexCoord;

out vec3 ourColour;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){
	ourColour = vec3(1.0f, 1.0f, 1.0f);

	vec4 worldPosition = modelMatrix * vec4(a_Position, 1.0f);
	vec4 viewPosition = viewMatrix * worldPosition;	
	gl_Position = projectionMatrix * viewPosition;      	
	texCoord = a_TexCoord;
}

