#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.hpp>
#include "CellularAutomaton.hpp"
#include "camera.hpp"

// Update the cellular automaton 20 times per second
const double updateLim = 1.0 / 20.0;

const int grid = 256;

Camera camera({-grid * 2, grid / 2, grid / 2}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);

  Shader* shader = static_cast<Shader*>(glfwGetWindowUserPointer(window));
  shader->use();
  shader->setVec2("resolution", (float)width, (float)height);

  camera.resolution = glm::vec2(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  static float lastX = 400, lastY = 300;
  static bool firstMouse = true;

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  camera.handleMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.handleMouseScroll(yoffset);
}

void initImGui(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

GLFWwindow* initGLFWGlad() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to init GLFW");
  }

  GLFWwindow* window =
      glfwCreateWindow(800, 600, "Game of Life - 0 FPS", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(window);
    glfwTerminate();
    throw std::runtime_error("Failed to init GLAD");
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  return window;
}

int main() {
  GLFWwindow* window = initGLFWGlad();
  initImGui(window);

  Shader shader("../src/vertex_shader.glsl", "../src/fragment_shader.glsl");
  shader.use();
  shader.setVec3("voxelSize", grid, grid, grid);
  glfwSetWindowUserPointer(window, &shader);

  float verts[] = {-1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0,
                   -1.0, 1.0, 0.0, 1.0,  -1.0, 0.0, 1.0, 1.0,  0.0};

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  CellularAutomaton ca({grid, grid, grid});

  float lastTime = glfwGetTime();
  float timer = lastTime;
  float limDT = 0;
  int frames = 0;

  while (!glfwWindowShouldClose(window)) {
    float nowTime = glfwGetTime();
    float deltaTime = nowTime - lastTime;
    limDT += deltaTime / updateLim;
    lastTime = nowTime;

    while (limDT >= 1.0) {
      ca.update();
      limDT--;
    }

    if (glfwGetTime() - timer > 1.0) {
      timer++;
      glfwSetWindowTitle(
          window,
          ("Game of Life - " + std::to_string(frames) + " FPS").c_str());
      std::cout << "FPS: " + std::to_string(frames) << std::endl;
      frames = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_R)) {
      ca.reset();
    }

    frames++;

    camera.handleKeyboardInput(window, deltaTime);

    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, ca.currentTexture());
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());
    shader.setInt("voxelData", 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::Begin("window");
    ImGui::Text("hello world");
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}