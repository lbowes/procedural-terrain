#version 330 core
in vec3 ourColour;
in vec2 texCoord;

out vec4 colour;

uniform sampler2D ourTexture;

void main(){
	colour = texture(ourTexture, texCoord) * vec4(ourColour, 1.0f);
}