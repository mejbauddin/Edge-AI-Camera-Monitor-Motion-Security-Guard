#include "EmbeddingMath.hpp"

#include <cmath>
#include <numeric>

namespace csx::recognition {

float l2Distance(const std::vector<float>& a, const std::vector<float>& b) {
    const auto count = std::min(a.size(), b.size());
    if (count == 0) {
        return 1.0F;
    }

    float sum = 0.0F;
    for (std::size_t i = 0; i < count; ++i) {
        const float delta = a[i] - b[i];
        sum += delta * delta;
    }
    return std::sqrt(sum);
}

float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    const auto count = std::min(a.size(), b.size());
    if (count == 0) {
        return 0.0F;
    }

    float dot = 0.0F;
    float normA = 0.0F;
    float normB = 0.0F;
    for (std::size_t i = 0; i < count; ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    if (normA <= 0.0F || normB <= 0.0F) {
        return 0.0F;
    }
    return dot / (std::sqrt(normA) * std::sqrt(normB));
}

void normalizeEmbedding(std::vector<float>& embedding) {
    const float norm = std::sqrt(
        std::accumulate(embedding.begin(), embedding.end(), 0.0F,
                        [](const float acc, const float value) { return acc + value * value; }));
    if (norm <= 0.0F) {
        return;
    }
    for (float& value : embedding) {
        value /= norm;
    }
}

std::vector<float> averageEmbeddings(const std::vector<std::vector<float>>& samples) {
    if (samples.empty()) {
        return {};
    }

    std::vector<float> average(samples.front().size(), 0.0F);
    for (const auto& sample : samples) {
        for (std::size_t i = 0; i < average.size() && i < sample.size(); ++i) {
            average[i] += sample[i];
        }
    }

    const float count = static_cast<float>(samples.size());
    for (float& value : average) {
        value /= count;
    }
    normalizeEmbedding(average);
    return average;
}

}  // namespace csx::recognition
