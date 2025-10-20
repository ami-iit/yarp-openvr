/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#include "OpenVRTrackersDriver.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

int main(int /*argc*/, char** /*argv*/)
{
    // Open
    std::cout << "[main] Opening the manager..." << std::endl;
    auto manager = openvr::DevicesManager();
    std::cout << "[main] ... done" << std::endl;

    // Initialize
    std::cout << "[main] Initializing the manager..." << std::endl;
    if (!manager.initialize()) {
        std::cerr << "Failed to initialize the manager" << std::endl;
        return 1;
    }
    std::cerr << "[main] ... done" << std::endl;

    // List devices
    for (const auto& sn : manager.managedDevices()) {
        std::cout << "Found: " << sn << std::endl;
    }

    // Print devices positions
    while (true) {

        std::this_thread::sleep_for(std::chrono::seconds(1));

        manager.computePoses();
        for (const auto& sn : manager.managedDevices()) {

            // Get the pose. If not available, the returned optional value
            // is empty
            const auto& poseOpt = manager.pose(sn);

            if (!poseOpt.has_value()) {
                std::cerr << "Failed to get position" << std::endl;
                continue;
            }

            // Get the value
            const auto& pose = poseOpt.value();

            // Print the pose
            std::cerr << sn << ": " << pose.position[0] << ", "
                      << pose.position[1] << ", " << pose.position[2]
                      << std::endl;
        }
    }

    return 0;
}
