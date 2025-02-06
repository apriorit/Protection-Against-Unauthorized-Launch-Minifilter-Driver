#include "pch.h"

#include "FsMinifilter.h"
#include "FilenameInfromationGuard.h"

static const HANDLE g_systemProcessId = reinterpret_cast<HANDLE>(4);

FLT_PREOP_CALLBACK_STATUS FLTAPI PreOperationCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(CompletionContext);

    //
    // Pre-create callback to get file info during creation or opening
    //

    if (!Data || !FltObjects)
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    // Skip if this PreCreate call was performed from the System process.
    //
    if (PsGetCurrentProcessId() == g_systemProcessId)
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    // Check if the FileObject being processed represents: Named pipe, Mailslot, or Volume. Skip this call if it returns true.
    // 
    if (FltObjects->FileObject->Flags & (FO_NAMED_PIPE | FO_MAILSLOT | FO_VOLUME_OPEN))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    // Check if the target file is a directory, paging file, or open by file ID. Skip this call if it returns true.
    //
    const auto& createParams = Data->Iopb->Parameters.Create;
    if (FlagOn(createParams.Options, FILE_DIRECTORY_FILE) ||
        FsRtlIsPagingFile(FltObjects->FileObject) ||
        // When the file opens by file ID, this is out of the scope of our example.
        FlagOn(createParams.Options, FILE_OPEN_BY_FILE_ID))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    PFLT_FILE_NAME_INFORMATION fileNameInfo = nullptr;
    FilenameInformationGuard guard(&fileNameInfo); // guard for PFLT_FILE_NAME_INFORMATION

    NTSTATUS status = FltGetFileNameInformation(
        Data,
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
        &fileNameInfo
    );
    
    if (!NT_SUCCESS(status) || !fileNameInfo) 
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    
    //
    // Retrieve filename information from data that passed to PreCreate callback.
    //
    status = FltParseFileNameInformation(fileNameInfo);
    if (!NT_SUCCESS(status)) 
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    // 
    // Check is FinalComponent valid after parsing the filename.
    // 
    if (!fileNameInfo->FinalComponent.Length)
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    // The filename of the unauthorized file that is not eligible to be opened in the system.
    //
    const UNICODE_STRING unauthorizedFileName = RTL_CONSTANT_STRING(L"passwords.txt");

    if (RtlCompareUnicodeString(&fileNameInfo->FinalComponent, &unauthorizedFileName, TRUE) == 0)
    {
        DbgPrint("FsMinifiler - Blocked! The user tried to launch of unauthorized file: %wZ\n", &fileNameInfo->Name);
        
        // Deny access to the target file to protect the system from unauthorized opening.
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = IO_REPARSE;

        return FLT_PREOP_COMPLETE;
    }

    //
    // The filename of the unauthorized process that is not eligible to be launched in the system.
    //
    const UNICODE_STRING unauthorizedProcessName = RTL_CONSTANT_STRING(L"msedge.exe");

    if ((createParams.SecurityContext->DesiredAccess & FILE_EXECUTE) &&
        RtlCompareUnicodeString(&fileNameInfo->FinalComponent, &unauthorizedProcessName, TRUE) == 0)
    {
        DbgPrint("FsMinifiler - Blocked! The user tried to launch of unauthorized file: %wZ\n", &fileNameInfo->Name);
        
        // Deny access to the target process to protect the system from unauthorized launching.
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = IO_REPARSE;

        return FLT_PREOP_COMPLETE;
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
