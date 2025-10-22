## yarp-openvr

YARP OpenVR project contains:
- ``yarp-openvr-trackers`` that streams OpenVR based tracker poses over a YARP Frame Transform Server.
- ``OpenVRCamera`` device that exposes the front facing camera of the VR headset as a YARP camera device.

The installation and usage procedure was tested with Windows 10 OS.

# Prerequisites

### Install Visual Studio
If you do not already have **Visual Studio** installed on your machine, you can install it using the **[Visual Studio Installer](https://visualstudio.microsoft.com/downloads/)** by enabling **Desktop and Mobile C++** development package.
This can be enabled by clicking on the `Modify` button in the installer corresponding to the desired installation.

### Install Steam and SteamVR
- Install [**Steam**](https://store.steampowered.com/about/)
- After installation, either create a Steam account or use an existing one to log-in and install [**SteamVR**](https://store.steampowered.com/app/250820/SteamVR/).

### Install robotology-superbuild
We rely on [**YARP**](https://www.yarp.it/latest/) for the use of **yarp-openvr**. If `YARP` is not already installed on your Windows machine, we recommend to install `YARP` using [**robotology-superbuild**](https://github.com/robotology/robotology-superbuild). 
`robotology-superbuild` can be installed either from source or using binaries with its dependencies installed using Package environment system called **Conda** following the [**installation procedure here**](https://github.com/robotology/robotology-superbuild/blob/master/doc/install-miniforge.md#windows).
Subsequently, `robotology-superbuild` can be installed from source or using binaries by following the [**installation procedure here**](https://github.com/robotology/robotology-superbuild/blob/master/doc/conda-forge.md#binary-installation).

While following the installation procedure using Conda-forge based dependencies, you create an environment within which the packages are installed. 
So everytime, we open a new terminal we need to activate this environment in order to access the installed dependencies and installed packages from the `robotology-superbuild`.
Note that, if you have followed a Source-based installation in [**installation procedure here**](https://github.com/robotology/robotology-superbuild/blob/master/doc/conda-forge.md#binary-installation), your environment needs to be activated using,
```
conda activate robsub
```
while, if you have followed the binary installation procedure in [**installation procedure here**](https://github.com/robotology/robotology-superbuild/blob/master/doc/conda-forge.md#binary-installation), you have to use,
```
conda activate robotologyenv
```
Note that this may not apply if you have installed `YARP` or `robotology-superbuild` following any other custom installation procedures.

### Other dependencies
Install `git` and `PkgConfig`.
```
conda install -c conda-forge  git pkg-config
```


# OpenVR and yarp-openvr

### Install OpenVR
We install [**OpenVR**](https://github.com/ami-iit/openvr) using a custom fork from  [**ami-iit**](https://github.com/ami-iit) organization because of some custom patches from the original repository.
We clone, build, and install  the repository from `fix_upstream_oct_2025` branch (which is the default branch of the fork).

```
git clone -b fix_upstream_oct_2025 https://github.com/ami-iit/openvr
cd openvr
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_INSTALL_PREFIX:PATH=<full-absolute-path-to-where-you-want-to-install-openvr> -DBUILD_SHARED=ON ..
cmake --build . --config Release
cmake --install . --config Release
```
⚠️ **WARNING: It is recommended to mention a custom location for CMAKE_INSTALL_PREFIX that does not require administrator privileges.**
⚠️ **WARNING: It is recommended to specify an absolute path to the install location in order to avoid strange, unresolved behaviors.**
⚠️ **WARNING: It must be noted that the configuration tag `Visual Studio 17 2022` applies for a Visual Studio 2022 Installation. Previous versions of installation should use corresponding tags, such as `Visual Studio 16 2019` for a 2019 installation**

**NOTE:** The CMake option `BUILD_SHARED` needs to be set to `ON` on Windows, while on Linux/macOS it can be left to be `OFF`.

**NOTE:** Since we are using the conda environment for installation, a common trick to avoid having to set new environment variables is just to install the CMake project directly in the environment, i.e. passing `-DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX` on Linux/macOS or `-DCMAKE_INSTALL_PREFIX=%CONDA_PREFIX%\Library` on Windows.

### Add OpenVR package to Package Configuration Path
To have OpenVR installation to be identified by CMake, we  need to add it to the package configuration path.
This can be done from the terminal after activating the `conda` environment by,
```
set PKG_CONFIG_PATH=%PKG_CONFIG_PATH%;<path_to_openvr-install-location>/share/pkgconfig
```
To verify if the package is properly added to the path, we may run
```
pkg-config --list-all
```
and check for `openvr` in the list of packages displayed.


### Install yarp-openvr
We install **yarp-openvr**

```
git clone https://github.com/ami-iit/yarp-openvr.git
cd yarp-openvr
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_INSTALL_PREFIX:PATH=<full-absolute-path-to-where-you-want-to-install-yarp-openvr-> ..
cmake --build . --config Release
cmake --install . --config Release
```


### Adjust firewall settings for YARP
In order to properly use YARP across the network, we have to provide firewall access to the YARP executables. To do this,
- Go to **Control Panel -> System and Security -> Windows Defender Firewall -> Allowed apps**
- You must be on `Firewall and network protection` page in the `Windows Security` section. Click on `Allow an app through firewall`
- Click on `Change settings` to have administrator privilege.
- Scroll down to find `yarp.exe` in the list and select `Details...->Network types...` and check all the network types (`Domain`, `Public`, and `Private`) and click `Ok`.
- Then we may have to repeat the procedure for `yarpdev`, `yarp-server`, and `yarp-openvr-trackers` using `Allow another app...` button.

## Checking the hardware setup
- Place the base stations in desired locations and power them on.
- Connect the Vive headset to the PC. (At this moment, connecting a headset is required to use SteamVR, in the future we need to find a workaround to avoid connecting the headset). It must be noted that the trackers pose are obtained relative to the Vive headset.
- Ensure that PC connecting the headset is not connected to an external display/monitor. Otherwise, the display is extended into the headset and is not identified by SteamVR.  With only the headset connected to the PC, ensure that the displays are extended (**Display settings -> Extend the displays**).
- Now, SteamVR should identify the base station and the headset.
- Once the headset is properly identified, it prompts you to **update Bluetooth driver settings** and **enable direct display mode**. Follow the procedures.
- The settings for the headset will be displayed. Go to **Settings->General** and disable **SteamVR home**.
- We then have to follow a calibration procedure indicated by SteamVR to set the seated position and floor position.
- In order to see what the user views through the headset, click on the menu button on SteamVR app and select **Display VR View**.
- In order to pair the Vive trackers, select **Menu->Devices->Pair Controllers** and select the Vive tracker to follow the pairing procedure.
- If the trackers are properly paired, you must see them through the headset.

## Running yarp-openvr-trackers
Now, in order to obtain the pose of the trackers through the YARP network, we will run the following commands.
- Run `yarpserver` in a new `mamba` activated terminal.
- Run `transformServer` in a new `mamba` activated terminal,
``` sh
yarpdev --device transformServer --ROS::enable_ros_publisher 0 --ROS::enable_ros_subscriber 0
```
- Run `yarp-openvr-trackers`
```
yarp-openvr-trackers
```

To verify the transforms are being published,
```
yarp read ... /transformServer/transforms:o
```
This must display the name of the headset and the marker name along with the poses in quaternion and position format.

### Note
You can pass the "Tracking universe" as a parameter to `yarp-openvr-trackers` by running the executable with the option `--vrOrigin`. For example:

```
yarp-openvr-trackers --vrOrigin standing
```

The value `standing` can be chagnged to `seated` or `raw`, and it's **case insensitive**. The default origin `seated` will be used when no parameter is passed or when passing an invalid value.

## Trackers roles 
From SteamVR, it is possible to assign a "role" to a tracker via the "Manage Trackers" menu. 

⚠️ **When using the ``HELD IN HAND`` role, the tracker orientation appears to be different with respect to all the other roles.**

| Tracker roles                                                                                                                          | Orientation                                                                                                                            |
|----------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------|
| ![Screenshot 2022-04-04 180311](https://user-images.githubusercontent.com/18591940/164017403-64e224e9-86ed-4e07-8e77-f18ed30af44f.png) | ![Screenshot 2022-04-04 180331](https://user-images.githubusercontent.com/18591940/164017600-4c051ec8-2345-4ad1-8fdd-eb32cf955409.png) |
| ![Screenshot 2022-04-04 180215](https://user-images.githubusercontent.com/18591940/164018203-34671fdb-ce8a-405e-8b0a-05ce1fe7c026.png) | ![Screenshot 2022-04-04 180402](https://user-images.githubusercontent.com/18591940/164018230-333bdb31-5d69-477d-a0b3-f058a665ccae.png) |

⚠️ The orientation around the y axis (in green) depends on the Room setup and according to the relative yaw angle between the headset and the trackers. In the pictures above, the headset was "looking" at the trackers.

In the images above, the trackers were positioned as follows: 
![image](https://user-images.githubusercontent.com/18591940/164016928-7d15681c-3967-4cf0-8d54-88e3080a1267.png)

## Running the OpenVRCamera device
The `OpenVRCamera` device exposes the front facing camera of the VR headset as a YARP camera device. Assuming to have ``yarpserver`` running, the device can be started with the following command:
```
yarpdev --device OpenVRCamera
```

It is automatically wrapped by a ``frameGrabber_nws_yarp``, so you can check the ``frameGrabber_nws_yarp`` parameters to change the 
port name or the streaming frequency, for example:
```
yarpdev --device OpenVRCamera --period 0.033 --name /openvr/camera
```
