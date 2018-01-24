# Orbbec Astra Acquisition Tool

The Orbbec Astra acquisition tool allows the user to save PCD files using the camera's point stream. The point sream consists of the world coordinates X, Y and Z (where Z is the depth/distance of object from camera). 

The tool makes use of the Astra SDK:
https://github.com/orbbec/astra

The SDK is included in the project as a git submodule. Read the installation guide to learn more about using the SDK.

# Setup

## Point Cloud Library (PCL):

The tool requires the PCL library to be installed. To download the PCL library, clone/download from the following repo:
https://github.com/PointCloudLibrary/pcl

To see how to install the library and its dependencies, see the following tutorial:
http://pointclouds.org/documentation/tutorials/compiling_pcl_macosx.php

_Note: VTK library could not be installed correctly using the instructions in the above tutorial. Please refer to the following document for installing the VTK library:
https://alitheon.atlassian.net/wiki/spaces/IE/pages/136806401/Point+Cloud+Data+Analysis+Pipeline
_

## Astra SDK:

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


