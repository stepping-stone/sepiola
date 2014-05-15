//Create not existing value if needed (which is the case in MinGW)
#include "winuser.h"
 
#ifndef CREATEPROCESS_MANIFEST_RESOURCE_ID
    #define CREATEPROCESS_MANIFEST_RESOURCE_ID 1
#endif
#ifndef RT_MANIFEST
    #define RT_MANIFEST 24
#endif
