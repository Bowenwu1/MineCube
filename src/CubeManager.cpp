#include "CubeManager.hpp"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
// 这样可以确保，当编译器提供了dirent.h的时候(比如mingw),优先使用编译自带的dirent.h, 使用前请重新CMAKE
#include <dirent.h>
#include "Cube.hpp"
#include "GLBufferManager.hpp"
using std::cout;
using std::endl;
CubeManager::CubeManager(unsigned int width, unsigned int height,
                         unsigned int depth, GLfloat sizePerCube)
    : width(width),
      height(height),
      depth(depth),
      sizePerCube(sizePerCube),
      glBufferManager(GLBufferManager::getInstance()) {
    totalCube = width * height * depth;
    auto aCubeVertexDataSize = Cube::VERTEX_PER_CUBE * Cube::COOR_DIMENSION;
    auto aCubeNormDataSize = Cube::VERTEX_PER_CUBE * Cube::NOR_VECTOR_DIMENSION;
    this->glBufferManager.init(totalCube, aCubeVertexDataSize,
                               aCubeNormDataSize);

    // 唯一一次初始化cube的顶点属性
    vector<GLfloat> vertexTemp =
        vector<GLfloat>(Cube::VERTEX_PER_CUBE * Cube::COOR_DIMENSION);
    for (unsigned int i = 0; i < vertexTemp.size(); ++i) {
        vertexTemp[i] = sizePerCube * Cube::cubeVertex[i];
    }
    this->glBufferManager.setCubeData(vertexTemp, Cube::cubeNormal);

    // 初始化cubes数组
    cubes = vector<shared_ptr<Cube>>(totalCube);

    // generate set of cubes position
    cubesOriginalPosition = vector<glm::vec3>(totalCube);
    // 计算遍历的起始点
    GLfloat coorMinX = -1 * sizePerCube * width / 2 + sizePerCube / 2;
    GLfloat coorMinY = -1 * sizePerCube * height / 2 + sizePerCube / 2;
    GLfloat coorMinZ = -1 * sizePerCube * depth / 2 + sizePerCube / 2;
    glm::vec3 startPoint = glm::vec3(coorMinX, coorMinY, coorMinZ);
    for (unsigned int x = 0; x < width; ++x) {
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int z = 0; z < depth; ++z) {
                auto id = getId(x, y, z);
                cubesOriginalPosition[id] =
                    glm::vec3(x * sizePerCube, y * sizePerCube,
                              z * sizePerCube) +
                    startPoint;
            }
        }
    }

    rotateAngleAroundY = 0;
    rotateAngleAroundX = 0;
	rotateAngleAroundZ = 0;
}

CubeManager::~CubeManager() {
    // do nothing
}

shared_ptr<Cube> CubeManager::getCube(unsigned int x, unsigned int y,
                                      unsigned int z) {
    return cubes[getId(x, y, z)];
}

// set cube with vec4 color
void CubeManager::setCube(unsigned int x, unsigned int y, unsigned int z,
                          glm::vec4 color, const unsigned int shaderID) {
    unsigned int&& id = getId(x, y, z);
    auto newCube = shared_ptr<Cube>(new Cube(id, sizePerCube, shaderID));
    newCube->editColor(color.x, color.y, color.z, color.w);
    newCube->setModelMat4(calculateModelMat4(id));
    cubes[id] = newCube;
}

// set cube with vec3 color, alpha default is 1.0
void CubeManager::setCube(unsigned int x, unsigned int y, unsigned int z,
                          glm::vec3 color, const unsigned int shaderID) {
    setCube(x, y, z, glm::vec4(color, 1.0), shaderID);
}

bool CubeManager::isThereACube(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int&& id = getId(x, y, z);
    return !this->cubes[id]->isDeleted();
}

void CubeManager::deleteCube(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int&& id = getId(x, y, z);
    auto temp = cubesOriginalPosition[id];
    cubesOriginalPosition[id] = CubeManager::infPos;
    cubes[id]->setModelMat4(calculateModelMat4(id));
    cubes[id]->isInInf = true;
    cubesOriginalPosition[id] = temp;
}

/**
 * This may be a bottleneck of performance.
 * We should optimize here later.
 */
