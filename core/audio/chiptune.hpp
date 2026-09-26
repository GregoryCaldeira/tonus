#pragma once
// Offline chiptune renderer (not real-time safe: it allocates). Used for the boot jingle and
// short UI stings. The future real-time engine lives elsewhere and must not call this.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace tonus::audio {

enum class Wave : uint8_t { Pulse12, Pulse25, Pulse50, Triangle, Noise };

struct Note {
    float start = 0;    // seconds
    float length = 0;   // seconds
    float hz = 0;       // ignored for Noise
    Wave wave = Wave::Pulse50;
    float gain = 1.0f;  // relative
    float decay = 0.3f; // exponential decay time constant, seconds
};

float midiToHz(int midi);

// Renders mono int16 PCM. The output is normalised to `peak` (0..1 of full scale) and ends with
// `tailSeconds` of digital silence. Every note gets a short attack and release (no clicks).
std::vector<int16_t> renderNotes(const std::vector<Note>& notes, int sampleRate, float peak = 0.6f,
                                 float tailSeconds = 0.05f);

// The Tonus boot sting: an E arpeggio into a power-chord hit with a triangle bass (~1.25 s).
std::vector<int16_t> renderBootJingle(int sampleRate);

// Phase-continuous sine oscillator for test tones (real-time safe: no allocation).
class SineOsc {
public:
    void set(float hz, int sampleRate);
    // Writes n interleaved frames with `channels` identical channels.
    void render(int16_t* out, size_t frames, int channels, float gain);

private:
    float phase_ = 0;
    float inc_ = 0;
};

}  // namespace tonus::audio
