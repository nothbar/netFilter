#pragma once

// driver version
#define DRIVER_VERSION_HIVER        0
#define DRIVER_VERSION_LOVER        4
#define DRIVER_VERSION_REVISION     402
// NOTE: the build number is always the next build number (current binary + 1)
#define DRIVER_VERSION_BUILD        5605
#define DRIVER_BUILD_DATE           __DATE__
#define DRIVER_BUILD_TIME           __TIME__
#define DRIVER_VERSIONSTAMP         ((((DWORD)DRIVER_VERSION_REVISION) << 16) | ((DWORD)DRIVER_VERSION_BUILD))
