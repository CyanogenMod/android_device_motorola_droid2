/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "Droid2CameraWrapper"

#include <cmath>
#include <dlfcn.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <camera/Camera.h>
#include "Droid2CameraWrapper.h"

namespace android {

extern "C" int HAL_getNumberOfCameras()
{
    return 1;
}

extern "C" void HAL_getCameraInfo(int cameraId, struct CameraInfo* cameraInfo)
{
    cameraInfo->facing = CAMERA_FACING_BACK;
    cameraInfo->orientation = 90;
}

extern "C" sp<CameraHardwareInterface> HAL_openCameraHardware(int cameraId)
{
    LOGV("openCameraHardware: call createInstance");
    return Droid2CameraWrapper::createInstance(cameraId);
}

wp<CameraHardwareInterface> Droid2CameraWrapper::singleton;

static sp<CameraHardwareInterface>
openMotoInterface(const char *libName, const char *funcName)
{
    sp<CameraHardwareInterface> interface;
    void *libHandle = ::dlopen(libName, RTLD_NOW);

    if (libHandle != NULL) {
        typedef sp<CameraHardwareInterface> (*OpenCamFunc)();
        OpenCamFunc func = (OpenCamFunc) ::dlsym(libHandle, funcName);
        if (func != NULL) {
            interface = func();
        } else {
            LOGE("Could not find library entry point!");
        }
    } else {
        LOGE("dlopen() error: %s\n", dlerror());
    }

    return interface;
}

sp<CameraHardwareInterface> Droid2CameraWrapper::createInstance(int cameraId)
{
    LOGV("%s :", __func__);
    if (singleton != NULL) {
        sp<CameraHardwareInterface> hardware = singleton.promote();
        if (hardware != NULL) {
            return hardware;
        }
    }

    sp<CameraHardwareInterface> motoInterface;
    sp<CameraHardwareInterface> hardware;

    /* entry point of driver is android::CameraHal::createInstance() */
    motoInterface = openMotoInterface("libmotocamera.so", "_ZN7android9CameraHal14createInstanceEv");

    if (motoInterface != NULL) {
        hardware = new Droid2CameraWrapper(motoInterface);
        singleton = hardware;
    } else {
        LOGE("Could not open hardware interface");
    }

    return hardware;
}

Droid2CameraWrapper::Droid2CameraWrapper(sp<CameraHardwareInterface>& motoInterface) :
    mMotoInterface(motoInterface)
{
    
}

Droid2CameraWrapper::~Droid2CameraWrapper()
{
}

sp<IMemoryHeap>
Droid2CameraWrapper::getPreviewHeap() const
{
    return mMotoInterface->getPreviewHeap();
}

sp<IMemoryHeap>
Droid2CameraWrapper::getRawHeap() const
{
    return mMotoInterface->getRawHeap();
}

void
Droid2CameraWrapper::setCallbacks(notify_callback notify_cb,
                                  data_callback data_cb,
                                  data_callback_timestamp data_cb_timestamp,
                                  void* user)
{
    mMotoInterface->setCallbacks(notify_cb, data_cb, data_cb_timestamp, user);
}

void
Droid2CameraWrapper::enableMsgType(int32_t msgType)
{
    mMotoInterface->enableMsgType(msgType);
}

void
Droid2CameraWrapper::disableMsgType(int32_t msgType)
{
    mMotoInterface->disableMsgType(msgType);
}

bool
Droid2CameraWrapper::msgTypeEnabled(int32_t msgType)
{
    return mMotoInterface->msgTypeEnabled(msgType);
}

status_t
Droid2CameraWrapper::startPreview()
{
    return mMotoInterface->startPreview();
}

bool
Droid2CameraWrapper::useOverlay()
{
    return mMotoInterface->useOverlay();
}

status_t
Droid2CameraWrapper::setOverlay(const sp<Overlay> &overlay)
{
    return mMotoInterface->setOverlay(overlay);
}

void
Droid2CameraWrapper::stopPreview()
{
    mMotoInterface->stopPreview();
}

bool
Droid2CameraWrapper::previewEnabled()
{
    return mMotoInterface->previewEnabled();
}

status_t
Droid2CameraWrapper::startRecording()
{
    return mMotoInterface->startRecording();
}

void
Droid2CameraWrapper::stopRecording()
{
    mMotoInterface->stopRecording();
}

bool
Droid2CameraWrapper::recordingEnabled()
{
    return mMotoInterface->recordingEnabled();
}

void
Droid2CameraWrapper::releaseRecordingFrame(const sp<IMemory>& mem)
{
    return mMotoInterface->releaseRecordingFrame(mem);
}

status_t
Droid2CameraWrapper::autoFocus()
{
    return mMotoInterface->autoFocus();
}

status_t
Droid2CameraWrapper::cancelAutoFocus()
{
    return mMotoInterface->cancelAutoFocus();
}

status_t
Droid2CameraWrapper::takePicture()
{
    return mMotoInterface->takePicture();
}

status_t
Droid2CameraWrapper::cancelPicture()
{
    return mMotoInterface->cancelPicture();
}

status_t
Droid2CameraWrapper::setParameters(const CameraParameters& params)
{
    return mMotoInterface->setParameters(params);
}

CameraParameters
Droid2CameraWrapper::getParameters() const
{
    CameraParameters params = mMotoInterface->getParameters();
    /* smooth zoom is broken */
    params.remove(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED);
    return params;
}

status_t
Droid2CameraWrapper::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    return mMotoInterface->sendCommand(cmd, arg1, arg2);
}

void
Droid2CameraWrapper::release()
{
    // I'm not entirely comfortable not passing this call along, but doing so
    // causes the whole thing to just crash and burn.
    //mMotoInterface->release();
}

status_t
Droid2CameraWrapper::dump(int fd, const Vector<String16>& args) const
{
    return mMotoInterface->dump(fd, args);
}

}; //namespace android
