#include "driver1.h"
#include "callouts.h"
#include "helper.h"

#include "Trace.h"
#include "driver1.tmh"


//
// Local prototypes
//
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath 
);

VOID
DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject 
);

VOID
DriverCleanup(
    _In_ PDRIVER_OBJECT DriverObject
);



//
// Assign text sections for each routine
//
#ifdef ALLOC_PRAGMA
    #pragma message("IMPORTANT: ALLOC_PRAGMA defined ==> using INIT and PAGE code sections")
    #pragma alloc_text(INIT, DriverEntry)
    #pragma alloc_text(PAGE, DriverUnload)
#else
    #pragma message("ALLOC_PRAGMA NOT defined ==> all driver code is NON-PAGED")
#endif


//
// Import kernel variable to detect safe mode
//
extern PULONG InitSafeBootMode;


//
// Global data for our driver, allocated dynamically
//
PDRV_GLOBAL_DATA gDrv = NULL;

FWPS_CALLOUT_CLASSIFY_FN0   FwpsCalloutClassifyFn0;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath 
    )
/*++

Routine Description:

    This is the initialization routine for this driver.

Arguments:

    DriverObject - Pointer to driver object created by the system to represent this driver.
    RegistryPath - Unicode string identifying where the parameters for this driver are
                   located in the registry.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{   
    NTSTATUS        status  = STATUS_UNSUCCESSFUL;
    FWPM_PROVIDER0  provider = { 0 };
    FWPM_SESSION0   session = { 0 };
    FWPM_SUBLAYER0  sublayer = { 0 };
//    FWPM_CALLOUT0   mCallout = { 0 };
//    FWPS_CALLOUT0   sCallout = { 0 };
//    FWPM_FILTER0    filter = { 0 };
    

    WPP_INIT_TRACING(DriverObject, RegistryPath);

    LogInfo("DriverEntry called ...");


    __try
    {
        if (*InitSafeBootMode > 0)
        {
            LogError("SAFE MODE ==> SKIP LOADING .SYS");
              status = STATUS_NOT_SAFE_MODE_DRIVER;
            __leave;
        }
        LogInfo(" Try to load driver...");

        gDrv = ExAllocatePoolWithTag(NonPagedPool, sizeof(DRV_GLOBAL_DATA), WFP_TAG_GLB);
        if (NULL == gDrv)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            LogErrorNt("ExAllocatePoolWithTag", status);
            __leave;
        }
        RtlZeroMemory(gDrv, sizeof(DRV_GLOBAL_DATA));

        gDrv->DriverObject = DriverObject;
        KeQuerySystemTime(&gDrv->DriverLoadTime);    

        RtlInitUnicodeString(&(gDrv->DeviceName), WFP_DEVICE_NAME);
        status = IoCreateDevice(
            DriverObject,
            0,
            &gDrv->DeviceName,
            FILE_DEVICE_UNKNOWN,
            0,
            FALSE,
            &gDrv->DeviceObject);
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("IoCreateDevice", status);
            __leave;
        }

        gDrv->DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING; // [!] mai trebuie? nu
        gDrv->DriverObject->DriverUnload = DriverUnload;

        //
        // Open handle to filter engine
        //
        RtlZeroMemory(&session, sizeof(session));

        session.displayData.name        = L"session name";
        session.displayData.description = L"session description";
        session.sessionKey              = WFP_SESSION_GUID;
        session.flags                   = FWPM_SESSION_FLAG_DYNAMIC;    // Changes are dropped when session closes
        session.txnWaitTimeoutInMSec    = 0;                            // If zero, a default time-out value will be used
        
        status = FwpmEngineOpen0(
            NULL,                   // The filter engine on the local system
            RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
            NULL,                   // Use the calling thread&#39;s credentials
            &session,               // There are session-specific parameters
            &gDrv->EngineHandle     // Pointer to a variable to receive the handle
        );
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("FwpmEngineOpen", status);
            __leave;
        }
        LogInfo(" Openeded engine");

        //
        // init & add provider
        //
        RtlZeroMemory(&provider, sizeof(provider));

        provider.displayData.name   = L"provider name";
        provider.providerKey        = WFP_PROVIDER_GUID;

        status = FwpmProviderAdd0(
            gDrv->EngineHandle,
            (const FWPM_PROVIDER0 *)&provider,
            NULL); // default sec desc
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("FwpmProviderAdd0", status);
            __leave;
        }
        gDrv->ProviderAdded = TRUE;
        LogInfo(" Added provider");

        //
        // Init and add sublayer
        //
        RtlZeroMemory(&sublayer, sizeof(sublayer));

        sublayer.displayData.name           = L"mySublayer";
        sublayer.displayData.description    = L"mySublayer description: finut shod";
        sublayer.subLayerKey                = WFP_SUBLAYER_GUID;                    // UuidCreate();
        sublayer.flags                      = 0;                                    // FWPM_SUBLAYER_FLAG_PERSISTENT -> Causes sublayer to be persistent, surviving across BFE stop / start.
        sublayer.weight                     = FWP_EMPTY;                            //?
        sublayer.providerKey                = &provider.providerKey;

        status = FwpmSubLayerAdd0(
            gDrv->EngineHandle,
            (const FWPM_SUBLAYER0 *)&sublayer,
            NULL);                              //  will assign a default security descriptor
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("FwpmSubLayerAdd", status);
            __leave; 
        }
        gDrv->SublayerAdded = TRUE;
        LogInfo(" Added sublayer");

        {
            // X 1
            RegisterAndAddCallout(
                WFP_AUTH_RECV_ACCEPT_CALLOUT,
                (GUID *)&WFP_PROVIDER_GUID,
                FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4);

            AddFilter(
                FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4,
                FWP_ACTION_CALLOUT_INSPECTION,
                sublayer.subLayerKey,
                WFP_AUTH_RECV_ACCEPT_CALLOUT);

            // X 2
            RegisterAndAddCallout(
                WFP_AUTH_CONNECT_CALLOUT,
                (GUID *)&WFP_PROVIDER_GUID,
                FWPM_LAYER_ALE_AUTH_CONNECT_V4);

            AddFilter(
                FWPM_LAYER_ALE_AUTH_CONNECT_V4,
                FWP_ACTION_CALLOUT_INSPECTION,
                sublayer.subLayerKey,
                WFP_AUTH_CONNECT_CALLOUT);
        }

        status = STATUS_SUCCESS;
    }
    __finally
    {
        if (!NT_SUCCESS(status))
        {
            DriverCleanup(DriverObject);
        }
        else
        {
            LogInfo("Done!");
        }
    }

    return status;
}



VOID
DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
/*++

Routine Description:

    This is the uninitialization routine for this driver.

Arguments:

    DriverObject - Pointer to driver object created by the system to represent this driver.

Return Value:

    None.

--*/
{
    PAGED_CODE();

    LogInfo("[CLEANUP] DriverUnload called ...");

    DriverCleanup(DriverObject); 
    // don't log after this using WPP

    return;
}



