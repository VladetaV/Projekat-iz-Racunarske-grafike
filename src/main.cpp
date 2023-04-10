#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadSkybox(vector<std::string> faces);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 objPosition = glm::vec3(10.0f);
    float objScale = 5.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // DEPTH TESTING
    glEnable(GL_DEPTH_TEST);

    // FACE CULLING
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    // -------------------------
    Shader earthShader("resources/shaders/earth.vs", "resources/shaders/earth.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader shader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader moonShader("resources/shaders/moon.vs", "resources/shaders/moon.fs");

    // temena za skybox
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/SkyBlue2_right1.png"),
                    FileSystem::getPath("resources/textures/skybox/SkyBlue2_left2.png"),
                    FileSystem::getPath("resources/textures/skybox/SkyBlue2_bottom4.png"),
                    FileSystem::getPath("resources/textures/skybox/SkyBlue2_top3.png"),
                    FileSystem::getPath("resources/textures/skybox/SkyBlue2_front5.png"),
                    FileSystem::getPath("resources/textures/skybox/SkyBlue2_back6.png")
            };
    unsigned int cubemapTexture = loadSkybox(faces);
    unsigned int sunTex = loadTexture("resources/objects/sun/13913_Sun_diff.jpg");

    skyboxShader.setInt("skybox", 0);
    skyboxShader.use();


    earthShader.setInt("earth", 1);
    earthShader.use();

    moonShader.setInt("moon", 2);
    moonShader.use();

    shader.setInt("tex", 3);
    shader.use();
    // load models
    // -----------

    Model sunModel("resources/objects/sun/Earth_2K.obj");
    Model moonModel("resources/objects/moon/moon.obj");
    Model earthModel("resources/objects/Earth/Earth_2K.obj");
    moonModel.SetShaderTextureNamePrefix("material.");
    sunModel.SetShaderTextureNamePrefix("material.");
    earthModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(-7.0f,14.5f,35.0f);
    pointLight.ambient = glm::vec3(1.2, 1.2, 1.2);
    pointLight.diffuse = glm::vec3(50.0, 50.0, 50.0);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;



    // draw in wireframe

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --------------- earthShader----------------------------------------------------

        earthShader.use();
        earthShader.setVec3("pointLight.position", pointLight.position);
        earthShader.setVec3("pointLight.ambient", pointLight.ambient);
        earthShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        earthShader.setVec3("pointLight.specular", pointLight.specular);
        earthShader.setFloat("pointLight.constant", pointLight.constant);
        earthShader.setFloat("pointLight.linear", pointLight.linear);
        earthShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        earthShader.setVec3("viewPosition", programState->camera.Position);
        earthShader.setFloat("material.shininess", 32.0f);

        earthShader.setVec3("dirLight1.direction", -0.2f, -1.0f, -0.3f);
        earthShader.setVec3("dirLight1.ambient", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight1.diffuse", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight1.specular", 0.005f, 0.005f, 0.005f);

        earthShader.setVec3("dirLight2.direction", 0.2f, -1.0f, -0.3f);
        earthShader.setVec3("dirLight2.ambient", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight2.diffuse", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight2.specular", 0.005f, 0.005f, 0.005f);

        earthShader.setVec3("dirLight3.direction", -0.2f, 1.0f, -0.3f);
        earthShader.setVec3("dirLight3.ambient", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight3.diffuse", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight3.specular", 0.005f, 0.005f, 0.005f);

        earthShader.setVec3("dirLight4.direction", 0.2f, 1.0f, -0.3f);
        earthShader.setVec3("dirLight4.ambient", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight4.diffuse", 0.005f, 0.005f, 0.005f);
        earthShader.setVec3("dirLight4.specular", 0.005f, 0.005f, 0.005f);

        // ---------------------shader---------------------------------------------------------
        shader.use();
        /*shader.setVec3("pointLight.position", pointLight.position);
        shader.setVec3("pointLight.ambient", pointLight.ambient);
        shader.setVec3("pointLight.diffuse", pointLight.diffuse);
        shader.setVec3("pointLight.specular", pointLight.specular);
        shader.setFloat("pointLight.constant", pointLight.constant);
        shader.setFloat("pointLight.linear", pointLight.linear);
        shader.setFloat("pointLight.quadratic", pointLight.quadratic);
        shader.setVec3("viewPosition", programState->camera.Position);
        shader.setFloat("material.shininess", 32.0f);
*/

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunTex);

        // -----------------moonShader --------------------------------------------
        moonShader.use();
        moonShader.setVec3("pointLight.position", pointLight.position);
        moonShader.setVec3("pointLight.ambient", pointLight.ambient);
        moonShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        moonShader.setVec3("pointLight.specular", pointLight.specular);
        moonShader.setFloat("pointLight.constant", pointLight.constant);
        moonShader.setFloat("pointLight.linear", pointLight.linear);
        moonShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        moonShader.setVec3("viewPosition", programState->camera.Position);
        moonShader.setFloat("material.shininess", 32.0f);

        moonShader.setVec3("dirLight1.direction", -0.2f, -1.0f, -0.3f);
        moonShader.setVec3("dirLight1.ambient", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight1.diffuse", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight1.specular", 0.005f, 0.005f, 0.005f);

        moonShader.setVec3("dirLight2.direction", 0.2f, -1.0f, -0.3f);
        moonShader.setVec3("dirLight2.ambient", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight2.diffuse", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight2.specular", 0.005f, 0.005f, 0.005f);

        moonShader.setVec3("dirLight3.direction", -0.2f, 1.0f, -0.3f);
        moonShader.setVec3("dirLight3.ambient", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight3.diffuse", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight3.specular", 0.005f, 0.005f, 0.005f);

        moonShader.setVec3("dirLight4.direction", 0.2f, 1.0f, -0.3f);
        moonShader.setVec3("dirLight4.ambient", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight4.diffuse", 0.005f, 0.005f, 0.005f);
        moonShader.setVec3("dirLight4.specular", 0.005f, 0.005f, 0.005f);

        // view/projection transformations

        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        
        earthShader.use();
        earthShader.setMat4("projection", projection);
        earthShader.setMat4("view", view);

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        moonShader.use();
        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);

        // render the loaded model


        earthShader.use();

        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3,glm::vec3(0.5f ,15.5f,3.0f));
        model3 = glm::scale(model3,glm::vec3(1.0f));
        model3 = glm::rotate(model3,glm::radians(180.0f),glm::vec3(1.0f,0.0f,0.0f));
        model3 = glm::rotate(model3,(float)glfwGetTime()/2,glm::vec3(0.0f,1.0f,0.0f));
        earthShader.setMat4("model3", model3);
        earthModel.Draw(earthShader);

        moonShader.use();
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2,glm::vec3(-2.0f * cos(currentFrame) * 4,14.5f ,-2.0f * sin(currentFrame) * 4));
        model2 = glm::scale(model2,glm::vec3(1.0f));
        moonShader.setMat4("model2", model2);
        moonModel.Draw(moonShader);

        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-28.0f,8.5f,75.0f));
        model = glm::scale(model, glm::vec3(5.0f));
        model = glm::rotate(model,(float)glfwGetTime()/4, glm::vec3(0.0f,1.0f,0.0f));
        shader.setMat4("model", model);
        sunModel.Draw(shader);


        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->objPosition);
        ImGui::DragFloat("Backpack scale", &programState->objScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

unsigned int loadSkybox(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    return textureID;
}


unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,format == GL_RGBA ? GL_CLAMP_TO_EDGE :  GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE :  GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}