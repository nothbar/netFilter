#include "callouts.h"

#include "Trace.h"
#include "callouts.tmh"

#define IP_1ST(x) ((x >> 24) & 0xFF)
#define IP_2ND(x) ((x >> 16) & 0xFF)
#define IP_3RD(x) ((x >> 8) & 0xFF)
#define IP_4TH(x) (x & 0xFF)

typedef struct  _CLASSIFY_INFO
{
    UINT16 layerId;
    UINT16 protocol;
    UINT16 localPort;
    UINT16 remotePort;
    UINT32 localAddr;
    UINT32 remoteAddr;
    FWP_BYTE_BLOB *appPath;

}CLASSIFY_INFO, PCLASSIFY_INFO;

VOID
PrintClassifyFn(
    _In_ CLASSIFY_INFO Info
)
{
    LogInfo("\t info.layerId   : %s", Info.layerId == FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4 ? "FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4" : "FWPS_LAYER_ALE_AUTH_CONNECT_V4");
    LogInfo("\t info.appPath   : %S", (PWCHAR)Info.appPath->data);
    LogInfo("\t info.protocol  : %u", Info.protocol);
    LogInfo("\t info.localAddr : %u.%u.%u.%u", IP_1ST(Info.localAddr), IP_2ND(Info.localAddr), IP_3RD(Info.localAddr), IP_4TH(Info.localAddr));
    LogInfo("\t info.localPort : %u", Info.localPort);
    LogInfo("\t info.remoteAddr: %u.%u.%u.%u", IP_1ST(Info.remoteAddr), IP_2ND(Info.remoteAddr), IP_3RD(Info.remoteAddr), IP_4TH(Info.remoteAddr));
    LogInfo("\t info.remotePort: %u", Info.remotePort);


    return;
}

VOID NTAPI
MonitorCallout(
    _In_        const FWPS_INCOMING_VALUES0*            inFixedValues,
    _In_        const FWPS_INCOMING_METADATA_VALUES0*   inMetaValues,
    _Inout_opt_ void                                    *layerData,
    _In_        const FWPS_FILTER0                      *filter,
    _In_        UINT64                                  flowContext,
    _Inout_     FWPS_CLASSIFY_OUT0                      *classifyOut
)
{
    CLASSIFY_INFO info = { 0 };

    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);
    
    LogInfo("[classifyFn] called");

    __try
    {
        // print: ver IP, remote Ip adr, port, protocol, app name
        switch (inFixedValues->layerId)
        {
            case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4:
            
                info.layerId    = FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
                info.appPath    = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_ALE_APP_ID].value.byteBlob;
                info.protocol   = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL].value.uint16;
                info.localAddr  = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS].value.uint32;
                info.localPort  = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT].value.uint16;
                info.remoteAddr = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS].value.uint32;
                info.remotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT].value.uint16;

                PrintClassifyFn(info);
            
                break;

            case FWPS_LAYER_ALE_AUTH_CONNECT_V4:
                
                info.layerId    = FWPS_LAYER_ALE_AUTH_CONNECT_V4;
                info.appPath    = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_ALE_APP_ID].value.byteBlob;
                info.protocol   = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_PROTOCOL].value.uint16;         // The IP protocol number, as specified in RFC 1700.
                info.localAddr  = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_LOCAL_ADDRESS].value.uint32;    // The local IP address.
                info.localPort  = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_LOCAL_PORT].value.uint16;
                info.remoteAddr = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_REMOTE_ADDRESS].value.uint32;
                info.remotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_REMOTE_PORT].value.uint16;

                PrintClassifyFn(info);

                break;

            default:
                LogInfo("  inFixedValues->layerId: de ce vine %d?", inFixedValues->layerId); //[!] dbg
                __leave;
                break;
        }

        LogInfo("  inFixedValues->valueCount: %d", inFixedValues->valueCount);

        //__debugbreak();

    }
    __finally
    {
        // If the decision to permit or block should be passed
        // to the next filter in the filter engine...
        classifyOut->actionType = FWP_ACTION_CONTINUE;
    }

    return;
}


NTSTATUS 
NotifyCallout(
    _In_    FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    _In_    const GUID               *filterKey,
    _Inout_ FWPS_FILTER0             *filter
)
{
    UNREFERENCED_PARAMETER(filterKey);

    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (notifyType)
    {
        case FWPS_CALLOUT_NOTIFY_ADD_FILTER:
            LogInfo("[notifyFn] Added filter ID: %llu", filter->filterId);
            break;

        case FWPS_CALLOUT_NOTIFY_DELETE_FILTER:
            LogInfo("[notifyFn] Deleted filter ID: %llu", filter->filterId);
            break;

        default:
            __debugbreak(); // [!] doar curios daca vine altceva pe aici
            break;
    }

    status = STATUS_SUCCESS;

    return status;
}