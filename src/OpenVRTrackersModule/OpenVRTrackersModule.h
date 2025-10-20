/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#ifndef OPENVR_TRACKERS_MODULE_H
#define OPENVR_TRACKERS_MODULE_H

#include "OpenVRTrackersDriver.h"
#include <thrifts/OpenVRTrackersCommands.h>

#include <yarp/dev/IFrameTransform.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/RFModule.h>
#include <yarp/sig/Matrix.h>
#include <yarp/os/Port.h>

#include <string>
#include <mutex>
#include <cctype>
#include <algorithm>

class OpenVRTrackersModule final : public yarp::os::RFModule,
                                   public OpenVRTrackersCommands
{
public:
    bool configure(yarp::os::ResourceFinder& rf) override;
    double getPeriod() override;
    bool updateModule() override;
    bool close() override;
    bool resetSeatedPosition() override;

private:
    double m_period;
    std::string m_baseFrame;

    yarp::sig::Matrix m_sendBuffer;
    yarp::dev::IFrameTransform* m_tf;

    yarp::dev::PolyDriver m_driver;

    openvr::DevicesManager m_manager;

    yarp::os::Port m_rpcPort;

    mutable std::mutex m_mutex;
};

#endif // OPENVR_TRACKERS_MODULE_H
