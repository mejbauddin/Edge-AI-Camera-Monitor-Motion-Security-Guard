#pragma once

#include "types/Frame.hpp"

#include <functional>
#include <memory>
#include <string>

namespace csx::core {

class ICameraSource {
public:
    virtual ~ICameraSource() = default;

    virtual bool open(const std::string& sourceUri) = 0;
    virtual void close() = 0;
    virtual bool grab(Frame& outFrame) = 0;
    [[nodiscard]] virtual bool isOpen() const = 0;
    [[nodiscard]] virtual CameraDiagnosticsSnapshot diagnostics() const = 0;
};

class IMotionEngine {
public:
    virtual ~IMotionEngine() = default;
    virtual void process(const Frame& frame, std::vector<Rect2f>& outBlobs) = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class ITracker {
public:
    virtual ~ITracker() = default;
    virtual void update(const Frame& frame, const std::vector<Rect2f>& detections,
                        std::vector<Track>& outTracks) = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IFaceRecognizer {
public:
    virtual ~IFaceRecognizer() = default;
    virtual void recognize(const Frame& frame, const std::vector<Track>& tracks,
                           std::vector<FaceMatch>& outMatches) = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IBehaviorEngine {
public:
    virtual ~IBehaviorEngine() = default;
    virtual void analyze(const Frame& frame, const std::vector<Track>& tracks,
                         std::vector<BehaviorAnomaly>& outAnomalies) = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IThreatEngine {
public:
    virtual ~IThreatEngine() = default;
    virtual ThreatAssessment assess(const std::vector<Track>& tracks,
                                    const std::vector<FaceMatch>& faces,
                                    const std::vector<BehaviorAnomaly>& anomalies) = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IRecorder {
public:
    virtual ~IRecorder() = default;
    virtual void feedFrame(const Frame& frame) = 0;
    virtual void triggerRecording(const ThreatAssessment& assessment) = 0;
    virtual void startRecording() = 0;
    virtual void stopRecording() = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual bool open(const std::string& path) = 0;
    virtual void close() = 0;
    [[nodiscard]] virtual bool isOpen() const = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IConfigService {
public:
    virtual ~IConfigService() = default;
    virtual bool load(const std::string& path) = 0;
    virtual bool reload() = 0;
    [[nodiscard]] virtual std::string getString(const std::string& key,
                                                const std::string& defaultValue) const = 0;
    [[nodiscard]] virtual int getInt(const std::string& key, int defaultValue) const = 0;
    [[nodiscard]] virtual float getFloat(const std::string& key, float defaultValue) const = 0;
    [[nodiscard]] virtual bool getBool(const std::string& key, bool defaultValue) const = 0;
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void trace(const std::string& module, const std::string& message) = 0;
    virtual void debug(const std::string& module, const std::string& message) = 0;
    virtual void info(const std::string& module, const std::string& message) = 0;
    virtual void warn(const std::string& module, const std::string& message) = 0;
    virtual void error(const std::string& module, const std::string& message) = 0;
    virtual void aiDecision(const std::string& module, const std::string& message) = 0;
    virtual void performance(const std::string& module, const std::string& message) = 0;
};

class IVoiceService {
public:
    virtual ~IVoiceService() = default;
    virtual void speak(const std::string& phrase, int priority = 0) = 0;
    virtual void speakAsync(const std::string& phrase, int priority = 0) = 0;
    virtual void setEnabled(bool enabled) = 0;
    [[nodiscard]] virtual bool isEnabled() const noexcept = 0;
    virtual void stop() = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

using EventCallback = std::function<void(const std::string& topic, const std::string& payload)>;

class IEventBus {
public:
    virtual ~IEventBus() = default;
    virtual void subscribe(const std::string& topic, EventCallback callback) = 0;
    virtual void publish(const std::string& topic, const std::string& payload) = 0;
    virtual void unsubscribeAll(const std::string& topic) = 0;
};

/// HUD overlay compositor — draws neon tactical overlay onto a BGR frame.
/// The output Frame owns its own bgrData copy so callers may hold it independently
/// of the input frame's lifetime.
class IHudRenderer {
public:
    virtual ~IHudRenderer() = default;

    /// Composite the HUD onto inputFrame and write the result to outFrame.
    /// outFrame.bgrData will be allocated/resized as needed.
    virtual void render(const Frame& inputFrame,
                        const std::vector<Track>& tracks,
                        const std::vector<FaceMatch>& faces,
                        const std::vector<BehaviorAnomaly>& anomalies,
                        const ThreatAssessment& assessment,
                        const SystemHealth& health,
                        Frame& outFrame) = 0;

    /// Toggle recording indicator blink (called from recorder module).
    virtual void setRecording(bool recording) noexcept = 0;

    [[nodiscard]] virtual EngineHealth health() const = 0;
};

class IAlertManager {
public:
    virtual ~IAlertManager() = default;
    virtual void onThreatAssessment(const ThreatAssessment& assessment) = 0;
    virtual void onFaceMatch(const FaceMatch& match) = 0;
    virtual void onMotionDetected(const std::vector<Track>& tracks) = 0;
    virtual void setVoiceService(IVoiceService* voice) = 0;
    virtual void setRecorder(IRecorder* recorder) = 0;
    virtual void acknowledgeAlert(const std::string& alertId) = 0;
    virtual void clearAcknowledged() = 0;
    [[nodiscard]] virtual EngineHealth health() const = 0;
};

}  // namespace csx::core
