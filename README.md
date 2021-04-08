# R2D

[![DOI](https://zenodo.org/badge/329111097.svg)](https://zenodo.org/badge/latestdoi/329111097)

This open-source research application provides an application
researchers can use to predict regional damage and loss estimates
for a region subjected to a natural hazard.

The computations are performed in a workflow application. That is, the
numerical simulations are actually performed by a number of different
applications. The rWHALE backend software runs these different
applications for the user, taking the outputs from some programs and
providing them as inputs to others. The design of the R2D
application is such that researchers are able to modify the backend
applications to utilize their own application in the workflow
computations. This will ensure researchers are not limited to using
the default applications we provide and will be enthused to provide
their own applications for others to use.

This is Version 1.1.0 of the tool and as such is limited in
scope. Researchers are encouraged to comment on what additional
features and applications they would like to see in this
application. If you want it, chances are many of your colleagues also
would benefit from it.

Please visit the [R2D Research Tool webpage](http://simcenter-messageboard.designsafe-ci.org/smf/index.php?board=8.0)
for more resources related to this tool. Additionally, this page
provides more information on the NHERI SimCenter, including other SimCenter
applications, FAQ, and how to collaborate.

### Documentation

Tool documentation can be found: https://nheri-simcenter.github.io/R2D-Documentation/


### How to Build

##### 1. Download this repo.

##### 2. Download the SimCenterCommon repo: https://github.com/NHERI-SimCenter/SimCenterCommon

Place the SimCenterCommon Repo in the same directory that you placed the R2DTool repo.

##### 3. Install Qt: https://www.qt.io/

Qt is free for open source developers. Download it and start the Qt Creator application. From Qt Creator open the R2D.pro file located in the directory the R2DTool repo was downloaded into and select build to build it. For detailed instructions on using Qt, browse their website.

#### 4. To run locally you will need to install and build the SimCenterBackendApplications repo: https://github.com/NHERI-SimCenter/SimCenterBackendApplications

SimCenterBackendApplications contains a number of applications written in C++, C and Python. Follow the build instructions on the SimCenterBackendApplications githib page to build them. Once built inside the R2D applicationss preferences set the applications directory entry to point to the applications folder that the build process creates.

### Acknowledgement

This material is based upon work supported by the National Science Foundation under Grant No. 1612843.

### Contact

NHERI-SimCenter nheri-simcenter@berkeley.edu

