/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciRomTable.c
  
Abstract:

  Option Rom Support for PCI Bus Driver

Revision History

--*/

#include "PciBus.h"

#include EFI_GUID_DEFINITION (PciOptionRomTable)

#include "PciRomTable.h"

typedef struct {
  EFI_HANDLE  ImageHandle;
  UINTN       Seg;
  UINT8       Bus;
  UINT8       Dev;
  UINT8       Func;
  UINT64      RomAddress;
  UINT64      RomLength;
} EFI_PCI_ROM_IMAGE_MAPPING;

static UINTN                      mNumberOfPciRomImages     = 0;
static UINTN                      mMaxNumberOfPciRomImages  = 0;
static EFI_PCI_ROM_IMAGE_MAPPING  *mRomImageTable           = NULL;

VOID
PciRomAddImageMapping (
  IN EFI_HANDLE  ImageHandle,
  IN UINTN       Seg,
  IN UINT8       Bus,
  IN UINT8       Dev,
  IN UINT8       Func,
  IN UINT64      RomAddress,
  IN UINT64      RomLength
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageHandle - TODO: add argument description
  Seg         - TODO: add argument description
  Bus         - TODO: add argument description
  Dev         - TODO: add argument description
  Func        - TODO: add argument description
  RomAddress  - TODO: add argument description
  RomLength   - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  EFI_PCI_ROM_IMAGE_MAPPING *TempMapping;

  if (mNumberOfPciRomImages >= mMaxNumberOfPciRomImages) {

    mMaxNumberOfPciRomImages += 0x20;

    TempMapping = NULL;
    TempMapping = EfiLibAllocatePool (mMaxNumberOfPciRomImages * sizeof (EFI_PCI_ROM_IMAGE_MAPPING));
    if (TempMapping == NULL) {
      return ;
    }

    EfiCopyMem (TempMapping, mRomImageTable, mNumberOfPciRomImages * sizeof (EFI_PCI_ROM_IMAGE_MAPPING));

    if (mRomImageTable != NULL) {
      gBS->FreePool (mRomImageTable);
    }

    mRomImageTable = TempMapping;
  }

  mRomImageTable[mNumberOfPciRomImages].ImageHandle = ImageHandle;
  mRomImageTable[mNumberOfPciRomImages].Seg         = Seg;
  mRomImageTable[mNumberOfPciRomImages].Bus         = Bus;
  mRomImageTable[mNumberOfPciRomImages].Dev         = Dev;
  mRomImageTable[mNumberOfPciRomImages].Func        = Func;
  mRomImageTable[mNumberOfPciRomImages].RomAddress  = RomAddress;
  mRomImageTable[mNumberOfPciRomImages].RomLength   = RomLength;
  mNumberOfPciRomImages++;
}

VOID
HexToString (
  CHAR16  *String,
  UINTN   Value,
  UINTN   Digits
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  String  - TODO: add argument description
  Value   - TODO: add argument description
  Digits  - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  for (; Digits > 0; Digits--, String++) {
    *String = (CHAR16) ((Value >> (4 * (Digits - 1))) & 0x0f);
    if (*String > 9) {
      (*String) += ('A' - 10);
    } else {
      (*String) += '0';
    }
  }
}

