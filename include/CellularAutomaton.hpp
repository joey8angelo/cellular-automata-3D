#include "computeShader.hpp"

bool randomProb(float probability) {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) <
         probability;
}

const float pr = 0.499f;

class CellularAutomaton {
 private:
  const char* computePath;
  GLuint textures[2];
  short read, write;
  glm::ivec3 size;

 public:
  ComputeShader computeShader;
  CellularAutomaton(const glm::ivec3& size)
      : computePath("../src/compute.glsl"),
        size(size),
        computeShader(computePath) {
    initTextures();
  }

  void resize(const glm::ivec3& s) {
    size = s;
    glDeleteTextures(2, textures);
    initTextures();
  }

  int getIdx(const glm::ivec3& p) {
    return p.z * size.y * size.x + p.y * size.x + p.x;
  }

  int cellCount() { return size.x * size.y * size.z; }

  void reset() {
    std::vector<GLubyte> initData(cellCount());
    for (int i = 0; i < cellCount(); i++) {
      initData[i] = randomProb(pr) ? 1 : 0;
    }
    glBindTexture(GL_TEXTURE_3D, textures[read]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, size.x, size.y, size.z, 0,
                 GL_RED_INTEGER, GL_UNSIGNED_BYTE, initData.data());
  }

  void update() {
    computeShader.use();

    glBindImageTexture(0, textures[write], 0, GL_TRUE, 0, GL_WRITE_ONLY,
                       GL_R8UI);
    glBindImageTexture(1, textures[read], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R8UI);

    glDispatchCompute((size.x + 7) / 8, (size.y + 7) / 8, (size.z + 7) / 8);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    std::swap(read, write);
  }

  void printTextureContents(int textureIndex, int sliceZ = 0) {
    std::vector<GLubyte> data(cellCount());

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_3D, textures[textureIndex]);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                  data.data());

    std::cout << "Texture " << textureIndex << " (slice z=" << sliceZ
              << "):" << std::endl;

    // Print 2D slice of the 3D texture
    for (int y = 0; y < size.y; y++) {
      for (int x = 0; x < size.x; x++) {
        int idx = getIdx(glm::ivec3(x, y, sliceZ));
        std::cout << (int)data[idx] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  void printReadTextureContents(int sliceZ = 0) {
    printTextureContents(read, sliceZ);
  }

  GLuint currentTexture() const { return textures[read]; }

 private:
  void initTextures() {
    read = 0;
    write = 1;
    glGenTextures(2, textures);

    srand(time(NULL));
    std::vector<GLubyte> initData(cellCount());
    for (int i = 0; i < cellCount(); i++) {
      initData[i] = randomProb(pr) ? 1 : 0;
    }

    for (int i = 0; i < 2; i++) {
      glBindTexture(GL_TEXTURE_3D, textures[i]);

      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, size.x, size.y, size.z, 0,
                   GL_RED_INTEGER, GL_UNSIGNED_BYTE, initData.data());
    }
  }
};