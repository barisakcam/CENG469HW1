#version 420 core
#define MAX_BEZIER_COUNT 36

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

uniform int samples;
uniform int horizCount;
uniform int vertiCount;
uniform float coordMultiplier;
uniform float scale;
uniform mat4 bezier[MAX_BEZIER_COUNT];

vec3 vertex;
vec3 normal;
mat4 Mb = mat4(
   -1.0, 3.0, -3.0, 1.0,
   3.0, -6.0, 3.0, 0.0,
   -3.0, 3.0, 0.0, 0.0,
   1.0, 0.0, 0.0, 0.0
);
mat4 bezierX = mat4(
   -0.5, -0.5, -0.5, -0.5,
   -0.1667, -0.1667, -0.1667, -0.1667,
   0.1667, 0.1667, 0.1667, 0.1667,
   0.5, 0.5, 0.5, 0.5
);
mat4 bezierY = mat4(
   0.5, 0.1667, -0.1667, -0.5,
   0.5, 0.1667, -0.1667, -0.5,
   0.5, 0.1667, -0.1667, -0.5,
   0.5, 0.1667, -0.1667, -0.5
);

out vec4 fragWorldPos;
out vec3 fragWorldNor;

float Qx(float s, float t)
{
    return dot(vec4(s * s * s, s * s, s, 1), Mb * bezierX * transpose(Mb) *  vec4(t * t * t, t * t, t, 1)) * coordMultiplier * scale;
}

float Qy(float s, float t)
{
    return dot(vec4(s * s * s, s * s, s, 1), Mb * bezierY * transpose(Mb) *  vec4(t * t * t, t * t, t, 1)) * coordMultiplier * scale;
}

float Qz(float s, float t)
{
    return dot(vec4(s * s * s, s * s, s, 1), Mb * bezier[gl_InstanceID] * transpose(Mb) *  vec4(t * t * t, t * t, t, 1));
}

float Qxds(float s, float t)
{
    return dot(vec4(3 * s * s, 2 * s, 1, 0), Mb * bezierX * transpose(Mb) *  vec4(t * t * t, t * t, t, 1)) * coordMultiplier * scale;
}

float Qyds(float s, float t)
{
    return dot(vec4(3 * s * s, 2 * s, 1, 0), Mb * bezierY * transpose(Mb) *  vec4(t * t * t, t * t, t, 1)) * coordMultiplier * scale;
}

float Qzds(float s, float t)
{
    return dot(vec4(3 * s * s, 2 * s, 1, 0), Mb * bezier[gl_InstanceID] * transpose(Mb) *  vec4(t * t * t, t * t, t, 1));
}

float Qxdt(float s, float t)
{
    return dot(vec4(s * s * s, s * s, s, 1), Mb * bezierX * transpose(Mb) *  vec4(3 * t * t, 2 * t, 1, 0)) * coordMultiplier * scale;
}

float Qydt(float s, float t)
{
    return dot(vec4(s * s * s, s * s, s, 1), Mb * bezierY * transpose(Mb) *  vec4(3 * t * t, 2 * t, 1, 0)) * coordMultiplier * scale;
}

float Qzdt(float s, float t)
{
    return dot(vec4(s * s * s, s * s, s, 1), Mb * bezier[gl_InstanceID] * transpose(Mb) *  vec4(3 * t * t, 2 * t, 1, 0));
}

void main(void)
{
    float s = (mod(float(gl_VertexID), float(samples))) / (float(samples) - 1);
    float t = (((gl_VertexID / samples) / (float(samples) - 1)));
    
    float l = coordMultiplier * scale * 0.5 - 0.5 * coordMultiplier;
    vertex.x = Qx(t, s) + l + mod(gl_InstanceID, horizCount) * scale * coordMultiplier;
    vertex.y = Qy(t, s) + l + (vertiCount - 1) * scale * coordMultiplier - (gl_InstanceID / horizCount) * scale * coordMultiplier;
    vertex.z = Qz(t, s);

    normal = cross(vec3(Qxds(t, s), Qyds(t, s), Qzds(t, s)), vec3(Qxdt(t, s), Qydt(t, s), Qzdt(t, s)));

    fragWorldPos = modelingMatrix * vec4(vertex, 1);
	fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * normal;

    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(vertex, 1);
}