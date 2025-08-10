#pragma once

#include <glm/glm.hpp>

#include <vector>

struct GridCell {
    std::vector<int> obstacleIndices;
};

class SpatialGrid {
public:
    SpatialGrid(float cellSize, glm::vec3 minBounds, glm::vec3 maxBounds)
        : cellSize(cellSize), minBounds(minBounds) {
        glm::vec3 size = maxBounds - minBounds;
        cellsX = static_cast<int>(std::ceil(size.x / cellSize));
        cellsY = static_cast<int>(std::ceil(size.y / cellSize));
        cellsZ = static_cast<int>(std::ceil(size.z / cellSize));
        cells.resize(cellsX * cellsY * cellsZ);
    }

    void insertObstacle(int index, const glm::vec3& pos) {
        glm::ivec3 c = toCellCoords(pos);
        cells[cellIndex(c)].obstacleIndices.push_back(index);
    }

    std::vector<int> queryNearby(const glm::vec3& pos, float radius) {
        glm::ivec3 cmin = toCellCoords(pos - glm::vec3(radius));
        glm::ivec3 cmax = toCellCoords(pos + glm::vec3(radius));

        std::vector<int> results;
        for (int x = cmin.x; x <= cmax.x; x++) {
            for (int y = cmin.y; y <= cmax.y; y++) {
                for (int z = cmin.z; z <= cmax.z; z++) {
                    if (inBounds(x, y, z)) {
                        auto& cell = cells[cellIndex({x, y, z})];
                        results.insert(results.end(),
                                       cell.obstacleIndices.begin(),
                                       cell.obstacleIndices.end());
                    }
                }
            }
        }
        return results;
    }

private:
    float cellSize;
    glm::vec3 minBounds;
    int cellsX, cellsY, cellsZ;
    std::vector<GridCell> cells;

    glm::ivec3 toCellCoords(const glm::vec3& pos) const {
        glm::vec3 rel = pos - minBounds;
        return glm::ivec3(rel.x / cellSize, rel.y / cellSize, rel.z / cellSize);
    }

    bool inBounds(int x, int y, int z) const {
        return x >= 0 && y >= 0 && z >= 0 &&
               x < cellsX && y < cellsY && z < cellsZ;
    }
    
    int cellIndex(glm::ivec3 c) const {
        return (c.z * cellsY * cellsX) + (c.y * cellsX) + c.x;
    }
};