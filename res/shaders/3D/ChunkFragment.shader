#version 430 core

in vec3 colour;
in vec3 texCoord;
in vec4 viewSpace;
in vec3 normal;
in vec4 vertexProjection;
in vec3 sunDir;
in vec3 sunColour;

out vec4 outColour;

uniform sampler2D ourTexture;
uniform vec3 skyColour;
uniform bool usesFog;
uniform float fogDensity;
uniform float fogGradient;

const float b1 = 1.0f, //0.999 
            b2 = 1.0f, //0.990
            b3 = 0.05f,   //0.05f
            edgeIntensity = 0.5f; //0.5f

vec3 getTriplanarBlend(vec3 norm) {
	vec3 blending = abs(norm);
	blending = normalize(max(blending, 0.00001));
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b, b, b);
	return blending;
}

vec4 getTextureColour() {
	vec3 blending = getTriplanarBlend(normal);
	vec4 xaxis = texture2D(ourTexture, texCoord.yz);
        vec4 yaxis = texture2D(ourTexture, texCoord.xz);
    	vec4 zaxis = texture2D(ourTexture, texCoord.xy);

	return xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
}

float getFogModifier() {
	float distance = length(viewSpace);	
	float fogModifier = exp(-pow((distance * fogDensity), fogGradient));
	fogModifier = clamp(fogModifier, 0.0, 1.0);
	return fogModifier;
}

float getIntensity() {
    	float intensity;
    	float betweenSun = max(dot(-normalize(vertexProjection.xyz), sunDir), 0.0);    

    	if(betweenSun > b1)
        	intensity = 1.0f;		   
    	else if(betweenSun > b2 && betweenSun < b1)
		intensity = edgeIntensity + ((betweenSun - b2) / (b1 - b2)) * (1.0f - edgeIntensity);
	else if(betweenSun > b3 && betweenSun < b2)
	        intensity = max((betweenSun - b3) / (b2 - b3) - 0.5f, 0.0f);       	
    	else
		intensity = 0.0f;

    	intensity = clamp(intensity, 0.0f, 1.0f);
	return intensity;
}

void main() {
	vec3 finalColour;

	vec4 diffuseColour = getTextureColour() * vec4(colour, 1.0f);
	
	vec3 sunBlendColour = mix(skyColour, sunColour, getIntensity()); 

	if(usesFog) { finalColour = mix(sunBlendColour, diffuseColour.rgb, getFogModifier()); }
	else { finalColour = diffuseColour.rgb; }

	outColour = vec4(finalColour, 1.0f);
}