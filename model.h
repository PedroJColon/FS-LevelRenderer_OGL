namespace MD
{
	struct ATTRIBUTES
	{
		// kd - diffuse reflectivity // d - dissolve (transparency) 
		H2B::VECTOR Kd; float d;
		// Ks - specular reflectivity // Ns - specular exponent
		H2B::VECTOR Ks; float Ns;
		// Ka - ambient reflectivity // shaprness - local reflection map sharpness
		H2B::VECTOR Ka; float sharpness;
		// Tf - transmission filter // Ni - optical density (index of refraction)
		H2B::VECTOR Tf; float Ni;
		// Ke - emissive reflectivity // illum - illumination model
		H2B::VECTOR Ke; unsigned illum;
	};


	// struct attrivutes
	struct alignas(void*) UBO_DATA
	{
		GW::MATH::GVECTORF lightDir, lightCol, lightAmbient, camPos;
		GW::MATH::GMATRIXF uViewMatrix, uProjMatrix;
		GW::MATH::GMATRIXF world;
		ATTRIBUTES material;
	};


	class Model
	{
		// what we need at a minimum to draw a triangle
		GLuint VAO = 0; // Vertex Array Object
		GLuint VBO = 0; // Vertex Buffer Object
		GLuint EBO = 0; // Element Buffer Object
		GLuint UBO = 0; // Uniform Buffer Object
		GLuint fragmentShader = 0;
		GLuint vertexShader = 0;
		GLuint shaderExecutable = 0;

		GLchar* fragmentShaderSource = nullptr;
		GLchar* vertexShaderSource = nullptr;

		GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;

		UBO_DATA uboData;
		H2B::Parser readModelData;
		GW::GRAPHICS::GOpenGLSurface ogl;

	public:

		Model(H2B::Parser _readModelData, GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GVECTORF _lightDir, GW::MATH::GVECTORF _lightCol, GW::MATH::GVECTORF _lightAmb, GW::MATH::GVECTORF _camPos, GW::MATH::GMATRIXF _viewMatrix, GW::MATH::GMATRIXF _projMatrix, std::ifstream& file, std::string vertexShader, std::string fragShader)
		{
			ogl = _ogl;
			readModelData = _readModelData;
			vertexShaderSource = (GLchar*) vertexShader.c_str();
			fragmentShaderSource = (GLchar*) fragShader.c_str();

			// Read matrix data (be sure to get line and read it using scanf
			std::vector<float> worldValues;


			// loop through file to get our matrix values
			for (int i = 0; i <= 3; i++)
			{
				std::string getString = "";
				getline(file, getString, '(');
				getline(file, getString);
				std::string matrixValues = getString.substr(getString.size() - getString.size(), getString.rfind(')'));
				std::string valueStr = matrixValues.substr(matrixValues.size() - matrixValues.size(), matrixValues.find_first_of(','));
				float xValue = std::stof(valueStr);
				worldValues.push_back(xValue);

				valueStr = matrixValues.substr(matrixValues.size() - 24, matrixValues.find_first_of(','));
				valueStr = valueStr.substr(valueStr.size() - valueStr.size(), valueStr.rfind(','));
				float yValue = std::stof(valueStr);
				worldValues.push_back(yValue);

				valueStr = matrixValues.substr(matrixValues.size() - 15, matrixValues.find_first_of(','));
				valueStr = valueStr.substr(valueStr.size() - valueStr.size(), valueStr.rfind(','));
				float zValue = std::stof(valueStr);
				worldValues.push_back(zValue);

				valueStr = matrixValues.substr(matrixValues.size() - 7, matrixValues.find_first_of(')'));
				valueStr = valueStr.substr(valueStr.size() - valueStr.size(), valueStr.rfind(')'));
				float wValue = std::stof(valueStr);
				worldValues.push_back(wValue);
			}

			// Save our world values into world matrices
			world.row1.x = worldValues[0];
			world.row1.y = worldValues[1];
			world.row1.z = worldValues[2];
			world.row1.w = worldValues[3];

			world.row2.x = worldValues[4];
			world.row2.y = worldValues[5];
			world.row2.z = worldValues[6];
			world.row2.w = worldValues[7];

			world.row3.x = worldValues[8];
			world.row3.y = worldValues[9];
			world.row3.z = worldValues[10];
			world.row3.w = worldValues[11];

			world.row4.x = worldValues[12];
			world.row4.y = worldValues[13];
			world.row4.z = worldValues[14];
			world.row4.w = worldValues[15];

			uboData = {_lightDir, _lightCol, _lightAmb, _camPos, _viewMatrix, _projMatrix, world, (ATTRIBUTES&)readModelData.materials[0].attrib};

			LoadModel(); // Load the drawing information from model
			LoadShaders(); // Load Shaders for model
		}

		void DrawModel(GW::MATH::GMATRIXF newView, GW::MATH::GMATRIXF newProj)
		{
			// Set up
			HF::glBindBuffer(GL_UNIFORM_BUFFER, UBO);
			HF::glUseProgram(shaderExecutable);

			// now we can draw
			HF::glBindVertexArray(VAO);
			HF::glUseProgram(shaderExecutable);
			GLuint index = HF::glGetUniformBlockIndex(shaderExecutable, "UBO_DATA");
			HF::glBindBufferBase(GL_UNIFORM_BUFFER, index, UBO);
			HF::glUniformBlockBinding(shaderExecutable, index, 0);

			for (int i = 0; i < readModelData.meshes.size(); i++)
			{
				if (readModelData.meshes[i].drawInfo.indexCount == 0) // If a mmesh doesn't have an index count, we switch to using batch
				{
					for (int i = 0; i < readModelData.batches.size(); i++)
					{
						if (readModelData.batches[i].indexCount == readModelData.indexCount)
						{
							GLvoid* ptr = HF::glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
							uboData.material = (ATTRIBUTES&)readModelData.materials[i].attrib;
							uboData.uViewMatrix = newView;
							uboData.uProjMatrix = newProj;
							memcpy(ptr, &uboData, sizeof(UBO_DATA));
							HF::glUnmapBuffer(GL_UNIFORM_BUFFER);
							glDrawElements(GL_TRIANGLES, readModelData.batches[i].indexCount, GL_UNSIGNED_INT, (void*)(readModelData.batches[i].indexOffset * sizeof(unsigned)));
						}
					}
				}
				else
				{
					GLvoid* ptr = HF::glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
					uboData.material = (ATTRIBUTES&)readModelData.materials[i].attrib;
					uboData.uViewMatrix = newView;
					uboData.uProjMatrix = newProj;
					memcpy(ptr, &uboData, sizeof(UBO_DATA));
					HF::glUnmapBuffer(GL_UNIFORM_BUFFER);
					glDrawElements(GL_TRIANGLES, readModelData.meshes[i].drawInfo.indexCount, GL_UNSIGNED_INT, (void*)(readModelData.meshes[i].drawInfo.indexOffset * sizeof(unsigned)));
				}

			}

			// some video cards(cough Intel) need this set back to zero or they won't display
			HF::glBindVertexArray(0);
		}

		void Free()
		{
			// free resources
			HF::glDeleteVertexArrays(1, &VAO);
			HF::glDeleteBuffers(1, &VBO);
			// DONE: Part 1g
			HF::glDeleteBuffers(1, &EBO);
			HF::glDeleteShader(vertexShader);

			HF::glDeleteShader(fragmentShader);
			HF::glDeleteProgram(shaderExecutable);
			// DONE: Part 2c
			HF::glDeleteBuffers(1, &UBO);
		}

		~Model()
		{

		}

	private:
		void LoadModel() 
		{
			// Create Vertex Buffer

			// Generate buffers
			HF::glGenVertexArrays(1, &VAO);
			HF::glGenBuffers(1, &VBO);
			HF::glGenBuffers(1, &EBO);
			HF::glGenBuffers(1, &UBO);
			
			// Bind buffers
			HF::glBindVertexArray(VAO);
			HF::glBindBuffer(GL_ARRAY_BUFFER, VBO);
			HF::glBufferData(GL_ARRAY_BUFFER, sizeof(H2B::VERTEX) * readModelData.vertexCount, readModelData.vertices.data(),GL_STATIC_DRAW);

			HF::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			HF::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * readModelData.indexCount, readModelData.indices.data(),GL_STATIC_DRAW);
			
			HF::glBindBuffer(GL_UNIFORM_BUFFER, UBO);
			HF::glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_DATA), &uboData, GL_DYNAMIC_DRAW);
			
			// Vertices
			HF::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (void*)0);
			HF::glEnableVertexAttribArray(0);
			// UVW
			HF::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (void*)(sizeof(float) * 3));
			HF::glEnableVertexAttribArray(1);
			// Normals
			HF::glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(H2B::VERTEX), (void*)(sizeof(float) * 6));
			HF::glEnableVertexAttribArray(2);
		}

		void LoadShaders()
		{
			// Create Vertex Shader
			vertexShader = HF::glCreateShader(GL_VERTEX_SHADER);
			HF::glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			HF::glCompileShader(vertexShader);
			char errors[1024]; GLint result;
			HF::glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
			if (result == false)
			{
				HF::glGetShaderInfoLog(vertexShader, 1024, NULL, errors);
				std::cout << errors << std::endl;
			}
			// Create Fragment Shader
			fragmentShader = HF::glCreateShader(GL_FRAGMENT_SHADER);
			HF::glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
			HF::glCompileShader(fragmentShader);
			HF::glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
			if (result == false)
			{
				HF::glGetShaderInfoLog(fragmentShader, 1024, NULL, errors);
				std::cout << errors << std::endl;
			}
			// Create Executable Shader Program
			shaderExecutable = HF::glCreateProgram();
			HF::glAttachShader(shaderExecutable, vertexShader);
			HF::glAttachShader(shaderExecutable, fragmentShader);
			HF::glLinkProgram(shaderExecutable);
			HF::glGetProgramiv(shaderExecutable, GL_LINK_STATUS, &result);
			if (result == false)
			{
				HF::glGetProgramInfoLog(shaderExecutable, 1024, NULL, errors);
				std::cout << errors << std::endl;
			}
		}
	};
}
