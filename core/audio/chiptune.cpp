#include "audio/chiptune.hpp"

#include <algorithm>
#include <cmath>

namespace tonus::audio {
namespace {

constexpr float kTwoPi = 6.28318530717958647692f;
constexpr float kAttack = 0.003f;
constexpr float kRelease = 0.010f;

float oscillator(Wave wave, float phase01, uint32_t& noiseState) {
    switch (wave) {
        case Wave::Pulse12: return phase01 < 0.125f ? 1.0f : -1.0f;
        case Wave::Pulse25: return phase01 < 0.25f ? 1.0f : -1.0f;
        case Wave::Pulse50: return phase01 < 0.5f ? 1.0f : -1.0f;
        case Wave::Triangle: return phase01 < 0.5f ? 4.0f * phase01 - 1.0f : 3.0f - 4.0f * phase01;
        case Wave::Noise:
            noiseState = noiseState * 1664525u + 1013904223u;
            return static_cast<float>(noiseState >> 8) / 8388608.0f - 1.0f;
    }
    return 0;
}

}  // namespace

float midiToHz(int midi) { return 440.0f * std::pow(2.0f, (midi - 69) / 12.0f); }

std::vector<int16_t> renderNotes(const std::vector<Note>& notes, int sampleRate, float peak,
                                 float tailSeconds) {
    float end = 0;
    for (const Note& n : notes) end = std::max(end, n.start + n.length);
    const size_t body = static_cast<size_t>(std::ceil(end * sampleRate));
    const size_t total = body + static_cast<size_t>(tailSeconds * sampleRate);

    std::vector<float> mix(total, 0.0f);
    uint32_t noise = 0x7015u;
    for (const Note& n : notes) {
        const size_t s0 = static_cast<size_t>(n.start * sampleRate);
        const size_t len = static_cast<size_t>(n.length * sampleRate);
        const float inc = n.hz / sampleRate;
        float phase = 0;
        for (size_t i = 0; i < len && s0 + i < body; ++i) {
            const float t = static_cast<float>(i) / sampleRate;
            const float remaining = n.length - t;
            float env = std::exp(-t / std::max(n.decay, 1e-3f));
            if (t < kAttack) env *= t / kAttack;
            if (remaining < kRelease) env *= std::max(0.0f, remaining / kRelease);
            mix[s0 + i] += n.gain * env * oscillator(n.wave, phase, noise);
            phase += inc;
            phase -= std::floor(phase);
        }
    }

    float maxAbs = 0;
    for (float v : mix) maxAbs = std::max(maxAbs, std::fabs(v));
    const float scale = maxAbs > 0 ? peak * 32767.0f / maxAbs : 0.0f;

    std::vector<int16_t> out(total);
    for (size_t i = 0; i < total; ++i) out[i] = static_cast<int16_t>(std::lrint(mix[i] * scale));
    return out;
}

std::vector<int16_t> renderBootJingle(int sampleRate) {
    constexpr float step = 0.085f;  // 16th notes at ~176 BPM
    std::vector<Note> notes;
    // Pick-up arpeggio: E4 B4 E5 G5 B5
    const int arp[] = {64, 71, 76, 79, 83};
    for (int i = 0; i < 5; ++i)
        notes.push_back({i * step, step * 1.4f, midiToHz(arp[i]), Wave::Pulse25, 0.45f, 0.12f});

    // Power-chord hit (E5 + B5 + E6) with a triangle bass on E2 and a short noise "crash"
    const float hit = 5 * step + 0.02f;
    notes.push_back({hit, 0.75f, midiToHz(76), Wave::Pulse25, 0.40f, 0.35f});
    notes.push_back({hit, 0.75f, midiToHz(83), Wave::Pulse12, 0.30f, 0.35f});
    notes.push_back({hit, 0.75f, midiToHz(88), Wave::Pulse50, 0.18f, 0.25f});
    notes.push_back({hit, 0.80f, midiToHz(40), Wave::Triangle, 0.90f, 0.45f});
    notes.push_back({hit, 0.25f, 0.0f, Wave::Noise, 0.25f, 0.06f});
    return renderNotes(notes, sampleRate);
}

void SineOsc::set(float hz, int sampleRate) { inc_ = kTwoPi * hz / static_cast<float>(sampleRate); }

void SineOsc::render(int16_t* out, size_t frames, int channels, float gain) {
    const float amp = std::clamp(gain, 0.0f, 1.0f) * 32767.0f;
    for (size_t i = 0; i < frames; ++i) {
        const int16_t v = static_cast<int16_t>(std::lrint(std::sin(phase_) * amp));
        for (int c = 0; c < channels; ++c) *out++ = v;
        phase_ += inc_;
        if (phase_ >= kTwoPi) phase_ -= kTwoPi;
    }
}

}  // namespace tonus::audio
