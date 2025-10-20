/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#include "OpenVRTrackersModule.h"
#include <yarp/os/LogStream.h>

namespace openvr_trackers_module {
    constexpr double DefaultPeriod = 0.010;
    const std::string DefaultTfLocal = "/tf";
    const std::string DefaultTfRemote = "/transformServer";
    const std::string DefaultTfBaseFrameName = "openVR_origin";
    const std::string ModuleName = "OpenVRTrackersModule";
    const std::string LogPrefix = ModuleName + ":";
    const std::string DefaultVrOrigin = "Seated";
} // namespace openvr_trackers_module

bool OpenVRTrackersModule::configure(yarp::os::ResourceFinder& rf)
{
    const auto lock = std::unique_lock(m_mutex);

    // ===========================
    // Check configuration options
    // ===========================

    // Try to find the "name" entry
    std::string name;
    if (!(rf.check("name")
          && rf.find("name").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix
                << "Using default name:"
                << openvr_trackers_module::ModuleName;
        name = openvr_trackers_module::ModuleName;
    }
    else {
        name = rf.find("name").asString();
    }
    this->setName(name.c_str());

    // Try to find the "period" entry
    if (!(rf.check("period") && rf.find("period").isFloat64())) {
        yInfo() << openvr_trackers_module::LogPrefix << "Using default period:"
                << openvr_trackers_module::DefaultPeriod << "s";
        m_period = openvr_trackers_module::DefaultPeriod;
    }
    else {
        m_period = rf.find("period").asFloat64();
    }

    // Try to find the "tfBaseFrameName" entry
    if (!(rf.check("tfBaseFrameName")
          && rf.find("tfBaseFrameName").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix
                << "Using default tfBaseFrameName:"
                << openvr_trackers_module::DefaultTfBaseFrameName;
        m_baseFrame = openvr_trackers_module::DefaultTfBaseFrameName;
    }
    else {
        m_baseFrame = rf.find("tfBaseFrameName").asString();
    }

    // Try to find the "tfLocal" entry
    std::string tfLocal;
    if (!(rf.check("tfLocal") && rf.find("tfLocal").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix << "Using default tfLocal:"
                << "/" + getName() + openvr_trackers_module::DefaultTfLocal;
        tfLocal = "/" + getName() + openvr_trackers_module::DefaultTfLocal;
    }
    else {
        tfLocal = rf.find("tfLocal").asString();
    }

    // Try to find the "tfRemote" entry
    std::string tfRemote;
    if (!(rf.check("tfRemote") && rf.find("tfRemote").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix
                << "Using default tfRemote:"
                << openvr_trackers_module::DefaultTfRemote;
        tfRemote = openvr_trackers_module::DefaultTfRemote;
    }
    else {
        tfRemote = rf.find("tfRemote").asString();
    }

    // Try to find the "vrOrigin" entry
    std::string vrOriginString;
    openvr::TrackingUniverseOrigin vrOrigin = openvr::TrackingUniverseOrigin::Seated;
    if (!(rf.check("vrOrigin") && rf.find("vrOrigin").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix
                << "Using default vrOrigin:"
                << openvr_trackers_module::DefaultVrOrigin;
        vrOriginString = openvr_trackers_module::DefaultVrOrigin;
    }
    else {
        vrOriginString = rf.find("vrOrigin").asString();
        std::transform(vrOriginString.begin(), vrOriginString.end(), vrOriginString.begin(), [](unsigned char c){ return std::tolower(c); });
        if(vrOriginString == "seated") {
            vrOrigin = openvr::TrackingUniverseOrigin::Seated;
        }
        else if(vrOriginString == "standing") {
            vrOrigin = openvr::TrackingUniverseOrigin::Standing;
        }
        else if(vrOriginString == "raw") {
            vrOrigin = openvr::TrackingUniverseOrigin::Raw;
        }
        else {
            vrOrigin = openvr::TrackingUniverseOrigin::Seated;
            yWarning() << openvr_trackers_module::LogPrefix
                << "Invalid inserted vrOrigin value: " << vrOriginString << ", using the default value: seated" ;
        }
    }

    // Create configuration of the "transformClient" device
    yarp::os::Property tfClientCfg;
    tfClientCfg.put("device", "transformClient");
    tfClientCfg.put("local", tfLocal);
    tfClientCfg.put("remote", tfRemote);

    // Open the transformClient device
    if (!m_driver.open(tfClientCfg)) {
        yError() << openvr_trackers_module::LogPrefix
                 << "Unable to open polydriver with the following options:"
                 << tfClientCfg.toString();
        return false;
    }

    // Extract and store the IFrameTransform interface
    if (!(m_driver.view(m_tf) && m_tf)) {
        yError() << openvr_trackers_module::LogPrefix
                 << "Unable to view IFrameTransform interface.";
        return false;
    }

    // Initialize the transform buffer
    m_sendBuffer.resize(4, 4);
    m_sendBuffer.eye();

    // Initialize the OpenVR driver
    if (!m_manager.initialize(vrOrigin)) {
        yError() << openvr_trackers_module::LogPrefix
                 << "Failed to initialize the OpenVR devices manager.";
        return false;
    }

    if (!m_manager.resetSeatedPosition())
    {
        yError() << openvr_trackers_module::LogPrefix << "Failed to reset seated position.";
        return false;
    }

    // Bind the RPC service to the module's object
    this->yarp().attachAsServer(this->m_rpcPort);
    
    if(!m_rpcPort.open("/" + openvr_trackers_module::ModuleName +  + "/rpc"))
    {
        yError() << openvr_trackers_module::LogPrefix << "Could not open"
                 << "/" + openvr_trackers_module::ModuleName +  + "/rpc" << " RPC port.";
        return false;
    }

    return true;
}

double OpenVRTrackersModule::getPeriod()
{
    const auto lock = std::unique_lock(m_mutex);

    return m_period;
}

bool OpenVRTrackersModule::updateModule()
{
    const auto lock = std::unique_lock(m_mutex);

    // compute the poses
    m_manager.computePoses();
    // Iterate over all the managed devices of the driver
    for (const auto& sn : m_manager.managedDevices()) {

        if (const auto& poseOpt = m_manager.pose(sn); poseOpt.has_value()) {

            // Extract the pose of the device
            const openvr::Pose& pose = poseOpt.value();

            // Compute the prefix of the transform based on the device type.
            // The final name will be "{tf_name_prefix}/{serial_number}".
            const std::string tfNamePrefix = [&]() {
                std::string prefix;

                switch (m_manager.type(sn)) {
                    case openvr::TrackedDeviceType::HMD:
                        prefix = "/hmd/";
                        break;
                    case openvr::TrackedDeviceType::Controller:
                        prefix = "/controllers/";
                        break;
                    case openvr::TrackedDeviceType::GenericTracker:
                        prefix = "/trackers/";
                        break;
                    default:
                        break;
                }
                return prefix;
            }();

            // Reset the transform
            m_sendBuffer.eye();

            // Fill the rotation of the transform using the row-major
            // serialization used by the driver
            m_sendBuffer[0][0] = pose.rotationRowMajor[0];
            m_sendBuffer[0][1] = pose.rotationRowMajor[1];
            m_sendBuffer[0][2] = pose.rotationRowMajor[2];
            m_sendBuffer[1][0] = pose.rotationRowMajor[3];
            m_sendBuffer[1][1] = pose.rotationRowMajor[4];
            m_sendBuffer[1][2] = pose.rotationRowMajor[5];
            m_sendBuffer[2][0] = pose.rotationRowMajor[6];
            m_sendBuffer[2][1] = pose.rotationRowMajor[7];
            m_sendBuffer[2][2] = pose.rotationRowMajor[8];

            // Fill the position of the transform
            m_sendBuffer[0][3] = pose.position[0];
            m_sendBuffer[1][3] = pose.position[1];
            m_sendBuffer[2][3] = pose.position[2];

            // Publish the transform
            m_tf->setTransform(tfNamePrefix + sn, m_baseFrame, m_sendBuffer);

        }
    }

    return true;
}

bool OpenVRTrackersModule::close()
{
    const auto lock = std::unique_lock(m_mutex);

    m_driver.close();
    m_rpcPort.close();
    return true;
}

bool OpenVRTrackersModule::resetSeatedPosition()
{
    const auto lock = std::unique_lock(m_mutex);

    if (!m_manager.resetSeatedPosition())
    {
        yError() << openvr_trackers_module::LogPrefix << "Failed to reset seated position.";
        return false;
    }

    return true;
}
