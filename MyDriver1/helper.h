#pragma once

#include "driver1.h"
#include "callouts.h"

NTSTATUS
RegisterAndAddCallout(
    GUID  CalloutKey,
    GUID  *ProviderKey,
    GUID  ApplicableLayer
);

NTSTATUS
AddFilter(
    GUID LayerKey,
    FWP_ACTION_TYPE ActionType,
    GUID SubLayerKey,
    GUID CalloutKey
);