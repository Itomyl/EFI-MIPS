/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FwBlockService.h
  
Abstract:

  Firmware volume block driver for Intel Firmware Hub (FWH) device

--*/

#ifndef _FW_BLOCK_SERVICE_H
#define _FW_BLOCK_SERVICE_H

//
// Statements that include other header files
//
#include "Tiano.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiRuntimeLib.h"
#include "EfiHobLib.h"

#include EFI_PROTOCOL_PRODUCER (FirmwareVolumeBlock)
#include EFI_PROTOCOL_PRODUCER (FvbExtension)
#include EFI_GUID_DEFINITION (AlternateFvBlock)
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (DxeServices)
#include EFI_PROTOCOL_CONSUMER (CpuIo)

//
// BugBug: Add documentation here for data structure!!!!
//
#define FVB_PHYSICAL  0
#define FVB_VIRTUAL   1

typedef struct {
  EFI_LOCK                    FvbDevLock;
  UINTN                       FvBase[2];
  UINTN                       NumOfBlocks;
  EFI_FIRMWARE_VOLUME_HEADER  VolumeHeader;
} EFI_FW_VOL_INSTANCE;

typedef struct {
  UINT32              NumFv;
  EFI_FW_VOL_INSTANCE *FvInstance[2];
  UINT8               *FvbScratchSpace[2];
} ESAL_FWB_GLOBAL;

//
// Fvb Protocol instance data
//
#define FVB_DEVICE_FROM_THIS(a)         CR (a, EFI_FW_VOL_BLOCK_DEVICE, FwVolBlockInstance, FVB_DEVICE_SIGNATURE)
#define FVB_EXTEND_DEVICE_FROM_THIS(a)  CR (a, EFI_FW_VOL_BLOCK_DEVICE, FvbExtension, FVB_DEVICE_SIGNATURE)
#define FVB_DEVICE_SIGNATURE            EFI_SIGNATURE_32 ('F', 'V', 'B', 'N')

typedef struct {
  MEMMAP_DEVICE_PATH        MemMapDevPath;
  EFI_DEVICE_PATH_PROTOCOL  EndDevPath;
} FV_DEVICE_PATH;

typedef struct {
  UINTN                               Signature;
  FV_DEVICE_PATH                      DevicePath;
  UINTN                               Instance;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  FwVolBlockInstance;
  EFI_FVB_EXTENSION_PROTOCOL          FvbExtension;
} EFI_FW_VOL_BLOCK_DEVICE;

EFI_STATUS
GetFvbInfo (
  IN  UINT64                            FvLength,
  OUT EFI_FIRMWARE_VOLUME_HEADER        **FvbInfo
  )
;

EFI_STATUS
FvbReadBlock (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba,
  IN UINTN                                BlockOffset,
  IN OUT UINTN                            *NumBytes,
  IN UINT8                                *Buffer,
  IN ESAL_FWB_GLOBAL                      *Global,
  IN BOOLEAN                              Virtual
  )
;

EFI_STATUS
FvbWriteBlock (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba,
  IN UINTN                                BlockOffset,
  IN OUT UINTN                            *NumBytes,
  IN UINT8                                *Buffer,
  IN ESAL_FWB_GLOBAL                      *Global,
  IN BOOLEAN                              Virtual
  )
;

EFI_STATUS
FvbEraseBlock (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba,
  IN ESAL_FWB_GLOBAL                      *Global,
  IN BOOLEAN                              Virtual
  )
;

EFI_STATUS
FvbSetVolumeAttributes (
  IN UINTN                                Instance,
  IN OUT EFI_FVB_ATTRIBUTES               *Attributes,
  IN ESAL_FWB_GLOBAL                      *Global,
  IN BOOLEAN                              Virtual
  )
;

EFI_STATUS
FvbGetVolumeAttributes (
  IN UINTN                                Instance,
  OUT EFI_FVB_ATTRIBUTES                  *Attributes,
  IN ESAL_FWB_GLOBAL                      *Global,
  IN BOOLEAN                              Virtual
  )
;

EFI_STATUS
FvbGetPhysicalAddress (
  IN UINTN                                Instance,
  OUT EFI_PHYSICAL_ADDRESS                *Address,
  IN ESAL_FWB_GLOBAL                      *Global,
  IN BOOLEAN                              Virtual
  )
;

EFI_STATUS
EFIAPI
FvbInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
;

EFI_RUNTIMESERVICE
VOID
FvbClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
;

EFI_STATUS
FvbGetLbaAddress (
  IN  UINTN                               Instance,
  IN  EFI_LBA                             Lba,
  OUT UINTN                               *LbaAddress,
  OUT UINTN                               *LbaLength,
  OUT UINTN                               *NumOfBlocks,
  IN  ESAL_FWB_GLOBAL                     *Global,
  IN  BOOLEAN                             Virtual
  )
;

//
// Protocol APIs
//
EFI_STATUS
EFIAPI
FvbProtocolGetAttributes (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  OUT EFI_FVB_ATTRIBUTES                          *Attributes
  )
;

EFI_STATUS
EFIAPI
FvbProtocolSetAttributes (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN OUT EFI_FVB_ATTRIBUTES                       *Attributes
  )
;

EFI_STATUS
EFIAPI
FvbProtocolGetPhysicalAddress (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  OUT EFI_PHYSICAL_ADDRESS                        *Address
  )
;

EFI_STATUS
EFIAPI
FvbProtocolGetBlockSize (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN  EFI_LBA                                     Lba,
  OUT UINTN                                       *BlockSize,
  OUT UINTN                                       *NumOfBlocks
  )
;

EFI_STATUS
EFIAPI
FvbProtocolRead (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  )
;

EFI_STATUS
EFIAPI
FvbProtocolWrite (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  )
;

EFI_STATUS
EFIAPI
FvbProtocolEraseBlocks (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    *This,
  ...
  )
;

EFI_STATUS
EFIAPI
FvbExtendProtocolEraseCustomBlockRange (
  IN EFI_FVB_EXTENSION_PROTOCOL           *This,
  IN EFI_LBA                              StartLba,
  IN UINTN                                OffsetStartLba,
  IN EFI_LBA                              LastLba,
  IN UINTN                                OffsetLastLba
  )
;

#endif
