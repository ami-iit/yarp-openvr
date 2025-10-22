/*
 * Copyright (C) 2025 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#ifndef YARP_OPENVR_CAMERA_H
#define YARP_OPENVR_CAMERA_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IFrameGrabberImage.h>

#include <memory>

namespace yarp::dev {
    class OpenVRCamera;
}

class yarp::dev::OpenVRCamera : public yarp::dev::DeviceDriver,
                                public yarp::dev::IFrameGrabberImage
{
public:

    OpenVRCamera();
    ~OpenVRCamera() override;

    // DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    virtual bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb> &image) override;
    int height() const override;
    int width() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // YARP_OPENVR_CAMERA_H
