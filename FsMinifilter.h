#pragma once

EXTERN_C_START
    FLT_PREOP_CALLBACK_STATUS FLTAPI PreOperationCreate(
        _Inout_ PFLT_CALLBACK_DATA Data,
        _In_ PCFLT_RELATED_OBJECTS FltObjects,
        _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
    );
EXTERN_C_END
