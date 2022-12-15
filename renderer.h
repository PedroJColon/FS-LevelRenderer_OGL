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
	GW::INPUT::GInput inputProxy;
	GW::INPUT::GController controllerProxy;
	H2B::Parser readModelData;

	// Matrices
	GW::MATH::GMATRIXF projMatrix = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF viewMatrix = GW::MATH::GIdentityMatrixF;

	// Vectors
	std::vector<MD::Model> allModels;
	std::vector<std::string> omitWords = {"_Plane", "_Cube", "_Cylinder"};

	// Timer
	std::chrono::steady_clock::time_point startTime;
	float delta;


public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl, std::string levelFile)
	{
		win = _win;
		ogl = _ogl;

		gMatrix.Create();
		inputProxy.Create(win);
		controllerProxy.Create();

		// View Matrix
		GW::MATH::GVECTORF eye = { 0.0f,0.f,0.f,1.f };
		GW::MATH::GVECTORF at = { 0.0f,0.0f,0.0f,1.0f };
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
			allModels[i].DrawModel(viewMatrix, projMatrix);
	}

	~Renderer()
	{
		for (MD::Model models : allModels)
			models.Free();
	}

	void UpdateCamera()
	{
		// Stop Time
		std::chrono::steady_clock::time_point endTime(std::chrono::steady_clock::now());
		// Get the time duration to serve as our delta time
		delta = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime).count();

		// DONE: Part 4c
		// Create our camera matrix (view copy)
		GW::MATH::GMATRIXF Camera = { GW::MATH::GIdentityMatrixF };
		// Inverse it to be able to change values
		gMatrix.InverseF(viewMatrix, Camera);
		// DONE: Part 4d
		// const variables that do not need to be changed
		const float CAMERA_SPEED = 1.5f;
		const float CAMERA_ROT_SPEED = 0.35f;
		const float THUMB_ROT_SPEED = 0.25f;
		const float FRAME_SPEED = CAMERA_SPEED * delta;
		// Get our screenHeight and Width to allow us to rotate our camera effecitvly
		unsigned int screenHeight = 0;
		unsigned int screenWidth = 0;
		win.GetClientHeight(screenHeight);
		win.GetClientWidth(screenWidth);

		// Variables to store our mouses float delta data
		float mouseX = 0.0f;
		float mouseY = 0.0f;
		// Variables to store our key states (KBM)
		float kbm_foreward = 0.f;
		float kbm_backward = 0.f;
		float kbm_left = 0.f;
		float kbm_right = 0.f;
		float kbm_up = 0.f;
		float kbm_down = 0.f;
		// Variables to store our key states (Gamepad)
		float gp_lHorizontal = 0.f;
		float gp_rHorizontal = 0.f;
		float gp_lVertical = 0.f;
		float gp_rVertical = 0.f;
		float gp_up = 0.f;
		float gp_down = 0.f;

		// Get our key states for each directions
		// KBM Keys
		inputProxy.GetState(G_KEY_SPACE, kbm_up);
		inputProxy.GetState(G_KEY_LEFTSHIFT, kbm_down);
		inputProxy.GetState(G_KEY_W, kbm_foreward);
		inputProxy.GetState(G_KEY_S, kbm_backward);
		inputProxy.GetState(G_KEY_A, kbm_left);
		inputProxy.GetState(G_KEY_D, kbm_right);

		// Gamepad Keys
		inputProxy.GetState(G_RIGHT_TRIGGER_AXIS, gp_up);
		inputProxy.GetState(G_LEFT_TRIGGER_AXIS, gp_down);
		inputProxy.GetState(G_LY_AXIS, gp_lVertical);
		inputProxy.GetState(G_LX_AXIS, gp_lHorizontal);
		inputProxy.GetState(G_RX_AXIS, gp_rHorizontal);
		inputProxy.GetState(G_RY_AXIS, gp_rVertical);

		// Get our moust delta info to change our rotation
		inputProxy.GetMouseDelta(mouseX, mouseY);

		// Get our input strength for our X Y and Z values
		float xValue = kbm_right - kbm_left + gp_lHorizontal;
		float yValue = kbm_up - kbm_down + gp_up - gp_down;
		float zValue = kbm_backward - kbm_foreward + gp_lVertical; // Reason for being backwards is due to how OPENGL handles Z axis

		// Get our values for rotation for camera control
		float rotateX = (65.f * (-mouseX * CAMERA_ROT_SPEED * FRAME_SPEED) / screenWidth) + (gp_rHorizontal * THUMB_ROT_SPEED * FRAME_SPEED);
		float rotateY = (65.f * (-mouseY * CAMERA_ROT_SPEED * FRAME_SPEED) / screenHeight) + (gp_rVertical * THUMB_ROT_SPEED * FRAME_SPEED);
		float rotateZ = 0; // Set this zero cause we have no need for this

		// DONE: Part 4e
		// DONE: Part 4f 
		// DONE: Part 4g
		// Translation vector to effect our camera position
		GW::MATH::GVECTORF cameraPos = { (xValue * FRAME_SPEED),(yValue * FRAME_SPEED),(zValue * FRAME_SPEED) };
		// Rotation Matrix to effect our camera rotation
		GW::MATH::GMATRIXF rotationMat = GW::MATH::GIdentityMatrixF;
		// Math done to change our rotation and translation for Camera
		gMatrix.RotationYawPitchRollF(rotateX, rotateY, rotateZ, rotationMat);
		gMatrix.TranslateGlobalF(Camera, cameraPos, Camera);
		// Combine the two by multiplying the two
		gMatrix.MultiplyMatrixF(Camera, rotationMat, Camera);
		// Inverse our matrix to follow our view
		gMatrix.InverseF(Camera, Camera);

		// DONE: Part 4c
		// Copy our new matrix to the view matrix once all is done to update it
		viewMatrix = Camera;

		// Reset time
		startTime = std::chrono::steady_clock::now();
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
					std::string objectPath = "../" + objectName + ".h2b";
					H2B::Parser parser;
					if (parser.Parse(objectPath.c_str()))
					{
						MD::Model tempModel(parser, ogl, viewMatrix.row4, viewMatrix, projMatrix, file, vertShader, fragShader);
						allModels.push_back(tempModel);
					}
				}
				std::cout << getString << "\n";
			}
		}
	}
};
