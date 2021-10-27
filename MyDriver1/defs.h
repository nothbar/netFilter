#pragma once 


// preprocessor macro for int ==> string coversion
#define PREPROCSTR2(x)          #x
#define PREPROCSTR(x)           PREPROCSTR2(x)

#define INITGUID
#include <guiddef.h>

#include <dontuse.h>
#include <suppress.h>
#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <Fwpsk.h>
#pragma warning(pop)
#include <Fwpmk.h>
#include <limits.h>




//#include <ntstrsafe.h>

// {A234B6E7-274E-4731-8B34-AA16E6103A0F}
DEFINE_GUID(WFP_PROVIDER_GUID,
    0xa234b6e7, 0x274e, 0x4731, 0x8b, 0x34, 0xaa, 0x16, 0xe6, 0x10, 0x3a, 0xf);

// {2FFF9517-0933-4549-ADCE-59588DD08D94}
DEFINE_GUID(WFP_SESSION_GUID,
    0x2fff9517, 0x933, 0x4549, 0xad, 0xce, 0x59, 0x58, 0x8d, 0xd0, 0x8d, 0x94);

// {51AF3CC7-6943-4F4C-85D6-6E26FA158333}
DEFINE_GUID(WFP_SUBLAYER_GUID,
    0x51af3cc7, 0x6943, 0x4f4c, 0x85, 0xd6, 0x6e, 0x26, 0xfa, 0x15, 0x83, 0x33);

// {DDFA30AA-0183-4CC6-8A51-BAD09E43CB6B}
DEFINE_GUID(WFP_AUTH_RECV_ACCEPT_CALLOUT,
    0xddfa30aa, 0x183, 0x4cc6, 0x8a, 0x51, 0xba, 0xd0, 0x9e, 0x43, 0xcb, 0x6b);

// {1363DAB3-3446-48C4-BF44-F5DB0E7FD200}
DEFINE_GUID(WFP_AUTH_CONNECT_CALLOUT,
    0x1363dab3, 0x3446, 0x48c4, 0xbf, 0x44, 0xf5, 0xdb, 0xe, 0x7f, 0xd2, 0x0);
