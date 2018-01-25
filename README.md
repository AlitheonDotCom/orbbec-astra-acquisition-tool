# Orbbec Astra Acquisition Tool

The Orbbec Astra acquisition tool allows the user to save PCD files using the camera's point stream. The point sream consists of the world coordinates X, Y and Z (where Z is the depth/distance of object from camera). 

The tool makes use of the Astra SDK:
https://github.com/orbbec/astra

The SDK is included in the project as a git submodule. Read the installation guide to learn more about using the SDK.

# Setup

## Dependencies

### Point Cloud Library (PCL):

The tool requires the PCL library to be installed. To download the PCL library, clone/download from the following repo:
https://github.com/PointCloudLibrary/pcl

To see how to install the library and its dependencies, see the following tutorial:
http://pointclouds.org/documentation/tutorials/compiling_pcl_macosx.php

*Note: VTK library could not be installed correctly using the instructions in the above tutorial. Please refer to the following document for installing the VTK library:*
https://alitheon.atlassian.net/wiki/spaces/IE/pages/136806401/Point+Cloud+Data+Analysis+Pipeline

### Astra SDK:

The Orbbec Astra SDK is included as a submodule in this project. After you have cloned the repo, simply run the following two commands to fetch the submodule:

```
git submodule init
git submodule update
```

To pull the latest version of the Astra SDK, you can update the submodule as follows:

```
git submodule update --remote
```

A full explanation of the submodule commands is out of scope of this document. Please refer to the following link to learn more:
https://git-scm.com/book/en/v2/Git-Tools-Submodules

## Installation
1. Clone/Download the repo to your project folder.
2. Using the submodule commands listed above, get the Astra SDK.
3. The SDK will be fetched inside the 'astra' subdirectory.
4. Copy the folder titled *AcquisitionTool* to  *astra/samples/sfml*. The *AcquisitionTool* folder contains our c++ code and the CMakeLists.txt file.
5. Now go to * astra/samples/scripts* and copy the bash script titled *build_samples.sh* to *astra/samples* parent folder. Then, run the bash script `./build_samples.sh`.
6. *Note:* Alternatively, you can also create a *build* folder and run the `cmake ..` command from inside the build folder, followed by `make && make install`.
7. The bash script will create executables in *astra/samples/build/bin*.
8. To run the tool:
```
cd astra/samples/build/bin
./AcquisitionTool
```

# Usage

1. Make sure the camera is plugged in before running the tool.
2. If everything goes well, you will be displayed with a window showing the depth stream from Astra.
3. Astra can only display one data stream at a time. Any attempts to stream the *ColorStream* data were futile. See the following link explaining the issue:
https://3dclub.orbbec3d.com/t/color-stream-issue-on-astra-pro-under-mac-osx/795
4. The bottom of the window displays the X, Y and Z values for the current cursor position. As you move around the mouse, these values wil be updated.
5. Press 'H/h' for the help menu.
6. To acquie the PCD file, simply press A/a. The depth stream will pause, asking the user to enter the name of the pcd file at the console.
7. After the user has entered the file name, a PCD file will be saved (in the same location as the executable). 
8. The depth stream will resume after that.
9. To close the tool, simply press *Control+C* or hit *Esc*.

# Output

The tool provides us with PCD files with ASCII data. The files are saved in the same location as the execution tool. 

To learn more about PCD files, see the following link:
http://pointclouds.org/about/

