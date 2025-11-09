#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.hpp>
#include "CellularAutomaton.hpp"
#include "camera.hpp"

#include <functional>

// Update the cellular automaton 20 times per second
const double updateLim = 1.0 / 20.0;

glm::ivec3 grid(256, 256, 64);
glm::ivec3 pgrid = grid;

Camera camera({-grid.x * 1.5, grid.y * 1.5, grid.z * 1.5}, {0.0, 1.0, 0.0}, 0.0,
              0.0);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);

  Shader* shader = static_cast<Shader*>(glfwGetWindowUserPointer(window));
  shader->use();
  shader->setVec2("resolution", (float)width, (float)height);

  camera.resolution = glm::vec2(width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
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

void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    camera.mouseEnabled = !camera.mouseEnabled;
    if (camera.mouseEnabled) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
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

  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);

  return window;
}

void handleKeyboardInput(GLFWwindow* window, float deltaTime,
                         CellularAutomaton& ca) {
  camera.handleKeyboardInput(window, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_R)) {
    ca.reset();
  }
  if (glfwGetKey(window, GLFW_KEY_Q)) {
    exit(0);
  }
}

int main() {
  GLFWwindow* window = initGLFWGlad();
  initImGui(window);

  Shader shader("../src/vertex_shader.glsl", "../src/fragment_shader.glsl");
  shader.use();
  shader.setVec3("voxelSize", grid);
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

  camera.lookAt(glm::vec3(grid.x / 2.0, grid.y / 2.0, grid.z / 2.0));

  CellularAutomaton ca({grid.x, grid.y, grid.z});

  float resizeDebounceTimer = 0.0;
  const float delay = 0.5;
  bool resizePending = false;

  float lastTime = glfwGetTime();
  float timer = lastTime;
  float limDT = 0;
  int frames = 0;
  std::string fps;

  while (!glfwWindowShouldClose(window)) {
    float nowTime = glfwGetTime();
    float deltaTime = nowTime - lastTime;
    lastTime = nowTime;
    frames++;

    limDT += deltaTime / updateLim;

    while (limDT >= 1.0) {
      ca.update();
      limDT--;
    }

    if (glfwGetTime() - timer > 1.0) {
      timer++;
      fps = std::to_string(frames);
      glfwSetWindowTitle(window, ("Game of Life - " + fps + " FPS").c_str());
      frames = 0;
    }

    if (resizePending) {
      resizeDebounceTimer -= deltaTime;
      if (resizeDebounceTimer <= 0.0) {
        resizePending = false;
        ca.resize(grid);
        shader.use();
        shader.setVec3("voxelSize", grid);
      }
    }

    handleKeyboardInput(window, deltaTime, ca);

    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, ca.currentTexture());
    shader.setInt("voxelData", 0);
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::Begin("Properties");
    ImGui::Text("FPS: %s", fps.c_str());

    ImGui::Text("Camera (%.1f, %.1f, %.1f)", camera.position.x,
                camera.position.y, camera.position.z);
    ImGui::Text("  Speed");
    ImGui::SameLine();
    ImGui::DragFloat("##speed", &camera.movementSpeed, 0.1, 0.1, 0.0);
    ImGui::Text("  Sensitivity");
    ImGui::SameLine();
    ImGui::DragFloat("##sens", &camera.mouseSensitivity, 0.005, 0.1, 0.0);

    ImGui::Text("Grid");
    ImGui::Text("  Size");
    ImGui::SameLine();
    if (ImGui::DragInt3("##gridSize", &grid.x, 1, 1, 512)) {
      resizePending = true;
      resizeDebounceTimer = delay;
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}