VOID
DriverCleanup(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    if (gDrv->EngineHandle)
    {
        FwpmCalloutDeleteByKey0(gDrv->EngineHandle, &WFP_AUTH_CONNECT_CALLOUT);
        FwpsCalloutUnregisterByKey0(&WFP_AUTH_CONNECT_CALLOUT);
        LogInfo("[CLEANUP] unregistered & deleted WFP_AUTH_CONNECT_CALLOUT");

        FwpmCalloutDeleteByKey0(gDrv->EngineHandle, &WFP_AUTH_RECV_ACCEPT_CALLOUT);
        FwpsCalloutUnregisterByKey0(&WFP_AUTH_RECV_ACCEPT_CALLOUT);
        LogInfo("[CLEANUP] unregistered & deleted WFP_AUTH_RECV_ACCEPT_CALLOUT");

        FwpmSubLayerDeleteByKey0(gDrv->EngineHandle, &WFP_SUBLAYER_GUID);
        LogInfo("[CLEANUP] deleted sublayer");
        
        FwpmProviderDeleteByKey0(gDrv->EngineHandle, &WFP_PROVIDER_GUID);
        LogInfo("[CLEANUP] deleted provider");
        
        FwpmEngineClose(gDrv->EngineHandle);
        LogInfo("[CLEANUP] engine closed");
    }

    IoDeleteDevice(gDrv->DeviceObject);
    gDrv->DeviceObject = NULL;

    if (gDrv != NULL)
    {
        ExFreePoolWithTag(gDrv, WFP_TAG_GLB);
        gDrv = NULL;
    }

    LogInfo("[CLEANUP] Done.");
    WPP_CLEANUP(DriverObject);

    return;
}
