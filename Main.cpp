#include "pch.h"
#include "FsMinifilter.h"

EXTERN_C_START
    DRIVER_INITIALIZE DriverEntry;
    
    NTSTATUS FLTAPI InstanceFilterUnloadCallback(_In_ FLT_FILTER_UNLOAD_FLAGS Flags);
    
    NTSTATUS FLTAPI InstanceSetupCallback(
        _In_ PCFLT_RELATED_OBJECTS  FltObjects,
        _In_ FLT_INSTANCE_SETUP_FLAGS  Flags,
        _In_ DEVICE_TYPE  VolumeDeviceType,
        _In_ FLT_FILESYSTEM_TYPE  VolumeFilesystemType);

    NTSTATUS FLTAPI InstanceQueryTeardownCallback(
        _In_ PCFLT_RELATED_OBJECTS FltObjects,
        _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );
EXTERN_C_END

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT, DriverEntry)
    #pragma alloc_text (PAGE, InstanceFilterUnloadCallback)
    #pragma alloc_text (PAGE, InstanceSetupCallback)
    #pragma alloc_text (PAGE, InstanceQueryTeardownCallback)
    #pragma alloc_text (PAGE, PreOperationCreate)
#endif

//
// The minifilter handle that results from a call to FltRegisterFilter
// NOTE: This handle must be passed to FltUnregisterFilter during minifilter unloading
//
PFLT_FILTER g_minifilterHandle = nullptr;

//
// Constant FLT_REGISTRATION structure for our filter.
// This initializes the callback routines our filter wants to register for.
//
CONST FLT_OPERATION_REGISTRATION g_callbacks[] =
{
    {
        IRP_MJ_CREATE,
        0,
        PreOperationCreate,
        0
    },

    { IRP_MJ_OPERATION_END }
};

//
// The FLT_REGISTRATION structure provides information about a file system minifilter to the filter manager.
//
CONST FLT_REGISTRATION g_filterRegistration =
{
    sizeof(FLT_REGISTRATION),      //  Size
    FLT_REGISTRATION_VERSION,      //  Version
    0,                             //  Flags
    NULL,                          //  Context registration
    g_callbacks,                   //  Operation callbacks
    InstanceFilterUnloadCallback,  //  FilterUnload
    InstanceSetupCallback,         //  InstanceSetup
    InstanceQueryTeardownCallback, //  InstanceQueryTeardown
    NULL,                          //  InstanceTeardownStart
    NULL,                          //  InstanceTeardownComplete
    NULL,                          //  GenerateFileName
    NULL,                          //  GenerateDestinationFileName
    NULL                           //  NormalizeNameComponent
};

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    //
    // register minifilter driver
    //
    NTSTATUS status = FltRegisterFilter(DriverObject, &g_filterRegistration, &g_minifilterHandle);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    //
    // start minifilter driver
    //
    status = FltStartFiltering(g_minifilterHandle);
    if (!NT_SUCCESS(status))
    {
        FltUnregisterFilter(g_minifilterHandle);
    }

    return status;
}

NTSTATUS FLTAPI InstanceFilterUnloadCallback(_In_ FLT_FILTER_UNLOAD_FLAGS Flags)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(Flags);

    //
    // This is called before a filter is unloaded.
    // If NULL is specified for this routine, then the filter can never be unloaded.
    //

    if (g_minifilterHandle)
    {
        FltUnregisterFilter(g_minifilterHandle);
    }

    return STATUS_SUCCESS;
}

NTSTATUS FLTAPI InstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS  FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS  Flags,
    _In_ DEVICE_TYPE  VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE  VolumeFilesystemType)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    //
    // This is called to see if a filter would like to attach an instance to the given volume.
    //

    return STATUS_SUCCESS;
}

NTSTATUS FLTAPI InstanceQueryTeardownCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    //
    // This is called to see if the filter wants to detach from the given volume.
    //

    return STATUS_SUCCESS;
}
