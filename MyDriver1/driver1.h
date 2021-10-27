#pragma once 

#include "defs.h"

//
// driver and NT legacy device name
//
#define WFP_DRIVER_NAME             "MyDriver1.sys"
#define WFP_DEVICE_NAME             L"\\MyDriver1"
#define WFP_TAG_GLB                 'PFW:'


//
// GLOBAL DATA
//
typedef struct _DRV_GLOBAL_DATA 
{
    PDRIVER_OBJECT  DriverObject;       // a driver and a device object - for an NT legacy style driver
    PDEVICE_OBJECT  DeviceObject;
    UNICODE_STRING  DeviceName;
    LARGE_INTEGER   DriverLoadTime;
    
    HANDLE          EngineHandle;       // handle for the open session to the filter engine
    BOOLEAN         ProviderAdded;
    BOOLEAN         SublayerAdded;
    
    /// ...

} DRV_GLOBAL_DATA, *PDRV_GLOBAL_DATA;

extern PDRV_GLOBAL_DATA gDrv;


