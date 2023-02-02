#version 330 core
in vec3 ourColour;
in vec2 texCoord;

out vec4 colour;

uniform sampler2D ourTexture;

void main(){
	colour = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}