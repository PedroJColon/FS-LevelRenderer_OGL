	#version 330 // GLSL 3.30
	// an ultra simple glsl vertex shader
	// DONE: Part 2b
struct OBJ_ATTRIBUTES {
		vec3 kd; // diffuse reflectivity
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
	// TODO: Part 4e
	// DONE: Part 1f
	layout (location = 0) in vec3 local_pos;
	layout (location = 1) in vec3 uvw;
	layout (location = 2) in vec3 norm;

	// TODO: Part 4a
	out vec3 Normal;
	out vec3 outUvw;
	out vec3 pixel_pos;


	void main()
	{
		// DONE: Part 1f
		// DONE: Part 1h
		// DONE: Part 2h
		mat4 view_proj = uViewMatrix * uProjMatrix;
		view_proj = transpose(view_proj);
		gl_Position = view_proj * (vec4(local_pos, 1) * world);
		pixel_pos = vec3(world * vec4(local_pos, 1));
		Normal = vec3(vec4(norm,1) * world);
		outUvw = uvw;


		// TODO: Part 4b
	}