/*
 * Copyright (C) 2025 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#include "OpenVRCamera.h"
#include <yarp/os/LogStream.h>

#include < openvr.h>

// Adapted from
// https://github.com/ValveSoftware/openvr/blob/91825305130f446f82054c1ec3d416321ace0072/samples/tracked_camera_openvr_sample/tracked_camera_openvr_sample.cpp

YARP_DECLARE_LOG_COMPONENT(CAMERA)
YARP_LOG_COMPONENT(CAMERA, "yarp.device.OpenVRCamera")

struct yarp::dev::OpenVRCamera::Impl
{
    vr::IVRSystem* pVRSystem;
    vr::IVRTrackedCamera* pVRTrackedCamera;
    vr::TrackedCameraHandle_t hTrackedCamera;

    std::string HMDSerialNumberString;

    uint32_t nCameraFrameWidth{0};
    uint32_t nCameraFrameHeight{0};
    uint32_t nCameraFrameBufferSize;
    uint8_t* pCameraFrameBuffer;
    uint32_t nLastFrameSequence;
};

yarp::dev::OpenVRCamera::OpenVRCamera()
    : pImpl{std::make_unique<Impl>()}
{}

yarp::dev::OpenVRCamera::~OpenVRCamera()
{
    close();
}

bool yarp::dev::OpenVRCamera::open(yarp::os::Searchable& config)
{
    // Loading the SteamVR Runtime
    yCInfo(CAMERA) << "Starting OpenVR...";
    vr::EVRInitError eError = vr::VRInitError_None;
    pImpl->pVRSystem = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None) {
        pImpl->pVRSystem = nullptr;
        yCError(CAMERA) << "Unable to init VR runtime:"
                        << vr::VR_GetVRInitErrorAsSymbol(eError);
        return false;
    }
    else {
        char systemName[1024];
        char serialNumber[1024];
        pImpl->pVRSystem->GetStringTrackedDeviceProperty(
            vr::k_unTrackedDeviceIndex_Hmd,
            vr::Prop_TrackingSystemName_String,
            systemName,
            sizeof(systemName));
        pImpl->pVRSystem->GetStringTrackedDeviceProperty(
            vr::k_unTrackedDeviceIndex_Hmd,
            vr::Prop_SerialNumber_String,
            serialNumber,
            sizeof(serialNumber));

        pImpl->HMDSerialNumberString = serialNumber;

        yCInfo(CAMERA) << "VR HMD:" << systemName << serialNumber;
    }

    pImpl->pVRTrackedCamera = vr::VRTrackedCamera();
    if (!pImpl->pVRTrackedCamera) {
        yCError(CAMERA) << "Unable to get Tracked Camera interface.";
        return false;
    }

    bool bHasCamera = false;
    vr::EVRTrackedCameraError nCameraError = pImpl->pVRTrackedCamera->HasCamera(
        vr::k_unTrackedDeviceIndex_Hmd, &bHasCamera);

    if (nCameraError != vr::VRTrackedCameraError_None || !bHasCamera) {
        yCError(CAMERA) << "No Tracked Camera Available:"
                        << pImpl->pVRTrackedCamera->GetCameraErrorNameFromEnum(
                               nCameraError);
        return false;
    }

    // Accessing the FW description is just a further check to ensure camera
    // communication is valid as expected.
    vr::ETrackedPropertyError propertyError;
    char buffer[128];
    pImpl->pVRSystem->GetStringTrackedDeviceProperty(
        vr::k_unTrackedDeviceIndex_Hmd,
        vr::Prop_CameraFirmwareDescription_String,
        buffer,
        sizeof(buffer),
        &propertyError);
    if (propertyError != vr::TrackedProp_Success) {
        yCError(CAMERA) << "Unable to get Tracked Camera Firmware Description:"
                        << pImpl->pVRSystem->GetPropErrorNameFromEnum(
                               propertyError);

        return false;
    }

    yCInfo(CAMERA) << "Camera FW Description:" << buffer;

    yCInfo(CAMERA) << "Starting video acquisition...";

    // Allocate for camera frame buffer requirements
    uint32_t nCameraFrameBufferSize = 0;
    auto error = pImpl->pVRTrackedCamera->GetCameraFrameSize(
        vr::k_unTrackedDeviceIndex_Hmd,
        vr::VRTrackedCameraFrameType_Undistorted,
        &pImpl->nCameraFrameWidth,
        &pImpl->nCameraFrameHeight,
        &nCameraFrameBufferSize);
    if (error != vr::VRTrackedCameraError_None) {
        yCError(CAMERA) << "GetCameraFrameBounds() Failed!";
        return false;
    }

    if (nCameraFrameBufferSize
        && nCameraFrameBufferSize != pImpl->nCameraFrameBufferSize) {
        delete[] pImpl->pCameraFrameBuffer;
        pImpl->nCameraFrameBufferSize = nCameraFrameBufferSize;
        pImpl->pCameraFrameBuffer = new uint8_t[pImpl->nCameraFrameBufferSize];
        memset(pImpl->pCameraFrameBuffer, 0, pImpl->nCameraFrameBufferSize);
    }

    pImpl->nLastFrameSequence = 0;

    pImpl->pVRTrackedCamera->AcquireVideoStreamingService(
        vr::k_unTrackedDeviceIndex_Hmd, &pImpl->hTrackedCamera);
    if (pImpl->hTrackedCamera == INVALID_TRACKED_CAMERA_HANDLE) {
        yCError(CAMERA) << "AcquireVideoStreamingService() Failed!";
        return false;
    }

    yCInfo(CAMERA) << "OpenVRCamera device ready.";

    return true;
}

bool yarp::dev::OpenVRCamera::close()
{
    if (pImpl->hTrackedCamera != INVALID_TRACKED_CAMERA_HANDLE) {
        pImpl->pVRTrackedCamera->ReleaseVideoStreamingService(
            pImpl->hTrackedCamera);
        pImpl->hTrackedCamera = INVALID_TRACKED_CAMERA_HANDLE;
    }

    if (pImpl->pVRSystem) {
        vr::VR_Shutdown();
        pImpl->pVRSystem = nullptr;
    }
    return true;
}

bool yarp::dev::OpenVRCamera::getImage(
    yarp::sig::ImageOf<yarp::sig::PixelRgb>& image)
{
    if (!pImpl->pVRTrackedCamera || !pImpl->hTrackedCamera) {
        yCError(CAMERA) << "getImage() called before camera has been opened.";
        return false;
    }

    // get the frame header only
    vr::CameraVideoStreamFrameHeader_t frameHeader;
    vr::EVRTrackedCameraError nCameraError =
        pImpl->pVRTrackedCamera->GetVideoStreamFrameBuffer(
            pImpl->hTrackedCamera,
            vr::VRTrackedCameraFrameType_Undistorted,
            nullptr,
            0,
            &frameHeader,
            sizeof(frameHeader));

    if (nCameraError != vr::VRTrackedCameraError_None) {
        yCError(CAMERA)
            << "GetVideoStreamFrameBuffer() Failed to get frame header. Error:"
            << pImpl->pVRTrackedCamera->GetCameraErrorNameFromEnum(
                   nCameraError);
        return false;
    }

    if (frameHeader.nFrameSequence == pImpl->nLastFrameSequence) {
        yCWarning(CAMERA) << "No new frame available.";
        return false;
    }

    // Frame has changed, do the more expensive frame buffer copy
    nCameraError = pImpl->pVRTrackedCamera->GetVideoStreamFrameBuffer(
        pImpl->hTrackedCamera,
        vr::VRTrackedCameraFrameType_Undistorted,
        pImpl->pCameraFrameBuffer,
        pImpl->nCameraFrameBufferSize,
        &frameHeader,
        sizeof(frameHeader));

    if (nCameraError != vr::VRTrackedCameraError_None) {
        yCError(CAMERA)
            << "GetVideoStreamFrameBuffer() Failed to get frame buffer. Error:"
            << pImpl->pVRTrackedCamera->GetCameraErrorNameFromEnum(
                   nCameraError);
        return false;
    }

    pImpl->nLastFrameSequence = frameHeader.nFrameSequence;

    image.resize(pImpl->nCameraFrameWidth, pImpl->nCameraFrameHeight);
    const uint8_t* pFrameImage = pImpl->pCameraFrameBuffer;
    for (uint32_t y = 0; y < pImpl->nCameraFrameHeight; y++) {
        for (uint32_t x = 0; x < pImpl->nCameraFrameWidth; x++) {
            image.pixel(x, y).r = pFrameImage[3] > 0 ? pFrameImage[0] : 0;
            image.pixel(x, y).g = pFrameImage[3] > 0 ? pFrameImage[1] : 0;
            image.pixel(x, y).b = pFrameImage[3] > 0 ? pFrameImage[2] : 0;
            pFrameImage += 4; // advance by 4 bytes (RGBA)
        }
    }

    return true;
}

int yarp::dev::OpenVRCamera::height() const
{
    return pImpl->nCameraFrameHeight;
}

int yarp::dev::OpenVRCamera::width() const
{
    return pImpl->nCameraFrameWidth;
}