void CubeManager::draw() {
    this->glBufferManager.bufferData();
    this->glBufferManager.bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, totalCube);
    this->glBufferManager.unbind();
}

unsigned int CubeManager::getId(unsigned int x, unsigned int y,
                                unsigned int z) {
    // check arguments
    if (x >= width || y >= height || z >= depth) {
        throw std::out_of_range(
            "OUT_OF_RANGE:Please make input argument is in legal range.");
    }
    return x * (height * depth) + y * depth + z;
}

unsigned int CubeManager::getId(unsigned int x, unsigned int y, unsigned int z,
                                unsigned int w) {
    if (x >= width || y >= height || z >= depth || w >= 6) {
        throw std::out_of_range(
            "OUT_OF_RANGE:Please make input argument is in legal range.");
    }
    return x * (height * depth * 6) + y * (depth * 6) + z * 6 + w;
}

// generate default cube for each position
void CubeManager::defalut_init_all(const GLuint& shaderID,
                                   const string& mat4Name) {
    for (unsigned int i = 0; i < totalCube; ++i) {
        cubes[i] = shared_ptr<Cube>(new Cube(i, sizePerCube, shaderID));
        cubes[i]->cubeColor2Buffer();
    }
    refreshModelMat4();
}

void CubeManager::refreshModelMat4() {
    for (unsigned int i = 0; i < totalCube; ++i) {
        if (!cubes[i]->isDeleted()) {
            cubes[i]->setModelMat4(calculateModelMat4(i));
        }
    }
}

void CubeManager::refreshModelMat4(const unsigned int& id) {
    if (cubes[id] == nullptr) {
        throw std::invalid_argument(
            "INVALID:There is no cube in this position!");
    }
    cubes[id]->setModelMat4(calculateModelMat4(id));
}

glm::mat4 CubeManager::calculateModelMat4(const unsigned int& id) {
    glm::mat4 model;
    model = glm::rotate(model, glm::radians(rotateAngleAroundY), yAxis);
    model = glm::rotate(model, glm::radians(rotateAngleAroundX), xAxis);
	model = glm::rotate(model, glm::radians(rotateAngleAroundZ), zAxis);
	return glm::translate(model, cubesOriginalPosition[id]);
}

void CubeManager::setAllShaderId(const GLuint& shaderID) {
    for (unsigned int i = 0; i < totalCube; ++i) {
        if (cubes[i] != nullptr) {
            cubes[i]->setShaderId(shaderID);
        }
    }
}

void CubeManager::rotateVertical(const GLfloat& offset) {
    rotateAngleAroundX += offset * rotateSensivitiy;
    this->refreshModelMat4();
}

void CubeManager::rotateHorizontal(const GLfloat& offset) {
    rotateAngleAroundY += offset * rotateSensivitiy;
    this->refreshModelMat4();
}

