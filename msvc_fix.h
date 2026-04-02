#pragma once

#if defined(_WIN32)
    // Only process these lines if we are in a C++ file
    #ifdef __cplusplus
        #include <cstddef>
        // Use a more specific guard to avoid colliding with 
        // the Windows SDK's definition of byte
        #ifndef _BYTE_DEFINED
            #define _BYTE_DEFINED
            #define _RPCNDR_H_ 
            using byte = unsigned char;
        #endif
    #endif

    // These macros are safe for both C and C++
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif
