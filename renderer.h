// minimalistic code to draw a single triangle, this is not part of the API.

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
	GW::MATH::GMatrix gMatrix;
	H2B::Parser readModelData;

	// Matrices
	GW::MATH::GMATRIXF projMatrix = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF viewMatrix = GW::MATH::GIdentityMatrixF;

	// Vectors
	std::vector<MD::Model> allModels;
	std::vector<std::string> omitWords = {"_Plane", "_Cube", "_Cylinder"};

	// Timer
	std::chrono::steady_clock::time_point startTime;

	float delta = 0;


public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl, std::string levelFile)
	{
		win = _win;
		ogl = _ogl;

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
		GW::MATH::GVECTORF lightColors = { 0.9f, 0.9f, 1.0f, 1.0f };
		// Light Direction
		GW::MATH::GVECTORF lightDirection = { -1,-1,-2 };
		GW::MATH::GVector::NormalizeF(lightDirection, lightDirection);
		GW::MATH::GVECTORF sunAmbRatio = { 25 / 255.f, 25 / 255.f, 35 / 255.f };

		// Link Needed OpenGL API functions
		HF::LoadExtensions(&ogl);
		// In debug mode we link openGL errors to the console
#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT);
		HF::glDebugMessageCallback(MessageCallback, 0);
#endif
		LoadLevel(levelFile);

	}

	void Render()
	{
		for (int i = 0; i < allModels.size(); i++)
			allModels[i].DrawModel();
	}

	~Renderer()
	{
		for (MD::Model models : allModels)
			models.Free();
	}

private:
	void LoadLevel(std::string levelFile)
	{
		std::string fragShader = HF::ShaderAsString("../Pixelshader.glsl");
		std::string vertShader = HF::ShaderAsString("../Vertexshader.glsl");

		std::string getString = "";
		std::ifstream file;
		file.open(levelFile);
		if (file.is_open())
		{
			while (!file.eof())
			{
				getline(file, getString);
				if (getString == "LIGHT")
				{

				}

				if (getString == "CAMERA")
				{

				}

				if (getString == "MESH")
				{
					std::string objectName;
					getline(file, objectName);
					for (int i = 0; i < omitWords.size(); i++)
					{
						objectName = objectName.substr(0, objectName.rfind(omitWords[i]));
					}
					//std::string objectPath = "../" + objectName + ".h2b";
					std::string objectPath = "../Column_1.h2b";
					GW::MATH::GVECTORF eye = { 0.75f,0.25f,1.5f,1.f };
					H2B::Parser parser;
					if (parser.Parse(objectPath.c_str()))
					{
						MD::Model tempModel(parser, ogl, eye, viewMatrix, projMatrix, file, vertShader, fragShader);
						allModels.push_back(tempModel);
					}
				}
				std::cout << getString << "\n";
			}
		}
	}
};
