/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#ifndef OPENVR_TRACKERS_DRIVER_H
#define OPENVR_TRACKERS_DRIVER_H

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace openvr {
    struct Pose;
    struct TrackedDevice;
    class DevicesManager;

    enum class TrackingUniverseOrigin
    {
        Seated = 0,
        Standing = 1,
        Raw = 2,
    };

    enum class TrackedDeviceType
    {
        Invalid = 0,
        HMD = 1,
        Controller = 2,
        GenericTracker = 3,
        TrackingReference = 4,
        DisplayRedirect = 5,
    };
} // namespace openvr

struct openvr::Pose
{
    std::array<double, 3> position;
    std::array<double, 9> rotationRowMajor;
};

struct openvr::TrackedDevice
{
    size_t index;
    std::string serialNumber;
    TrackedDeviceType type = TrackedDeviceType::Invalid;
};

class openvr::DevicesManager
{
public:
    DevicesManager();
    ~DevicesManager();

    bool initialize(const TrackingUniverseOrigin& vrOrigin = TrackingUniverseOrigin::Seated);
    bool initialized() const;

    bool addDevice(const size_t index);
    bool removeDevice(const std::string& serialNumber);
    std::vector<std::string> managedDevices() const;

    TrackedDeviceType type(const std::string& serialNumber) const;
    bool computePoses();
    std::optional<Pose> pose(const std::string& serialNumber) const;

    bool resetSeatedPosition();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    void clearEvents();
    void processEvents();
};

#endif // OPENVR_TRACKERS_DRIVER_H
