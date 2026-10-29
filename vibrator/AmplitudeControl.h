/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include <utility>

class AmplitudeControl {
  public:
    explicit AmplitudeControl(std::string path) : mPath(std::move(path)) {
        std::ifstream node(mPath);
        unsigned int value;
        if (node >> value && value > 0 && value <= kMaximumAmplitude) {
            // Restore the driver default when the HAL starts or restarts.
            mAvailable = write(kMaximumAmplitude);
        }
    }

    bool available() const { return mAvailable; }

    static bool valid(float amplitude) {
        return std::isfinite(amplitude) && amplitude > 0.0f && amplitude <= 1.0f;
    }

    bool set(float amplitude) const {
        if (!mAvailable || !valid(amplitude)) return false;
        const auto value = static_cast<unsigned int>(std::lround(amplitude * kMaximumAmplitude));
        return write(std::clamp(value, 1u, kMaximumAmplitude));
    }

    bool reset() const { return !mAvailable || write(kMaximumAmplitude); }

  private:
    bool write(unsigned int value) const {
        std::ofstream node(mPath);
        node << value;
        node.flush();
        return node.good();
    }

    static constexpr unsigned int kMaximumAmplitude = 255;
    const std::string mPath;
    bool mAvailable = false;
};
