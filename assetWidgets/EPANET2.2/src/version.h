/*
 *  version.h - EPANET version header file
 *
 *  Created on: Sep 2, 2023
 *  
 *  Author:     see CONTRIBUTORS
 *
 *  Note: 
 *    The cmake build process automatically generates this file. Do not edit.
 */


#ifndef VERSION_H_
#define VERSION_H_



#define PROJECT             "EPANET"
#define ORGANIZATION        "US EPA ORD"

#define VERSION             "2.2.0"
#define VERSION_MAJOR       2
#define VERSION_MINOR       2
#define VERSION_PATCH       0
#define GIT_HASH            "f5c766fdb6735593b57c05a92f7098b6d7ecb943"

#define PLATFORM            "Darwin"
#define COMPILER            "AppleClang"
#define COMPILER_VERSION    "15.0.0.15000040"
#define BUILD_ID            "2023-10-23T22:43:59Z"


static inline int get_version_legacy() { \
    return VERSION_MAJOR * 10000 + VERSION_MINOR * 1000 + VERSION_PATCH; \
}



#endif /* VERSION_H_ */
