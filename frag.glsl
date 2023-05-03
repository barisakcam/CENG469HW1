#version 420 core
#define MAX_LIGHT_COUNT 5

vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 kd = vec3(0.8, 0.8, 0.8);   // diffuse reflectance coefficient
vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient

struct Light {
	vec3 position;
	vec3 intensity;
};

uniform vec3 eyePos;
uniform int lightCount;
uniform Light lights[MAX_LIGHT_COUNT];

vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
vec3 specularColor = vec3(0.0, 0.0, 0.0);

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
	for (int i = 0; i < lightCount; i ++) 
	{
		vec3 l = lights[i].position - vec3(fragWorldPos);
		float r = length(l);
		vec3 E = (lights[i].intensity  / (r * r));
		vec3 L = normalize(l);
		vec3 V = normalize(eyePos - vec3(fragWorldPos));
		vec3 H = normalize(L + V);
		vec3 N = normalize(fragWorldNor);

		if (!gl_FrontFacing) // To make both sides of triangles reflect color
		{
			N = - N;
		}

		float NdotL = dot(N, L); // for diffuse component
		float NdotH = dot(N, H); // for specular component

		diffuseColor += E * kd * max(0, NdotL);
		specularColor += E * ks * pow(max(0, NdotH), 400);
	}
    
	vec3 ambientColor = Iamb * ka;

	fragColor = vec4(diffuseColor + specularColor + ambientColor, 1);
}