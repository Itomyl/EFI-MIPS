/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name:
  SecMain.h

Abstract:
  Include file for Windows API based SEC

--*/

// TODO: add protective #ifndef
#include "Efi2Linux.h"
#include "PeiHob.h"
#include "PeiLib.h"
#include "PeiHobLib.h"
#include "EfiImageFormat.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiFirmwareFileSystem.h"
#include "FlashLayout.h"
#include "EfiStatusCode.h"
#include "EfiCommonLib.h"
#include "Pei.h"

#include EFI_PPI_DEFINITION (LinuxPeiLoadFile)
#include EFI_PPI_DEFINITION (LinuxAutoScan)
#include EFI_PPI_DEFINITION (LinuxThunkPpi)
#include EFI_PPI_DEFINITION (LinuxFwh)
#include EFI_PPI_DEFINITION (LinuxLoadAsDll)
#include EFI_PPI_DEFINITION (StatusCode)
#include EFI_PPI_DEFINITION (TemporaryRamSupport)

#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)

#include EFI_GUID_DEFINITION (FirmwareFileSystem)
#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiElfLoader)
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

#define EFI_MEMORY_SIZE_STR       "EFI_MEMORY_SIZE"
#define EFI_FIRMWARE_VOLUMES_STR  "EFI_FIRMWARE_VOLUMES"
#define EFI_BOOT_MODE_STR         "EFI_BOOT_MODE"

#define STACK_SIZE                0x20000      

typedef struct {
  EFI_PHYSICAL_ADDRESS  Address;
  UINT64                Size;
} LINUX_FD_INFO;

typedef struct {
  EFI_PHYSICAL_ADDRESS  Memory;
  UINT64                Size;
} LINUX_SYSTEM_MEMORY;



