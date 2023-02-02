#version 330 core

out vec4 colour;

in vec2 texCoord;
in vec3 normal;

uniform sampler2D ourTexture;

vec3 sunDirection = vec3(0.5f, 0.5f, 0.5f);

void main(){
	colour = texture(ourTexture, texCoord) * ((dot(normal, sunDirection)) + 1.0f) * 0.5f;
}
