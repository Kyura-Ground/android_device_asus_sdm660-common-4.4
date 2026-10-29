/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Vibrator.h>
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <mutex>

#include "AmplitudeControl.h"

using aidl::android::hardware::vibrator::IVibrator;
using aidl::android::hardware::vibrator::IVibratorCallback;
using aidl::android::hardware::vibrator::Vibrator;

namespace {

class AsusVibrator final : public Vibrator {
  public:
    AsusVibrator() : mAmplitude("/sys/class/leds/vibrator/amplitude") {
        LOG(INFO) << "QPNP amplitude control: " << mAmplitude.available();
    }

    ndk::ScopedAStatus getCapabilities(int32_t* capabilities) override {
        auto status = Vibrator::getCapabilities(capabilities);
        if (status.isOk() && mAmplitude.available()) {
            *capabilities |= IVibrator::CAP_AMPLITUDE_CONTROL;
        }
        return status;
    }

    ndk::ScopedAStatus on(int32_t timeoutMs,
                         const std::shared_ptr<IVibratorCallback>& callback) override {
        if (timeoutMs <= 0) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mAmplitude.reset()) return amplitudeError();
        return Vibrator::on(timeoutMs, callback);
    }

    ndk::ScopedAStatus off() override {
        std::lock_guard<std::mutex> lock(mMutex);
        return Vibrator::off();
    }

    ndk::ScopedAStatus setAmplitude(float amplitude) override {
        if (!mAmplitude.available()) return Vibrator::setAmplitude(amplitude);
        if (!AmplitudeControl::valid(amplitude)) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mAmplitude.set(amplitude)) return amplitudeError();
        return ndk::ScopedAStatus::ok();
    }

  private:
    static ndk::ScopedAStatus amplitudeError() {
        LOG(ERROR) << "Failed to update QPNP haptics amplitude";
        return ndk::ScopedAStatus::fromServiceSpecificError(1);
    }

    const AmplitudeControl mAmplitude;
    std::mutex mMutex;
};

}  // namespace

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    auto vibrator = ndk::SharedRefBase::make<AsusVibrator>();
    const std::string instance = std::string(IVibrator::descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(vibrator->asBinder().get(), instance.c_str()), STATUS_OK);
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;
}
