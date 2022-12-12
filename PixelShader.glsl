#version 330 // GLSL 3.30
out vec4 Pixel;
// an ultra simple glsl fragment shader
// DONE: Part 3a

struct OBJ_ATTRIBUTES {
	vec3 kd;// diffuse reflectivity
	float d; // dissolve (transparency) 
	vec3 Ks; // specular reflectivity
	float Ns; // specular exponent
	vec3 Ka; // ambient reflectivity
	float sharpness; // local reflection map sharpness
	vec3 Tf; // transmission filter
	float Ni; // optical density (index of refraction)
	vec3 Ke; // emissive reflectivity
	uint illum; // illumination model
};

layout(row_major) uniform UBO_DATA
{
	vec4 sunDir, sunCol, sunAmbient, camPos;
	mat4 uViewMatrix, uProjMatrix;
	mat4 world;
	OBJ_ATTRIBUTES material;
};
// DONE: Part 4e
// DONE: Part 4b

// Input from our vertex shader
in vec3 Normal;
in vec3 pixel_pos;

// Convert Vec4 to vec3 
vec3 cam_pos = vec3(camPos);
vec3 sun_amb = vec3(sunAmbient);
vec3 light_dir = vec3(sunDir);
vec3 light_col = vec3(sunCol);
vec3 diffuse = vec3(material.kd);
vec3 spec = vec3(material.Ks);
// normalize our input normal before using it
vec3 norm = normalize(Normal);

// Setting Material values
float spec_exponent = material.Ns;
float dissolve = material.d;
vec3 emissive_reflect = material.Ke;


void main() 
{	
	// DONE: Part 3a
	// Directional Light
	float diff_ratio = clamp(dot(-light_dir, norm), 0, 1); // Light Ratio
	vec3 total_direct = diff_ratio * light_col; // Multiply our light Ratio with Light color
	// Ambient Lighting
	vec3 total_indrect = sun_amb * diffuse;
	// Refelcted Lighting (half vec) method
	vec3 view_dir = normalize(cam_pos - pixel_pos);
	vec3 half_vec = normalize((-light_dir) + view_dir);
	float intensity = pow(max(dot(norm, half_vec), 0), spec_exponent);
	vec3 specular = spec * light_col * intensity;
	// Reflected Lighting (reflect) method (needs to be fixed)
//	vec3 view_dir_R = normalize(cam_pos - pixel_pos);
//	vec3 reflect_dir = reflect(-light_dir, norm);
//	float spec_intensity_r = pow(max(dot(view_dir_R, reflect_dir), 0), material.Ns);
//	vec3 specular_r = spec * light_col * spec_intensity_r;

	// Final result of all three
	vec4 result =  vec4(clamp(total_direct + total_indrect, 0, 1) * diffuse + specular + emissive_reflect, dissolve);
	// Output to mesh
	Pixel = result; // DONE: Part 1a
	// DONE: Part 4c
	// DONE: Part 4e
	// DONE: Part 4f (half-vector or reflect method)
}