EFI_STATUS
PciRomLoadEfiDriversFromRomImage (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_PCI_OPTION_ROM_DESCRIPTOR  *PciOptionRomDescriptor
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    PciOptionRomDescriptor - add argument and description to function comment
{
  VOID                          *RomBar;
  UINTN                         RomSize;
  CHAR16                        *FileName;
  EFI_PCI_EXPANSION_ROM_HEADER  *EfiRomHeader;
  PCI_DATA_STRUCTURE            *Pcir;
  UINTN                         ImageIndex;
  UINTN                         RomBarOffset;
  UINT32                        ImageSize;
  UINT16                        ImageOffset;
  EFI_HANDLE                    ImageHandle;
  EFI_STATUS                    Status;
  EFI_STATUS                    retStatus;
  EFI_DEVICE_PATH_PROTOCOL      *FilePath;
  BOOLEAN                       SkipImage;
  UINT32                        DestinationSize;
  UINT32                        ScratchSize;
  UINT8                         *Scratch;
  VOID                          *ImageBuffer;
  VOID                          *DecompressedImageBuffer;
  UINT32                        ImageLength;
  EFI_DECOMPRESS_PROTOCOL       *Decompress;

  RomBar = (VOID *) (UINTN) PciOptionRomDescriptor->RomAddress;
  RomSize = (UINTN) PciOptionRomDescriptor->RomLength;
  FileName = L"PciRom Seg=00000000 Bus=00 Dev=00 Func=00 Image=0000";

  HexToString (&FileName[11], PciOptionRomDescriptor->Seg, 8);
  HexToString (&FileName[24], PciOptionRomDescriptor->Bus, 2);
  HexToString (&FileName[31], PciOptionRomDescriptor->Dev, 2);
  HexToString (&FileName[39], PciOptionRomDescriptor->Func, 2);

  ImageIndex    = 0;
  retStatus     = EFI_NOT_FOUND;
  RomBarOffset  = (UINTN) RomBar;

  do {

    EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *) (UINTN) RomBarOffset;

    if (EfiRomHeader->Signature != 0xaa55) {
      return retStatus;
    }

    Pcir      = (PCI_DATA_STRUCTURE *) (UINTN) (RomBarOffset + EfiRomHeader->PcirOffset);
    ImageSize = Pcir->ImageLength * 512;

    if ((Pcir->CodeType == PCI_CODE_TYPE_EFI_IMAGE) &&
        (EfiRomHeader->EfiSignature == EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE) ) {

      if ((EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) ||
          (EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) ) {

        ImageOffset             = EfiRomHeader->EfiImageHeaderOffset;
        ImageSize               = EfiRomHeader->InitializationSize * 512;

        ImageBuffer             = (VOID *) (UINTN) (RomBarOffset + ImageOffset);
        ImageLength             = ImageSize - ImageOffset;
        DecompressedImageBuffer = NULL;

        //
        // decompress here if needed
        //
        SkipImage = FALSE;
        if (EfiRomHeader->CompressionType > EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          SkipImage = TRUE;
        }

        if (EfiRomHeader->CompressionType == EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          Status = gBS->LocateProtocol (&gEfiDecompressProtocolGuid, NULL, (VOID **) &Decompress);
          if (EFI_ERROR (Status)) {
            SkipImage = TRUE;
          } else {
            SkipImage = TRUE;
            Status = Decompress->GetInfo (
                                  Decompress,
                                  ImageBuffer,
                                  ImageLength,
                                  &DestinationSize,
                                  &ScratchSize
                                  );
            if (!EFI_ERROR (Status)) {
              DecompressedImageBuffer = NULL;
              DecompressedImageBuffer = EfiLibAllocatePool (DestinationSize);
              if (DecompressedImageBuffer != NULL) {
                Scratch = EfiLibAllocatePool (ScratchSize);
                if (Scratch != NULL) {
                  Status = Decompress->Decompress (
                                        Decompress,
                                        ImageBuffer,
                                        ImageLength,
                                        DecompressedImageBuffer,
                                        DestinationSize,
                                        Scratch,
                                        ScratchSize
                                        );
                  if (!EFI_ERROR (Status)) {
                    ImageBuffer = DecompressedImageBuffer;
                    ImageLength = DestinationSize;
                    SkipImage   = FALSE;
                  }

                  gBS->FreePool (Scratch);
                }
              }
            }
          }
        }

        if (!SkipImage) {

          //
          // load image and start image
          //

          HexToString (&FileName[48], ImageIndex, 4);
          FilePath = EfiFileDevicePath (NULL, FileName);

          Status = gBS->LoadImage (
                          FALSE,
                          This->ImageHandle,
                          FilePath,
                          ImageBuffer,
                          ImageLength,
                          &ImageHandle
                          );
          if (!EFI_ERROR (Status)) {
            Status = gBS->StartImage (ImageHandle, NULL, NULL);
            if (!EFI_ERROR (Status)) {
              PciRomAddImageMapping (
                ImageHandle,
                PciOptionRomDescriptor->Seg,
                PciOptionRomDescriptor->Bus,
                PciOptionRomDescriptor->Dev,
                PciOptionRomDescriptor->Func,
                PciOptionRomDescriptor->RomAddress,
                PciOptionRomDescriptor->RomLength
                );
              retStatus = Status;
            }
          }
        }

        if (DecompressedImageBuffer != NULL) {
          gBS->FreePool (DecompressedImageBuffer);
        }

      }
    }

    RomBarOffset = RomBarOffset + ImageSize;
    ImageIndex++;
  } while (((Pcir->Indicator & 0x80) == 0x00) && ((RomBarOffset - (UINTN) RomBar) < RomSize));

  return retStatus;
}

