/*
  ==============================================================================

    Octaver.h
    Created: 29 May 2026
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <cmath>
#include <vector>

class Octaver {
  public:
    inline static constexpr float MinOctaveGainDb = -60.0f;
    inline static constexpr float MaxOctaveGainDb = 12.0f;
    inline static constexpr float DefaultOctaveGainDb = 0.0f;

    enum class Shift {
        twoDown = 0,
        oneDown,
        oneUp,
        twoUp
    };

    Octaver() noexcept
    {
        setOctaveGainDb(DefaultOctaveGainDb);
        setShift(Shift::oneDown);
    }

    void prepare(double sampleRate, int maximumBlockSize, int numChannels)
    {
        juce::ignoreUnused(maximumBlockSize);

        sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
        delaySize_ = juce::jmax(4, static_cast<int>(std::ceil(sampleRate_ * 0.25)));
        writePosition_ = 0;

        delayBuffer_.assign(static_cast<size_t>(juce::jmax(1, numChannels)),
                            std::vector<float>(static_cast<size_t>(delaySize_), 0.0f));
        readPositions_.assign(delayBuffer_.size(), static_cast<float>(delaySize_) * 0.5f);
    }

    void reset() noexcept
    {
        for (auto& channel : delayBuffer_)
            std::fill(channel.begin(), channel.end(), 0.0f);

        writePosition_ = 0;
        std::fill(readPositions_.begin(), readPositions_.end(),
                  static_cast<float>(delaySize_) * 0.5f);
    }

    void setOctaveGainDb(float gainDb) noexcept
    {
        octaveGainDb_ = std::clamp(gainDb, MinOctaveGainDb, MaxOctaveGainDb);
        octaveGainLinear_ = std::pow(10.0f, octaveGainDb_ / 20.0f);
    }

    void setShift(Shift shift) noexcept
    {
        shift_ = shift;

        switch (shift_)
        {
            case Shift::twoDown: pitchRatio_ = 0.25f; break;
            case Shift::oneDown: pitchRatio_ = 0.5f; break;
            case Shift::oneUp:   pitchRatio_ = 2.0f; break;
            case Shift::twoUp:   pitchRatio_ = 4.0f; break;
        }
    }

    void setShiftFromChoiceIndex(int choiceIndex) noexcept
    {
        switch (choiceIndex)
        {
            case 0: setShift(Shift::twoDown); break;
            case 1: setShift(Shift::oneDown); break;
            case 2: setShift(Shift::oneUp); break;
            case 3: setShift(Shift::twoUp); break;
            default: setShift(Shift::oneDown); break;
        }
    }

    [[nodiscard]] float getOctaveGainDb() const noexcept { return octaveGainDb_; }

    [[nodiscard]] float getLinearOctaveGain() const noexcept { return octaveGainLinear_; }

    [[nodiscard]] Shift getShift() const noexcept { return shift_; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        ensureChannelCount(buffer.getNumChannels());

        if (delaySize_ <= 0 || buffer.getNumChannels() == 0)
            return;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto& delayChannel = delayBuffer_[static_cast<size_t>(channel)];
                const auto input = buffer.getSample(channel, sample);

                delayChannel[static_cast<size_t>(writePosition_)] = input;

                buffer.setSample(channel, sample,
                                 readInterpolatedSample(channel) * octaveGainLinear_);
            }

            advancePositions();
        }
    }

  private:
    void ensureChannelCount(int numChannels)
    {
        if (numChannels <= 0 || static_cast<size_t>(numChannels) <= delayBuffer_.size())
            return;

        delayBuffer_.resize(static_cast<size_t>(numChannels),
                            std::vector<float>(static_cast<size_t>(delaySize_), 0.0f));
        readPositions_.resize(static_cast<size_t>(numChannels),
                              static_cast<float>(delaySize_) * 0.5f);
    }

    [[nodiscard]] float readInterpolatedSample(int channel) const noexcept
    {
        const auto& delayChannel = delayBuffer_[static_cast<size_t>(channel)];
        const auto readPosition = readPositions_[static_cast<size_t>(channel)];
        const auto index0 = static_cast<int>(readPosition);
        const auto index1 = (index0 + 1) % delaySize_;
        const auto fraction = readPosition - static_cast<float>(index0);

        const auto sample0 = delayChannel[static_cast<size_t>(index0)];
        const auto sample1 = delayChannel[static_cast<size_t>(index1)];

        return sample0 + (sample1 - sample0) * fraction;
    }

    void advancePositions() noexcept
    {
        writePosition_ = (writePosition_ + 1) % delaySize_;

        for (auto& readPosition : readPositions_)
        {
            readPosition += pitchRatio_;

            while (readPosition >= static_cast<float>(delaySize_))
                readPosition -= static_cast<float>(delaySize_);
        }
    }

    double sampleRate_{44100.0};
    int delaySize_{0};
    int writePosition_{0};
    float octaveGainDb_{DefaultOctaveGainDb};
    float octaveGainLinear_{1.0f};
    float pitchRatio_{0.5f};
    Shift shift_{Shift::oneDown};
    std::vector<std::vector<float>> delayBuffer_;
    std::vector<float> readPositions_;
};
