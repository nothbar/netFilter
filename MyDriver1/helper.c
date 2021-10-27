#include "helper.h"

#include "Trace.h"
#include "helper.tmh"


extern PDRV_GLOBAL_DATA gDrv;



NTSTATUS
RegisterAndAddCallout(
    GUID  CalloutKey,
    GUID  *ProviderKey,
    GUID  ApplicableLayer
)
{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    FWPS_CALLOUT0   sCallout = { 0 };
    FWPM_CALLOUT0   mCallout = { 0 };
    UINT32          calloutId = 0;

    __try
    {
        //
        // register the callout
        //
        RtlZeroMemory(&sCallout, sizeof(sCallout));

        sCallout.calloutKey     = CalloutKey;         // WDF_AUTH_RECV_ACCEPT_CALLOUT;
        sCallout.classifyFn     = MonitorCallout;     // classify fn
        sCallout.notifyFn       = NotifyCallout;      // notify fn

        status = FwpsCalloutRegister0(
            gDrv->DeviceObject,
            &sCallout,
            &calloutId);            // TODO: save this for cleanup
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("FwpsCalloutRegister0", status);
            __leave;
        }
        LogInfo("  Registered callout: %llu", calloutId);

        //
        // add the callout
        //
        mCallout.displayData.name = L"callout name";
        mCallout.applicableLayer  = ApplicableLayer;     // FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
        mCallout.calloutKey       = CalloutKey;          // WDF_AUTH_RECV_ACCEPT_CALLOUT;
        mCallout.providerKey      = ProviderKey;         // &provider.providerKey;

        status = FwpmCalloutAdd0(
            gDrv->EngineHandle,
            &mCallout,
            NULL,         // default security desc
            &calloutId);  // same as about calloutid
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("FwpmCalloutAdd0", status);
            __leave;
        }
        LogInfo("  Added callout: %llu", calloutId);

    }
    __finally
    {
    }

    return status;
}


NTSTATUS
AddFilter(
    GUID LayerKey,
    FWP_ACTION_TYPE ActionType,
    GUID SubLayerKey,
    GUID CalloutKey
)
{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    FWPM_FILTER0    filter = { 0 };
    UINT64          filterId = 0;

    __try
    {
        RtlZeroMemory(&filter, sizeof(FWPM_FILTER));

        filter.displayData.name     = L"ale filter name";
        filter.layerKey             = LayerKey; //FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
        filter.action.type          = ActionType; // FWP_ACTION_CALLOUT_INSPECTION;        // We're only doing inspection.
        filter.action.calloutKey    = CalloutKey;// WDF_AUTH_RECV_ACCEPT_CALLOUT;
        filter.subLayerKey          = SubLayerKey;// sublayer.subLayerKey;
        filter.weight.type          = FWP_EMPTY;                            // auto-weight.
        filter.numFilterConditions  = 0;                                    // this applies to all application traffic

        status = FwpmFilterAdd0(
            gDrv->EngineHandle,
            &filter,
            NULL,           // default security desc
            &filterId);     // TODO: save this for cleanup
        if (!NT_SUCCESS(status))
        {
            LogErrorNt("FwpmFilterAdd", status);
            __leave;
        }
        LogInfo(" Added filter: %llu", filterId);

    }
    __finally
    {
    }

    return status;
}