EFI_STATUS
EFIAPI
SecLinuxPeiLoadFile (
  VOID                  *Pe32Data,  // TODO: add IN/OUT modifier to Pe32Data
  EFI_PHYSICAL_ADDRESS  *ImageAddress,  // TODO: add IN/OUT modifier to ImageAddress
  UINT64                *ImageSize,  // TODO: add IN/OUT modifier to ImageSize
  EFI_PHYSICAL_ADDRESS  *EntryPoint  // TODO: add IN/OUT modifier to EntryPoint
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Pe32Data      - TODO: add argument description
  ImageAddress  - TODO: add argument description
  ImageSize     - TODO: add argument description
  EntryPoint    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxPeiAutoScan (
  IN  UINTN                 Index,
  OUT EFI_PHYSICAL_ADDRESS  *MemoryBase,
  OUT UINT64                *MemorySize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Index       - TODO: add argument description
  MemoryBase  - TODO: add argument description
  MemorySize  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxThunkAddress (
  IN OUT UINT64                *InterfaceSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *InterfaceBase
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  InterfaceSize - TODO: add argument description
  InterfaceBase - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxFwhAddress (
  IN OUT UINT64                *FwhSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *FwhBase
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FwhSize - TODO: add argument description
  FwhBase - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecPeiReportStatusCode (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_STATUS_CODE_TYPE       CodeType,
  IN EFI_STATUS_CODE_VALUE      Value,
  IN UINT32                     Instance,
  IN EFI_GUID                   *CallerId,
  IN EFI_STATUS_CODE_DATA       *Data OPTIONAL
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PeiServices - TODO: add argument description
  CodeType    - TODO: add argument description
  Value       - TODO: add argument description
  Instance    - TODO: add argument description
  CallerId    - TODO: add argument description
  Data        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SecSwitchStacks (
  IN VOID                       *EntryPoint,
  IN EFI_PEI_STARTUP_DESCRIPTOR *PeiStartup,
  IN VOID                       *NewStack,
  IN VOID                       *NewBsp
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  EntryPoint  - TODO: add argument description
  PeiStartup  - TODO: add argument description
  NewStack    - TODO: add argument description
  NewBsp      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
CopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Destination - TODO: add argument description
  Source      - TODO: add argument description
  Length      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_PHYSICAL_ADDRESS *
MapMemory (
  INTN fd,
  UINT64 length,
  INTN   prot,
  INTN   flags
);

EFI_STATUS
MapFile (
  IN  CHAR8                     *FileName,
  IN OUT  EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT UINT64                    *Length
  );

int
main (
  IN  int   Argc,
  IN  char  **Argv,
  IN  char  **Envp
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Argc  - TODO: add argument description
  Argv  - TODO: add argument description
  Envp  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SecLoadFromCore (
  IN  UINTN   LargestRegion,
  IN  UINTN   LargestRegionSize,
  IN  UINTN   BootFirmwareVolumeBase,
  IN  VOID    *PeiCoreFile
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  LargestRegion           - TODO: add argument description
  LargestRegionSize       - TODO: add argument description
  BootFirmwareVolumeBase  - TODO: add argument description
  PeiCoreFile             - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecLoadFile (
  IN  VOID                    *Pe32Data,
  IN  EFI_PHYSICAL_ADDRESS    *ImageAddress,
  IN  UINT64                  *ImageSize,
  IN  EFI_PHYSICAL_ADDRESS    *EntryPoint
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Pe32Data      - TODO: add argument description
  ImageAddress  - TODO: add argument description
  ImageSize     - TODO: add argument description
  EntryPoint    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecFfsFindPeiCore (
  IN  EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  OUT VOID                        **Pe32Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FwVolHeader - TODO: add argument description
  Pe32Data    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecFfsFindNextFile (
  IN EFI_FV_FILETYPE             SearchType,
  IN EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  IN OUT EFI_FFS_FILE_HEADER     **FileHeader
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  SearchType  - TODO: add argument description
  FwVolHeader - TODO: add argument description
  FileHeader  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecFfsFindSectionData (
  IN EFI_SECTION_TYPE      SectionType,
  IN EFI_FFS_FILE_HEADER   *FfsFileHeader,
  IN OUT VOID              **SectionData
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  SectionType   - TODO: add argument description
  FfsFileHeader - TODO: add argument description
  SectionData   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxElfLoaderLoadAsDll (
  IN CHAR8    *PdbFileName,
  IN VOID     **ImageEntryPoint,
  OUT VOID    **ModHandle
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PdbFileName     - TODO: add argument description
  ImageEntryPoint - TODO: add argument description
  ModHandle       - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxElfLoaderFreeLibrary (
  OUT VOID    *ModHandle
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ModHandle - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxFdAddress (
  IN     UINTN                 Index,
  IN OUT EFI_PHYSICAL_ADDRESS  *FdBase,
  IN OUT UINT64                *FdSize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Index   - TODO: add argument description
  FdBase  - TODO: add argument description
  FdSize  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecLinuxFlushCache (
  IN EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL *This,
  IN EFI_PHYSICAL_ADDRESS                     Start,
  IN UINT64                                   Length
  )

/*++

Routine Description:
  Linux Flush cache

Arguments:
  Start  -
  Length -

Returns:
  EFI_SUCCESS

--*/
;

EFI_STATUS
GetImageReadFunction (
  IN EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext,
  IN EFI_PHYSICAL_ADDRESS                  *TopOfMemory
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageContext  - TODO: add argument description
  TopOfMemory   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FileHandle  - TODO: add argument description
  FileOffset  - TODO: add argument description
  ReadSize    - TODO: add argument description
  Buffer      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InstallEfiPeiTransferControl (
  IN OUT EFI_PEI_TRANSFER_CONTROL_PROTOCOL  **This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InstallEfiPeiFlushInstructionCache (
  IN OUT EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  **This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

CHAR16                            *
AsciiToUnicode (
  IN  CHAR8   *Ascii,
  IN  UINTN   *StrLen OPTIONAL
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Ascii   - TODO: add argument description
  StrLen  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINTN
CountSeperatorsInString (
  IN  CHAR8   *String,
  IN  CHAR8   Seperator
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  String    - TODO: add argument description
  Seperator - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecTemporaryRamSupport (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN EFI_PHYSICAL_ADDRESS     TemporaryMemoryBase,
  IN EFI_PHYSICAL_ADDRESS     PermanentMemoryBase,
  IN UINTN                    CopySize
  );

EFI_STATUS
EFIAPI
SecLinux32ElfGetImageInfo (
  IN EFI_PEI_ELF_LOADER_PROTOCOL          *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT         *ImageContext
  );

EFI_STATUS
EFIAPI
SecLinux32ElfLoadImage (
  IN EFI_PEI_ELF_LOADER_PROTOCOL          *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT         *ImageContext
  );

EFI_STATUS
EFIAPI
SecLinux32ElfRelocateImage (
  IN EFI_PEI_ELF_LOADER_PROTOCOL          *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT         *ImageContext
  );

EFI_STATUS
EFIAPI
SecLinux32ElfUnloadimage (
  IN EFI_PEI_ELF_LOADER_PROTOCOL      *This,
  IN EFI_PEI_ELF_LOADER_IMAGE_CONTEXT         *ImageContext
  );


typedef struct {
  EFI_PEI_ELF_LOADER_PROTOCOL Elf;
  VOID                            *ModHandle;
} EFI_PEI_ELF_LOADER_PROTOCOL_INSTANCE;

extern EFI_LINUX_THUNK_PROTOCOL  *gLinux;
