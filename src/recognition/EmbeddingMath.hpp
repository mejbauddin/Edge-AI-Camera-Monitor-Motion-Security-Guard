#pragma once

#include <vector>

namespace csx::recognition {

[[nodiscard]] float l2Distance(const std::vector<float>& a, const std::vector<float>& b);
[[nodiscard]] float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
void normalizeEmbedding(std::vector<float>& embedding);
[[nodiscard]] std::vector<float> averageEmbeddings(const std::vector<std::vector<float>>& samples);

}  // namespace csx::recognition
