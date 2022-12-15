	#version 330 // GLSL 3.30
	// an ultra simple glsl vertex shader
struct ATTRIBUTES {
	// kd - diffuse reflectivity // d - dissolve (transparency) 
	vec3 kd; float d;
	// Ks - specular reflectivity // Ns - specular exponent
	vec3 Ks;  float Ns;
	// Ka - ambient reflectivity // shaprness - local reflection map sharpness
	vec3 Ka; float sharpness; 
	// tf - transmission filter // Ni - optical density (index of refraction)
	vec3 Tf;  float Ni; 
	// Ke - emissive reflectivity // illum - illumination model
	vec3 Ke; uint illum;
};

layout(row_major) uniform UBO_DATA
{
	vec4 lightDir, lightCol, lightAmbient, camPos;
	mat4 uViewMatrix, uProjMatrix;
	mat4 world;
	ATTRIBUTES material;
};

// Values taken from our vertexattribpointers in our model header
layout (location = 0) in vec3 local_pos;
layout (location = 1) in vec3 uvw;
layout (location = 2) in vec3 norm;

// out values that will be used in our pixel shader
out vec3 Normal;
out vec3 outUvw;
out vec3 pixel_pos;


void main()
{
	mat4 view_proj = uViewMatrix * uProjMatrix;
	view_proj = transpose(view_proj);
	gl_Position = view_proj * (vec4(local_pos, 1) * world);
	pixel_pos = vec3(world * vec4(local_pos, 1));
	Normal = vec3(vec4(norm,1) * world);
	outUvw = uvw;
}