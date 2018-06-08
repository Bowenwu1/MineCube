#include "Global.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Gui.hpp"
#include "CraftManager.hpp"
#include "CubeManager.hpp"

#define SHADOW
//#define PLANE
#define DEBUG

// interactive variables
int screenWidth = 960;
int screenHeight = 960;

// About cubes
GLfloat sizePerCube = 0.1f;
unsigned int numPerEdge = 10;
const string mat4Name = "model";
vector<GLuint> attriSize;
GLfloat rotateSensivitiy = 30.0f;
GLfloat lookAroundSensivitiy = 1.0f;

// hover color
glm::vec3 hoverColor(1.0f, 0.0f, 0.0f);

// Camera class
static Camera* camera = Camera::getInstance();

bool isFpsMode = true;

// lighting
glm::vec3 lightPos(1.5f, 1.0f, 1.5f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

glm::vec3 objectColor(cubes_color[0], cubes_color[1], cubes_color[2]);
glm::vec3 specular(0.2f, 0.2f, 0.2f);  // test material
// parameters
float shininess = 32.0f;

// callback functions
void glfw_error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


#ifdef PLANE
GLuint planeVAO;
#endif

void RenderScene(Shader &phongShader, CubeManager & cubeManager);

// util functions
void PickOneCube(
	int xpos, int ypos,
	int screenWidth, int screenHeight,
	const glm::mat4& view,
	const glm::mat4& projection,
	unsigned int numPerEdge,
	float sizePerCube,
	CubeManager cubeManager,
	const glm::vec3& hoverColor
);


int main()
{
	glfwSetErrorCallback(glfw_error_callback);

	// initialize GLFW
	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    
    // creat GLFW window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "MineCube", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vertical synchronization
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// initialize GLAD and load pointer address of OpenGL functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    // Define the viewport dimensions
#ifdef SHADOW
    glViewport(0, 0, screenWidth, screenHeight);
#endif

	glEnable(GL_DEPTH_TEST);

	Shader phongShader("../src/Shader/phongvs.vs", "../src/Shader/phongfs.fs");

#ifdef SHADOW	
	Shader simpleDepthShader("../src/Shader/shadow_mapping_depth.vs", "../src/Shader/shadow_mapping_depth.fs");
    Shader debugDepthQuad("../src/Shader/debug_quad_depth.vs", "../src/Shader/debug_quad_depth.fs");
#endif

	Gui gui(window);

#ifdef PLANE
	// set floor
    GLfloat planeVertices[] = {
        // Positions          // Normals       
        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 
        -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f,

        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f,
        25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f
    };
    // Setup plane VAO
    GLuint planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
#endif

#ifdef SHADOW
	// Configure depth map FBO
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // - Create depth texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 防止纹理贴图在远处重复渲染
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // phongShader configuration
    // --------------------
    phongShader.use();
    phongShader.setInt("diffuseTexture", 0);
    phongShader.setInt("shadowMap", 1);
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);
#endif



	// to normalize
	attriSize.push_back(3);
	CraftManager craftManger(sizePerCube, numPerEdge, phongShader.ID, mat4Name, attriSize);

    CubeManager cubeManager(numPerEdge, numPerEdge, numPerEdge, sizePerCube);

	cubeManager.defalut_init_all(phongShader.ID, mat4Name);

	// main loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		gui.createNewFrame();
		gui.draw();

		glm::vec3 objectColor = glm::vec3(cubes_color[0], cubes_color[1], cubes_color[2]);
		glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef SHADOW
		// -------------------------------------------
        // 1. Render depth of scene to texture (from light's perspective)
        // - Get light projection/view matrix.
        glCullFace(GL_FRONT);
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        GLfloat near_plane = 1.0f, far_plane = 7.5f;

        // Orthographic 
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // - render scene from light's point of view
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        RenderScene(simpleDepthShader, cubeManager);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK); // 不要忘记设回原先的culling face

        // -----------------------------------------
        // reset viewport
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#endif

		glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		phongShader.use();
		phongShader.setVec3("viewPos", camera->getCameraPosition());

        // material
        phongShader.setVec3("material.specular", specular);
        phongShader.setFloat("material.shininess", shininess);
        // light
        phongShader.setVec3("light.position",  lightPos);
        phongShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        phongShader.setVec3("light.diffuse",  1.0f, 1.0f, 1.0f);
        phongShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		glm::mat4 view = camera->getViewMatrix();
		phongShader.setMat4("view", view);
		glm::mat4 projection = glm::perspective(glm::radians(camera->getZoomFactor()), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		phongShader.setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderScene(phongShader, cubeManager);

#ifdef SHADOW
		// render Depth map to quad for visual debugging
        // ---------------------------------------------
        debugDepthQuad.use();
        debugDepthQuad.setFloat("near_plane", near_plane);
        debugDepthQuad.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        // RenderQuad();
#endif
		/*
		-----------------------------------------------------------------------------------
			set/reset cubes
		-----------------------------------------------------------------------------------
		*/
		for (int index = 0; index < glm::pow(numPerEdge, 3); index++) {
			int t_index = index;
			int x = t_index / (int)glm::pow(numPerEdge, 2);
			t_index -= x * (int)glm::pow(numPerEdge, 2);
			int y = t_index / glm::pow(numPerEdge, 1);
			t_index -= y * (int)glm::pow(numPerEdge, 1);
			int z = t_index;

			auto cube = cubeManager.getCube(x, y, z);
			for (int plane = 0; plane < 6; plane++)
				cube->editColor(objectColor.x, objectColor.y, objectColor.z, plane);
		}

		/*
		-----------------------------------------------------------------------------------
			OBB-ray hitting test
		-----------------------------------------------------------------------------------
		*/
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		PickOneCube(
			(int)xpos, (int)ypos, (int)screenWidth, (int)screenHeight,
			view, projection, 
			numPerEdge, sizePerCube, 
			cubeManager, 
			hoverColor
		);

		/*
		-----------------------------------------------------------------------------------
		    move cubes
		-----------------------------------------------------------------------------------
		*/
		if (camera->isRotateX()) {
			cubeManager.setRotateSensivity(rotateSensivitiy);
			cubeManager.rotateHorizontal(camera->getRotateX());
			camera->resetRotateX();
		}

		if (camera->isRotateY()) {
			cubeManager.setRotateSensivity(rotateSensivitiy);
			cubeManager.rotateHorizontal(camera->getRotateY());
			camera->resetRotateY();
		}

		if (camera->isMoving) {
			cubeManager.setRotateSensivity(lookAroundSensivitiy);
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			glm::vec2 offset = camera->updateXYoffset((float)x, (float)y);
			cubeManager.rotateHorizontal(offset.x);
			cubeManager.rotateVertical(offset.y);
		}

		cubeManager.setAllShaderId(phongShader.ID);
		// RenderScene(phongShader, cubeManager);
		// cubeManager.draw();
		gui.render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	gui.clear();


	glfwTerminate();
	return 0;
}

void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Error %d: %s\n", error, description);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void RenderScene(Shader &shader, CubeManager & cubeManager)
{
#ifdef PLANE
    // Floor
    glm::mat4 model;
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
#endif // PLANE
    // Cubes
    cubeManager.draw();
    // model = glm::mat4();
    // //model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    // phongShader.setMat4("model", glm::value_ptr(model));
    // phongShader.setFloat3("objectColor", glm::value_ptr(glm::vec3(1.0f, 0.5f, 0.31f)));
    // glBindVertexArray(cubeVAO);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    // glBindVertexArray(0);
}