/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DriverDiagnostics.c

Abstract:

--*/

#include "IdeBus.h"

#define IDE_BUS_DIAGNOSTIC_ERROR  L"PCI IDE/ATAPI Driver Diagnostics Failed"

//
// EFI Driver Diagnostics Functions
//
EFI_STATUS
IDEBusDriverDiagnosticsRunDiagnostics (
  IN  EFI_DRIVER_DIAGNOSTICS_PROTOCOL               *This,
  IN  EFI_HANDLE                                    ControllerHandle,
  IN  EFI_HANDLE                                    ChildHandle  OPTIONAL,
  IN  EFI_DRIVER_DIAGNOSTIC_TYPE                    DiagnosticType,
  IN  CHAR8                                         *Language,
  OUT EFI_GUID                                      **ErrorType,
  OUT UINTN                                         *BufferSize,
  OUT CHAR16                                        **Buffer
  );

//
// EFI Driver Diagnostics Protocol
//
EFI_DRIVER_DIAGNOSTICS_PROTOCOL gIDEBusDriverDiagnostics = {
  IDEBusDriverDiagnosticsRunDiagnostics,
  "eng"
};

EFI_STATUS
IDEBusDriverDiagnosticsRunDiagnostics (
  IN  EFI_DRIVER_DIAGNOSTICS_PROTOCOL               *This,
  IN  EFI_HANDLE                                    ControllerHandle,
  IN  EFI_HANDLE                                    ChildHandle  OPTIONAL,
  IN  EFI_DRIVER_DIAGNOSTIC_TYPE                    DiagnosticType,
  IN  CHAR8                                         *Language,
  OUT EFI_GUID                                      **ErrorType,
  OUT UINTN                                         *BufferSize,
  OUT CHAR16                                        **Buffer
  )
/*++

  Routine Description:
    Runs diagnostics on a controller.

  Arguments:
    This             - A pointer to the EFI_DRIVER_DIAGNOSTICS_PROTOCOL 
                       instance.
    ControllerHandle - The handle of the controller to run diagnostics on.
    ChildHandle      - The handle of the child controller to run diagnostics on  
                       This is an optional parameter that may be NULL.  It will 
                       be NULL for device drivers.  It will also be NULL for a 
                       bus drivers that wish to run diagnostics on the bus 
                       controller.  It will not be NULL for a bus driver that 
                       wishes to run diagnostics on one of its child 
                       controllers.
    DiagnosticType   - Indicates type of diagnostics to perform on the 
                       controller specified by ControllerHandle and ChildHandle.
                       See "Related Definitions" for the list of supported 
                       types.
    Language         - A pointer to a three character ISO 639-2 language 
                       identifier.  This is the language in which the optional 
                       error message should be returned in Buffer, and it must 
                       match one of the languages specified in 
                       SupportedLanguages. The number of languages supported by
                       a driver is up to the driver writer.
    ErrorType        - A GUID that defines the format of the data returned in 
                       Buffer.  
    BufferSize       - The size, in bytes, of the data returned in Buffer.  
    Buffer           - A buffer that contains a Null-terminated Unicode string 
                       plus some additional data whose format is defined by 
                       ErrorType.  Buffer is allocated by this function with 
                       AllocatePool(), and it is the caller's responsibility 
                       to free it with a call to FreePool().  

  Returns:
    EFI_SUCCESS           - The controller specified by ControllerHandle and 
                            ChildHandle passed the diagnostic.
    EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a valid 
                            EFI_HANDLE.
    EFI_INVALID_PARAMETER - Language is NULL.
    EFI_INVALID_PARAMETER - ErrorType is NULL.
    EFI_INVALID_PARAMETER - BufferType is NULL.
    EFI_INVALID_PARAMETER - Buffer is NULL.
    EFI_UNSUPPORTED       - The driver specified by This does not support 
                            running diagnostics for the controller specified 
                            by ControllerHandle and ChildHandle.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            type of diagnostic specified by DiagnosticType.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.
    EFI_OUT_OF_RESOURCES  - There are not enough resources available to complete
                            the diagnostics.
    EFI_OUT_OF_RESOURCES  - There are not enough resources available to return
                            the status information in ErrorType, BufferSize, 
                            and Buffer.
    EFI_DEVICE_ERROR      - The controller specified by ControllerHandle and 
                            ChildHandle did not pass the diagnostic.

--*/
{
  EFI_STATUS            Status;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  EFI_BLOCK_IO_PROTOCOL *BlkIo;
  IDE_BLK_IO_DEV        *IdeBlkIoDevice;
  UINT32                VendorDeviceId;
  VOID                  *BlockBuffer;

  *ErrorType  = NULL;
  *BufferSize = 0;

  if (ChildHandle == NULL) {
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gIDEBusDriverGuid,
                    NULL,
                    gIDEBusDriverBinding.DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;
    }

    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo,
                    gIDEBusDriverBinding.DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }

    //
    // Use services of PCI I/O Protocol to test the PCI IDE/ATAPI Controller
    // The following test simply reads the Device ID and Vendor ID.
    // It should never fail.  A real test would perform more advanced
    // diagnostics.
    //

    Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, 1, &VendorDeviceId);
    if (EFI_ERROR (Status) || VendorDeviceId == 0xffffffff) {
      return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
  }

  Status = gBS->OpenProtocol (
                  ChildHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlkIo,
                  gIDEBusDriverBinding.DriverBindingHandle,
                  ChildHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  IdeBlkIoDevice = IDE_BLOCK_IO_DEV_FROM_THIS (BlkIo);

  //
  // Use services available from IdeBlkIoDevice to test the IDE/ATAPI device
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  IdeBlkIoDevice->BlkMedia.BlockSize,
                  (VOID **) &BlockBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = IdeBlkIoDevice->BlkIo.ReadBlocks (
                                  &IdeBlkIoDevice->BlkIo,
                                  IdeBlkIoDevice->BlkMedia.MediaId,
                                  0,
                                  IdeBlkIoDevice->BlkMedia.BlockSize,
                                  BlockBuffer
                                  );

  if (EFI_ERROR (Status)) {
    *ErrorType  = &gIDEBusDriverGuid;
    *BufferSize = sizeof (IDE_BUS_DIAGNOSTIC_ERROR);

    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    (UINTN) (*BufferSize),
                    (VOID **) Buffer
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    EfiCopyMem (*Buffer, IDE_BUS_DIAGNOSTIC_ERROR, *BufferSize);

    Status = EFI_DEVICE_ERROR;
  }

  gBS->FreePool (BlockBuffer);

  return Status;
}
