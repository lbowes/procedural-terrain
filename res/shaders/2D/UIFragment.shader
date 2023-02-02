#version 330 core

in vec3 ourColour;
in vec2 texCoord;

out vec4 colour;

uniform sampler2D ourTexture;

void main(){
	colour = texture(ourTexture, texCoord) * vec4(ourColour, 1.0f);
	//colour = vec4(0.5f, 0.5f, 0.5f, 1.0f);
}