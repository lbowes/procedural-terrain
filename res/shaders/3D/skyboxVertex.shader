#version 330 core
layout (location = 0) in vec3 position;

out vec3 TexCoords;
out vec3 vertexColour;
out vec3 vertexPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 skyColour;

void main() {
    vertexPosition = position;
    vec4 viewSpace = viewMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * viewSpace;
    TexCoords = position;

    vertexColour = skyColour;
}