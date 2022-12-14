#version 330 // GLSL 3.30
out vec4 Pixel;
// an ultra simple glsl fragment shader

struct ATTRIBUTES {
	// kd - diffuse reflectivity // d - dissolve (transparency) 
	vec3 kd; float d;
	// Ks - specular reflectivity // Ns - specular exponent
	vec3 Ks;  float Ns;
	// Ka - ambient reflectivity // shaprness - local reflection map sharpness
	vec3 Ka; float sharpness; 
	// Tf - transmission filter // Ni - optical density (index of refraction)
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

// Input from our vertex shader
in vec3 Normal;
in vec3 pixel_pos;

// Convert Vec4 to vec3 
vec3 cam_pos = vec3(camPos);
vec3 light_amb = vec3(lightAmbient);
vec3 light_dir = vec3(lightDir);
vec3 light_col = vec3(lightCol);
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
	// Directional Light
	float diff_ratio = clamp(dot(-light_dir, norm), 0, 1); // Light Ratio
	vec3 total_direct = diff_ratio * light_col; // Multiply our light Ratio with Light color
	// Ambient Lighting
	vec3 total_indrect = light_amb * diffuse;
	// Refelcted Lighting (half vec) method
	vec3 view_dir = normalize(cam_pos - pixel_pos);
	vec3 half_vec = normalize((-light_dir) + view_dir);
	float intensity = pow(max(dot(norm, half_vec), 0), spec_exponent);
	vec3 specular = spec * light_col * intensity;

	// Final result of all three
	vec4 result =  vec4(clamp(total_direct + total_indrect, 0, 1) * diffuse + specular + emissive_reflect, dissolve);
	// Output to mesh
	Pixel = result; 
}