EFI_STATUS
PciRomLoadEfiDriversFromOptionRomTable (
  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    PciRootBridgeIo - add argument and description to function comment
// TODO:    EFI_NOT_FOUND - add return value to function comment
{
  EFI_STATUS                        Status;
  EFI_PCI_OPTION_ROM_TABLE          *PciOptionRomTable;
  EFI_PCI_OPTION_ROM_DESCRIPTOR     *PciOptionRomDescriptor;
  UINTN                             Index;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptors;
  UINT16                            MinBus;
  UINT16                            MaxBus;

  Status = EfiLibGetSystemConfigurationTable (&gEfiPciOptionRomTableGuid, (VOID **) &PciOptionRomTable);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;

  for (Index = 0; Index < PciOptionRomTable->PciOptionRomCount; Index++) {
    PciOptionRomDescriptor = &PciOptionRomTable->PciOptionRomDescriptors[Index];
    if (!PciOptionRomDescriptor->DontLoadEfiRom) {
      if (PciOptionRomDescriptor->Seg == PciRootBridgeIo->SegmentNumber) {
        Status = PciRootBridgeIo->Configuration (PciRootBridgeIo, (VOID **) &Descriptors);
        if (EFI_ERROR (Status)) {
          return Status;
        }

        PciGetBusRange (&Descriptors, &MinBus, &MaxBus, NULL);
        if ((MinBus <= PciOptionRomDescriptor->Bus) && (PciOptionRomDescriptor->Bus <= MaxBus)) {
          Status = PciRomLoadEfiDriversFromRomImage (This, PciOptionRomDescriptor);
          PciOptionRomDescriptor->DontLoadEfiRom |= 2;
        }
      }
    }
  }

  return Status;
}

EFI_STATUS
PciRomGetRomResourceFromPciOptionRomTable (
  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  PCI_IO_DEVICE                       *PciIoDevice
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    PciRootBridgeIo - add argument and description to function comment
// TODO:    PciIoDevice - add argument and description to function comment
// TODO:    EFI_NOT_FOUND - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                    Status;
  EFI_PCI_OPTION_ROM_TABLE      *PciOptionRomTable;
  EFI_PCI_OPTION_ROM_DESCRIPTOR *PciOptionRomDescriptor;
  UINTN                         Index;

  Status = EfiLibGetSystemConfigurationTable (&gEfiPciOptionRomTableGuid, (VOID **) &PciOptionRomTable);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < PciOptionRomTable->PciOptionRomCount; Index++) {
    PciOptionRomDescriptor = &PciOptionRomTable->PciOptionRomDescriptors[Index];
    if (PciOptionRomDescriptor->Seg == PciRootBridgeIo->SegmentNumber &&
        PciOptionRomDescriptor->Bus == PciIoDevice->BusNumber         &&
        PciOptionRomDescriptor->Dev == PciIoDevice->DeviceNumber      &&
        PciOptionRomDescriptor->Func == PciIoDevice->FunctionNumber ) {

      PciIoDevice->PciIo.RomImage = (VOID *) (UINTN) PciOptionRomDescriptor->RomAddress;
      PciIoDevice->PciIo.RomSize  = (UINTN) PciOptionRomDescriptor->RomLength;
    }
  }

  for (Index = 0; Index < mNumberOfPciRomImages; Index++) {
    if (mRomImageTable[Index].Seg  == PciRootBridgeIo->SegmentNumber &&
        mRomImageTable[Index].Bus  == PciIoDevice->BusNumber         &&
        mRomImageTable[Index].Dev  == PciIoDevice->DeviceNumber      &&
        mRomImageTable[Index].Func == PciIoDevice->FunctionNumber    ) {

      AddDriver (PciIoDevice, mRomImageTable[Index].ImageHandle);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciRomGetImageMapping (
  PCI_IO_DEVICE                       *PciIoDevice
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
// TODO:    PciIoDevice - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
  UINTN                           Index;

  PciRootBridgeIo = PciIoDevice->PciRootBridgeIo;

  for (Index = 0; Index < mNumberOfPciRomImages; Index++) {
    if (mRomImageTable[Index].Seg  == PciRootBridgeIo->SegmentNumber &&
        mRomImageTable[Index].Bus  == PciIoDevice->BusNumber         &&
        mRomImageTable[Index].Dev  == PciIoDevice->DeviceNumber      &&
        mRomImageTable[Index].Func == PciIoDevice->FunctionNumber    ) {

      if (mRomImageTable[Index].ImageHandle != NULL) {
        AddDriver (PciIoDevice, mRomImageTable[Index].ImageHandle);
      } else {
        PciIoDevice->PciIo.RomImage = (VOID *) (UINTN) mRomImageTable[Index].RomAddress;
        PciIoDevice->PciIo.RomSize  = (UINTN) mRomImageTable[Index].RomLength;
      }
    }
  }

  return EFI_SUCCESS;
}
