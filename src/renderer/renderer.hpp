#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace Renderer {
    void Init();
    void Cleanup();
    void UpdateProjection(int width, int height);
    void RenderFrame(const std::vector<glm::vec2>& positions, const std::vector<float>& radii);
}
