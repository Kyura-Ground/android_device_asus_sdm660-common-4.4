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

#define LOG_TAG "android.hardware.light-service.asus_sdm660"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Light.h"

using aidl::android::hardware::light::ILights;
using aidl::android::hardware::light::implementation::Light;

// LCD
const static std::string kLcdBacklightPath = "/sys/class/leds/lcd-backlight/brightness";
const static std::string kLcdMaxBacklightPath = "/sys/class/leds/lcd-backlight/max_brightness";

// Red led
const static std::string kRedBlinkPath = "/sys/class/leds/red/blink";
const static std::string kRedLedPath = "/sys/class/leds/red/brightness";

// Green led
const static std::string kGreenBlinkPath = "/sys/class/leds/green/blink";
const static std::string kGreenLedPath = "/sys/class/leds/green/brightness";

int main() {
    uint32_t lcdMaxBrightness = 255;

    std::ofstream lcdBacklight(kLcdBacklightPath);
    if (!lcdBacklight) {
        LOG(ERROR) << "Failed to open " << kLcdBacklightPath << ", error=" << errno << " ("
                   << strerror(errno) << ")";
        return -errno;
    }

    std::ifstream lcdMaxBacklight(kLcdMaxBacklightPath);
    if (!lcdMaxBacklight) {
        LOG(ERROR) << "Failed to open " << kLcdMaxBacklightPath << ", error=" << errno << " ("
                   << strerror(errno) << ")";
        return -errno;
    } else {
        lcdMaxBacklight >> lcdMaxBrightness;
    }

    std::ofstream redBlink(kRedBlinkPath);
    if (!redBlink) {
        LOG(ERROR) << "Failed to open " << kRedBlinkPath << ", error=" << errno << " ("
                   << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redLed(kRedLedPath);
    if (!redLed) {
        LOG(ERROR) << "Failed to open " << kRedLedPath << ", error=" << errno << " ("
                   << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream greenBlink(kGreenBlinkPath);
    if (!greenBlink) {
        LOG(ERROR) << "Failed to open " << kGreenBlinkPath << ", error=" << errno << " ("
                   << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream greenLed(kGreenLedPath);
    if (!greenLed) {
        LOG(ERROR) << "Failed to open " << kGreenLedPath << ", error=" << errno << " ("
                   << strerror(errno) << ")";
        return -errno;
    }

    auto service = ndk::SharedRefBase::make<Light>(
        std::make_pair(std::move(lcdBacklight), lcdMaxBrightness), std::move(redBlink),
        std::move(redLed), std::move(greenBlink), std::move(greenLed));

    ABinderProcess_setThreadPoolMaxThreadCount(0);
    const std::string instance = std::string(ILights::descriptor) + "/default";
    binder_status_t status = AServiceManager_addService(service->asBinder().get(), instance.c_str());

    if (status != STATUS_OK) {
        LOG(ERROR) << "Cannot register Light HAL service";
        return 1;
    }

    LOG(INFO) << "Light HAL Ready.";
    ABinderProcess_joinThreadPool();
    // Under normal cases, execution will not reach this line.
    LOG(ERROR) << "Light HAL failed to join thread pool.";
    return 1;
}
