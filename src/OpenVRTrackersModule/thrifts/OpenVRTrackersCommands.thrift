/*
 * Copyright (C) 2022 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-2-Clause license. See the accompanying LICENSE file for details.
 */

service OpenVRTrackersCommands
{
    /**
     * Resets the VR seated position, such that the headset appears in the origin looking forward.
     * @return true if the reset was successful.
     */
    bool resetSeatedPosition();
}
