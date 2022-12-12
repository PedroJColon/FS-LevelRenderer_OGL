// minimalistic code to draw a single triangle, this is not part of the API.
// DONE: Part 1b
#include "FSLogo.h"
// Simple Vertex Shader
const char* vertexShaderSource = nullptr;
// Simple Fragment Shader
const char* fragmentShaderSource = nullptr;

// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
#ifndef NDEBUG
void MessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}
#endif
// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;
	// what we need at a minimum to draw a triangle
	GLuint vertexArray = 0;
	GLuint vertexBufferObject = 0;
	// DONE: Part 1g
	GLuint elementBufferObject = 0;
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
	GLuint shaderExecutable = 0;
	// DONE: Part 2c
	GLuint uniformBufferObject = 0;
	// DONE: Part 2a
	GW::MATH::GMatrix gMatrix;
	GW::MATH::GMATRIXF worldMatrix[2] = { GW::MATH::GIdentityMatrixF, GW::MATH::GIdentityMatrixF };
	GW::MATH::GMATRIXF viewMatrix = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF projMatrix = GW::MATH::GIdentityMatrixF;
	std::chrono::steady_clock::time_point startTime;

	float delta = 0;
	// DONE: Part 2b
	// DONE: Part 4e
	struct UBO_DATA
	{
		GW::MATH::GVECTORF sunDir, sunCol, sunAmbient, camPos;
		GW::MATH::GMATRIXF uViewMatrix, uProjMatrix;
		GW::MATH::GMATRIXF world;
		OBJ_ATTRIBUTES material;
	};
	UBO_DATA ubo;

	// Load a shader file as a string of characters.
	std::string ShaderAsString(const char* shaderFilePath) {
		std::string output;
		unsigned int stringLength = 0;
		GW::SYSTEM::GFile file; file.Create();
		file.GetFileSize(shaderFilePath, stringLength);
		if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
			output.resize(stringLength);
			file.Read(&output[0], stringLength);
		}
		else
			std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
		return output;
	}

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl)
	{
		win = _win;
		ogl = _ogl;
		// DONE: part 2a
		gMatrix.Create();

		// View Matrix
		GW::MATH::GVECTORF eye = { 0.75f,0.25f,1.5f,1.f };
		GW::MATH::GVECTORF at = { 0.15f,0.75f,0.0f,1.0f };
		GW::MATH::GVECTORF up = { 0.f,1.f,0.f,1.f };
		gMatrix.LookAtRHF(eye, at, up, viewMatrix);

		// Projection Matrix
		float verticalFOV = G_DEGREE_TO_RADIAN(65.f);
		float aspectRatio = 0.0f;
		ogl.GetAspectRatio(aspectRatio);
		float zNear = 0.1f;
		float zFar = 100.f;
		gMatrix.ProjectionOpenGLRHF(verticalFOV, aspectRatio, zNear, zFar, projMatrix);

		// Light Colors
		// DONE: Part 4e
		GW::MATH::GVECTORF lightColors = { 0.9f, 0.9f, 1.0f, 1.0f };
		// Light Direction
		GW::MATH::GVECTORF lightDirection = { -1,-1,-2 };
		GW::MATH::GVector::NormalizeF(lightDirection, lightDirection);
		GW::MATH::GVECTORF sunAmbRatio = { 25 / 255.f, 25 / 255.f, 35 / 255.f };

		// DONE: Part 2b
		ubo = { lightDirection, lightColors, sunAmbRatio, eye, viewMatrix, projMatrix, worldMatrix[0], FSLogo_materials[0].attrib };

		std::string pixelShader = ShaderAsString("../Pixelshader.glsl");
		std::string vertShader = ShaderAsString("../Vertexshader.glsl");

		fragmentShaderSource = pixelShader.c_str();
		vertexShaderSource = vertShader.c_str();
		// Link Needed OpenGL API functions
		LoadExtensions();
		// In debug mode we link openGL errors to the console
#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
#endif
		// DONE: Part 1c
		// Create Vertex Buffer

		// Base triangle test
		float verts[] = {
			   0,   0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f
		};

		// DONE: Part 1g
		glGenVertexArrays(1, &vertexArray);
		glGenBuffers(1, &vertexBufferObject);
		glGenBuffers(1, &elementBufferObject);
		glGenBuffers(1, &uniformBufferObject);
		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(OBJ_VERT) * FSLogo_vertexcount, &FSLogo_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * FSLogo_indexcount, &FSLogo_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_DATA), &ubo, GL_DYNAMIC_DRAW);
		// Vertices
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OBJ_VERT), (void*)0);
		glEnableVertexAttribArray(0);
		// UVW
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(OBJ_VERT), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(1);
		// Normals
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(OBJ_VERT), (void*)(sizeof(float) * 6));
		glEnableVertexAttribArray(2);
		// DONE: Part 2c
		// Create Vertex Shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		char errors[1024]; GLint result;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
		if (result == false)
		{
			glGetShaderInfoLog(vertexShader, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}
		// Create Fragment Shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
		if (result == false)
		{
			glGetShaderInfoLog(fragmentShader, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}
		// Create Executable Shader Program
		shaderExecutable = glCreateProgram();
		glAttachShader(shaderExecutable, vertexShader);
		glAttachShader(shaderExecutable, fragmentShader);
		glLinkProgram(shaderExecutable);
		glGetProgramiv(shaderExecutable, GL_LINK_STATUS, &result);
		if (result == false)
		{
			glGetProgramInfoLog(shaderExecutable, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}
	}
	void Render()
	{
		// DONE: Part 2a
		// setup the pipeline
		// DONE: Part 1e
		glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
		glUseProgram(shaderExecutable);
		// now we can draw
		glBindVertexArray(vertexArray);
		glUseProgram(shaderExecutable);
		// DONE: Part 1d
		// DONE: Part 1h
		// DONE: Part 2e
		GLuint index = glGetUniformBlockIndex(shaderExecutable, "UBO_DATA");
		// DONE: Part 2f
		glBindBufferBase(GL_UNIFORM_BUFFER, index, uniformBufferObject);
		// DONE: Part 2g
		glUniformBlockBinding(shaderExecutable, index, 0);
		// DONE: Part 3b
			// DONE: Part 4d
			// DONE: Part 3c
			// DONE: Part 4e
		// Renders Logo
		GLvoid* secPtr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		// Get time delta to rotate our logo
		std::chrono::steady_clock::time_point endTime(std::chrono::steady_clock::now());
		delta = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime).count();
		gMatrix.RotateYLocalF(worldMatrix[0], 1.f * delta, worldMatrix[0]);
		ubo.material = FSLogo_materials[1].attrib;
		ubo.world = worldMatrix[0];
		memcpy(secPtr, &ubo, sizeof(UBO_DATA));
		startTime = std::chrono::steady_clock::now();
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glDrawElements(GL_TRIANGLES, FSLogo_meshes[1].indexCount, GL_UNSIGNED_INT, (void*)(FSLogo_meshes[1].indexOffset * sizeof(unsigned)));

		// Renders Full Sail University
		GLvoid* firPtr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		ubo.material = FSLogo_materials[0].attrib;
		ubo.world = worldMatrix[1];
		memcpy(firPtr, &ubo, sizeof(UBO_DATA));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glDrawElements(GL_TRIANGLES, FSLogo_meshes[0].indexCount, GL_UNSIGNED_INT, (void*)(FSLogo_meshes[0].indexOffset * sizeof(unsigned)));


		// some video cards(cough Intel) need this set back to zero or they won't display
		glBindVertexArray(0);
	}
	~Renderer()
	{
		// free resources
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteBuffers(1, &vertexBufferObject);
		// DONE: Part 1g
		glDeleteBuffers(1, &elementBufferObject);
		glDeleteShader(vertexShader);

		glDeleteShader(fragmentShader);
		glDeleteProgram(shaderExecutable);
		// DONE: Part 2c
		glDeleteBuffers(1, &uniformBufferObject);
	}
private:
	// Modern OpenGL API Functions must be queried before use
	PFNGLCREATESHADERPROC				glCreateShader = nullptr;
	PFNGLSHADERSOURCEPROC				glShaderSource = nullptr;
	PFNGLCOMPILESHADERPROC				glCompileShader = nullptr;
	PFNGLGETSHADERIVPROC				glGetShaderiv = nullptr;
	PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog = nullptr;
	PFNGLATTACHSHADERPROC				glAttachShader = nullptr;
	PFNGLDETACHSHADERPROC				glDetachShader = nullptr;
	PFNGLDELETESHADERPROC				glDeleteShader = nullptr;
	PFNGLCREATEPROGRAMPROC				glCreateProgram = nullptr;
	PFNGLLINKPROGRAMPROC				glLinkProgram = nullptr;
	PFNGLUSEPROGRAMPROC					glUseProgram = nullptr;
	PFNGLGETPROGRAMIVPROC				glGetProgramiv = nullptr;
	PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog = nullptr;
	PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays = nullptr;
	PFNGLBINDVERTEXARRAYPROC			glBindVertexArray = nullptr;
	PFNGLGENBUFFERSPROC					glGenBuffers = nullptr;
	PFNGLBINDBUFFERPROC					glBindBuffer = nullptr;
	PFNGLBUFFERDATAPROC					glBufferData = nullptr;
	PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray = nullptr;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray = nullptr;
	PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer = nullptr;
	PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation = nullptr;
	PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv = nullptr;
	PFNGLDELETEBUFFERSPROC				glDeleteBuffers = nullptr;
	PFNGLDELETEPROGRAMPROC				glDeleteProgram = nullptr;
	PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays = nullptr;
	PFNGLDEBUGMESSAGECALLBACKPROC		glDebugMessageCallback = nullptr;
	// DONE: Part 2d
	PFNGLGETUNIFORMBLOCKINDEXPROC		glGetUniformBlockIndex = nullptr;
	PFNGLUNIFORMBLOCKBINDINGPROC		glUniformBlockBinding = nullptr;
	PFNGLBINDBUFFERBASEPROC				glBindBufferBase = nullptr;
	PFNGLMAPBUFFERPROC					glMapBuffer = nullptr;
	PFNGLUNMAPBUFFERPROC				glUnmapBuffer = nullptr;


	// Modern OpenGL API functions need to be queried
	void LoadExtensions()
	{
		ogl.QueryExtensionFunction(nullptr, "glCreateShader", (void**)&glCreateShader);
		ogl.QueryExtensionFunction(nullptr, "glShaderSource", (void**)&glShaderSource);
		ogl.QueryExtensionFunction(nullptr, "glCompileShader", (void**)&glCompileShader);
		ogl.QueryExtensionFunction(nullptr, "glGetShaderiv", (void**)&glGetShaderiv);
		ogl.QueryExtensionFunction(nullptr, "glGetShaderInfoLog", (void**)&glGetShaderInfoLog);
		ogl.QueryExtensionFunction(nullptr, "glAttachShader", (void**)&glAttachShader);
		ogl.QueryExtensionFunction(nullptr, "glDetachShader", (void**)&glDetachShader);
		ogl.QueryExtensionFunction(nullptr, "glDeleteShader", (void**)&glDeleteShader);
		ogl.QueryExtensionFunction(nullptr, "glCreateProgram", (void**)&glCreateProgram);
		ogl.QueryExtensionFunction(nullptr, "glLinkProgram", (void**)&glLinkProgram);
		ogl.QueryExtensionFunction(nullptr, "glUseProgram", (void**)&glUseProgram);
		ogl.QueryExtensionFunction(nullptr, "glGetProgramiv", (void**)&glGetProgramiv);
		ogl.QueryExtensionFunction(nullptr, "glGetProgramInfoLog", (void**)&glGetProgramInfoLog);
		ogl.QueryExtensionFunction(nullptr, "glGenVertexArrays", (void**)&glGenVertexArrays);
		ogl.QueryExtensionFunction(nullptr, "glBindVertexArray", (void**)&glBindVertexArray);
		ogl.QueryExtensionFunction(nullptr, "glGenBuffers", (void**)&glGenBuffers);
		ogl.QueryExtensionFunction(nullptr, "glBindBuffer", (void**)&glBindBuffer);
		ogl.QueryExtensionFunction(nullptr, "glBufferData", (void**)&glBufferData);
		ogl.QueryExtensionFunction(nullptr, "glEnableVertexAttribArray", (void**)&glEnableVertexAttribArray);
		ogl.QueryExtensionFunction(nullptr, "glDisableVertexAttribArray", (void**)&glDisableVertexAttribArray);
		ogl.QueryExtensionFunction(nullptr, "glVertexAttribPointer", (void**)&glVertexAttribPointer);
		ogl.QueryExtensionFunction(nullptr, "glGetUniformLocation", (void**)&glGetUniformLocation);
		ogl.QueryExtensionFunction(nullptr, "glUniformMatrix4fv", (void**)&glUniformMatrix4fv);
		ogl.QueryExtensionFunction(nullptr, "glDeleteBuffers", (void**)&glDeleteBuffers);
		ogl.QueryExtensionFunction(nullptr, "glDeleteProgram", (void**)&glDeleteProgram);
		ogl.QueryExtensionFunction(nullptr, "glDeleteVertexArrays", (void**)&glDeleteVertexArrays);
		ogl.QueryExtensionFunction(nullptr, "glDebugMessageCallback", (void**)&glDebugMessageCallback);
		// DONE: Part 2d
		ogl.QueryExtensionFunction(nullptr, "glGetUniformBlockIndex", (void**)&glGetUniformBlockIndex);
		ogl.QueryExtensionFunction(nullptr, "glUniformBlockBinding", (void**)&glUniformBlockBinding);
		ogl.QueryExtensionFunction(nullptr, "glBindBufferBase", (void**)&glBindBufferBase);
		ogl.QueryExtensionFunction(nullptr, "glMapBuffer", (void**)&glMapBuffer);
		ogl.QueryExtensionFunction(nullptr, "glUnmapBuffer", (void**)&glUnmapBuffer);
	}
};
