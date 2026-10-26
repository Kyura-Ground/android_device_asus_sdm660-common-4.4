/*
 * Copyright (C) 2019 Vyacheslav Vidanov (aka Anomalchik)
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ASUS_SDM660_AIDL_LIGHT_H
#define ASUS_SDM660_AIDL_LIGHT_H

#include <aidl/android/hardware/light/BnLights.h>

#include <cstdint>
#include <fstream>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace light {
namespace implementation {

struct Light : public BnLights {
    Light(std::pair<std::ofstream, uint32_t>&& lcd_backlight, std::ofstream&& red_blink,
          std::ofstream&& red_led, std::ofstream&& green_blink, std::ofstream&& green_led);

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;

  private:
    void setAttentionLight(const HwLightState& state);
    void setBatteryLight(const HwLightState& state);
    void setButtonsBacklight(const HwLightState& state);
    void setLcdBacklight(const HwLightState& state);
    void setNotificationLight(const HwLightState& state);
    void setSpeakerBatteryLightLocked();
    void setSpeakerLightLocked(const HwLightState& state);

    std::pair<std::ofstream, uint32_t> mLcdBacklight;

    // Red led
    std::ofstream mRedBlink;
    std::ofstream mRedLed;

    // Green led
    std::ofstream mGreenBlink;
    std::ofstream mGreenLed;

    HwLightState mAttentionState{};
    HwLightState mBatteryState{};
    HwLightState mNotificationState{};

    std::unordered_map<LightType, std::function<void(const HwLightState&)>> mLights;
    std::mutex mLock;
};

}  // namespace implementation
}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // ASUS_SDM660_AIDL_LIGHT_H
