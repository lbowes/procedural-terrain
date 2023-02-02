#version 430 core

layout (location = 0) in vec2 a_XZPosition;
layout (location = 1) in float a_YPosition;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Normal;

out vec3 colour;
out vec3 normal;
out vec3 texCoord;
out vec4 viewSpace;
out vec4 vertexProjection;
out vec3 sunDir;
out vec3 sunColour;

layout (location = 0) uniform mat4 modelMatrix;
layout (location = 1) uniform mat4 viewMatrix;
layout (location = 2) uniform mat4 projectionMatrix;

uniform vec3 sunDirection;
uniform vec3 sunCol;

void main() {
	texCoord = vec3(a_TexCoord.x, a_YPosition / 16.0f, a_TexCoord.y);
	normal = a_Normal;
	vertexProjection = modelMatrix * vec4(a_XZPosition.x, 0, a_XZPosition.y, 1.0f);
	sunDir = sunDirection;
	sunColour = sunCol;

	float diff = max(dot(a_Normal, sunDirection), 0.0);
	vec3 ambient = sunColour * 0.5f; //0.21, 0.16, 0.16
	//0.0f, 0.5f, 0.5f

	colour = vec3(diff) * ambient;

	viewSpace = viewMatrix * modelMatrix * vec4(a_XZPosition.x, a_YPosition, a_XZPosition.y, 1.0f);
	gl_Position = projectionMatrix * viewSpace;
}