void CubeManager::rotateAll(const glm::vec2& offset) {
	if (315.0f <= rotateAngleAroundX || rotateAngleAroundX < 45.0f) {
		if (315.0f <= rotateAngleAroundY || rotateAngleAroundY < 45.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundY += offset.y * rotateSensivitiy;
				rotateAngleAroundX += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundY += offset.y * rotateSensivitiy;
				rotateAngleAroundX += offset.x * rotateSensivitiy;
			}
		}
		else if (45.0f <= rotateAngleAroundY && rotateAngleAroundY < 135.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else if (135.0f <= rotateAngleAroundY && rotateAngleAroundY < 225.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
	}
	else if (45.0f <= rotateAngleAroundX && rotateAngleAroundX < 135.0f) {
		if (315.0f <= rotateAngleAroundY || rotateAngleAroundY < 45.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundZ -= offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundZ -= offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundZ -= offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundZ -= offset.x * rotateSensivitiy;
			}
		}
		else if (45.0f <= rotateAngleAroundY && rotateAngleAroundY < 135.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundZ += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else if (135.0f <= rotateAngleAroundY && rotateAngleAroundY < 225.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
	}
	else if (135.0f <= rotateAngleAroundX && rotateAngleAroundX < 225.0f) {
		if (315.0f <= rotateAngleAroundY || rotateAngleAroundY < 45.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;  
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else if (45.0f <= rotateAngleAroundY && rotateAngleAroundY < 135.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundZ -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else if (135.0f <= rotateAngleAroundY && rotateAngleAroundY < 225.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
	}
	else {
		if (315.0f <= rotateAngleAroundY || rotateAngleAroundY < 45.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;  
				rotateAngleAroundZ += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundZ += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundZ += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX += offset.y * rotateSensivitiy;
				rotateAngleAroundZ += offset.x * rotateSensivitiy;
			}
		}
		else if (45.0f <= rotateAngleAroundY && rotateAngleAroundY < 135.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else if (135.0f <= rotateAngleAroundY && rotateAngleAroundY < 225.0f) {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundX -= offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
		else {
			if (315.0f <= rotateAngleAroundZ || rotateAngleAroundZ < 45.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;  
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (45.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 135.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else if (135.0f <= rotateAngleAroundZ && rotateAngleAroundZ < 225.0f) {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
			else {
				rotateAngleAroundZ += offset.y * rotateSensivitiy;
				rotateAngleAroundY += offset.x * rotateSensivitiy;
			}
		}
	}

	if (rotateAngleAroundX > 360.0f) rotateAngleAroundX -= 360.0f; if (rotateAngleAroundX < 0.0f) rotateAngleAroundX = 360.0f - rotateAngleAroundX;
	if (rotateAngleAroundY > 360.0f) rotateAngleAroundY -= 360.0f; if (rotateAngleAroundY < 0.0f) rotateAngleAroundY = 360.0f - rotateAngleAroundY;
	if (rotateAngleAroundZ > 360.0f) rotateAngleAroundZ -= 360.0f; if (rotateAngleAroundZ < 0.0f) rotateAngleAroundZ = 360.0f - rotateAngleAroundZ;
	//cout << rotateAngleAroundX << ' ' << rotateAngleAroundY << ' ' << rotateAngleAroundZ << endl;
	for (unsigned int i = 0; i < totalCube; ++i) {
		if (!cubes[i]->isDeleted()) {

			glm::mat4 model;
			model = glm::rotate(model, glm::radians(rotateAngleAroundY), yAxis);
			model = glm::rotate(model, glm::radians(rotateAngleAroundX), xAxis);
			model = glm::rotate(model, glm::radians(rotateAngleAroundZ), zAxis);
			glm::mat4 final_mat = glm::translate(model, cubesOriginalPosition[i]);

			cubes[i]->setModelMat4(final_mat);
		}
	}
}

void CubeManager::setRotateSensivity(const GLfloat& rotateSensivitiy) {
    this->rotateSensivitiy = rotateSensivitiy;
}

glm::mat4 CubeManager::getModelMat4(unsigned int x, unsigned int y,
                                    unsigned int z) {
    return calculateModelMat4(this->getId(x, y, z));
}
const glm::vec3 CubeManager::yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 CubeManager::xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 CubeManager::zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
const glm::vec3 CubeManager::infPos = glm::vec3(3000.0f, 3000.0f, 3000.0f);

void CubeManager::dump(string model_path) {
    json j;
    j["width"] = width;
    j["height"] = height;
    j["depth"] = depth;
    j["totalCube"] = totalCube;
    j["sizePerCube"] = sizePerCube;
    j["rotateAngleAroundX"] = rotateAngleAroundX;
    j["rotateAngleAroundY"] = rotateAngleAroundY;
    j["rotateSensivitiy"] = rotateSensivitiy;
    json j_cubeOriginalPosition = json::array();
    for (unsigned int i = 0; i < cubesOriginalPosition.size(); ++i) {
        glm::vec3 & temp = cubesOriginalPosition[i];
        j_cubeOriginalPosition.push_back({temp.x, temp.y, temp.z});
    }
    j["cubesOriginalPosition"] = j_cubeOriginalPosition;
    json j_cubes = json::array();
    for (unsigned int i = 0; i < cubes.size(); ++i) {
        json cube;
        auto ptr_cube = cubes[i];
        cube["shaderID"] = ptr_cube->shaderID;
        cube["id"] = ptr_cube->id;
        cube["isInInf"] = ptr_cube->isInInf;
        glm::vec4 color = ptr_cube->cubeColor;
        /** 
         * Bug by Bowen Wu
         * Since we have created a lot model under this bug, and value of alpha
         * not change, we fix this problem in a tricky way
         * Notice this in load model
         */
        // cube["cubeColor"] = {color.x, color.y, color.z, color.y};
        cube["cubeColor"] = {color.x, color.y, color.z, color.w};
        j_cubes.push_back(cube);
    }
    j["cubes"] = j_cubes;
    // cout << j.dump() << endl;
    try {
    std::ofstream out(model_saved_dir + model_path);
    out << j.dump();
    out.close();
    cout << "Model Saved To " << model_path << endl;
    } catch(...) {
        cout << "ERROR::OPEN_FILE_FAIL" << endl;
        return;
}
}

bool CubeManager::load(string model_path) {
    string in_data;
    try {
        std::ifstream in(model_saved_dir + model_path);
    in >> in_data;
        // recover from json
    auto j = json::parse(in_data);

    // basic variable
    width = j["width"];
    height = j["height"];
    depth = j["depth"];
    totalCube = j["totalCube"];
    sizePerCube = j["sizePerCube"];
    rotateAngleAroundX = j["rotateAngleAroundX"];
    rotateAngleAroundY = j["rotateAngleAroundY"];
    rotateSensivitiy = j["rotateSensivitiy"];

	numPerEdge = width;

    totalCube = width * height * depth;
    auto aCubeVertexDataSize = Cube::VERTEX_PER_CUBE * Cube::COOR_DIMENSION;
    auto aCubeNormDataSize = Cube::VERTEX_PER_CUBE * Cube::NOR_VECTOR_DIMENSION;
    this->glBufferManager.init(totalCube, aCubeVertexDataSize,
                               aCubeNormDataSize);
    
    // recover origin positions
    auto j_cubeOriginalPosition = j["cubesOriginalPosition"];
    cubesOriginalPosition = vector<glm::vec3>(totalCube);
    for (unsigned int i = 0; i < cubesOriginalPosition.size(); ++i) {
        auto & temp = cubesOriginalPosition[i];
        auto & in_data = j_cubeOriginalPosition[i];
        temp.x = in_data[0];
        temp.y = in_data[1];
        temp.z = in_data[2];
    }

    // recover cubes
    auto j_cubes = j["cubes"];
    cubes = vector<shared_ptr<Cube>>(totalCube);
    for (unsigned int i = 0; i < cubes.size(); ++i) {
        auto j_c = j_cubes[i];
        cubes[i] = shared_ptr<Cube>(new Cube(i, sizePerCube, j_c["shaderID"]));
        cubes[i]->shaderID = j_c["shaderID"];
        cubes[i]->isInInf = j_c["isInInf"];
        if (cubes[i]->isInInf) {
            auto temp = cubesOriginalPosition[i];
            cubesOriginalPosition[i] = CubeManager::infPos;
            cubes[i]->setModelMat4(calculateModelMat4(i));
            cubes[i]->isInInf = true;
            cubesOriginalPosition[i] = temp;
        }
        auto & in_data = j_c["cubeColor"];
        /**
         * To temporarily fix the bug of dump model.
         * We do not dump correct alpha value when dump model
         */
        // cubes[i]->cubeColor = glm::vec4(in_data[0], in_data[1], in_data[2], in_data[3]);
        cubes[i]->cubeColor = glm::vec4(in_data[0], in_data[1], in_data[2], 1.0f);
        cubes[i]->cubeColor2Buffer();
    }
    refreshModelMat4();
        return true;
    } catch(...) {
        cout << "ERROR:OPEN::FILE::FAIL" << endl;
        return false;
    }
}

vector<string> CubeManager::getModels() {
    vector<string> result = vector<string>();
    struct dirent *ptr;
    DIR *dir;
    dir = opendir(model_saved_dir.c_str());
    while ((ptr=readdir(dir)) != NULL) {
        if (ptr->d_name[0] == '.')
            continue;
        // cout << ptr->d_name << endl;
        result.push_back(string(ptr->d_name));

    }
    closedir(dir);
    return result;
}
#ifdef __APPLE__
const string CubeManager::model_saved_dir = "/Users/wubowen/Documents/MineCube/Asset/example/";
#else
const string CubeManager::model_saved_dir = "../Asset/example/";
#endif
