#version 330 core
in vec3 TexCoords;

in vec3 vertexColour;
in vec3 vertexPosition;
out vec4 colour;

uniform vec3 sunDir;
uniform vec3 sunColour;

const float b1 = 0.999f, //0.999 
            b2 = 0.990f, //0.990
            b3 = 0.05f,  //0.05f
            edgeIntensity = 0.5f; //0.5f

void main() {    
    float intensity;
    float betweenSun = max(dot(normalize(vertexPosition), sunDir), 0.0);    

    if(betweenSun > b1)
        intensity = 1.0f;		   
    else if(betweenSun > b2 && betweenSun < b1)
	intensity = edgeIntensity + ((betweenSun - b2) / (b1 - b2)) * (1.0f - edgeIntensity);
    else if(betweenSun > b3 && betweenSun < b2)
        intensity = max((betweenSun - b3) / (b2 - b3) - 0.5f, 0.0f);       	
    else
	intensity = 0.0f;

    intensity = clamp(intensity, 0.0f, 1.0f);

    colour = vec4(mix(vertexColour, sunColour, intensity), 1.0f);
}