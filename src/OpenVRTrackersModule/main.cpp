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
#include <yarp/os/Network.h>
#include <yarp/os/ResourceFinder.h>

int main(int argc, char** argv)
{
    yarp::os::Network yarp; // to initialize the network

    if (!yarp.checkNetwork()) {
        yError() << "No YARP network found.";
        return EXIT_FAILURE;
    }

    yarp::os::ResourceFinder& rf =
        yarp::os::ResourceFinder::getResourceFinderSingleton();
    rf.configure(argc, argv);

    OpenVRTrackersModule module;

    return module.runModule(rf);
}
