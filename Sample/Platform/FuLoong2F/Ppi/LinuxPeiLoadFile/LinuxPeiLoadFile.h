/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 LinuxPeiLoadFile.h

Abstract:

  Linux Load File PPI.

  When the PEI core is done it calls the DXE IPL via PPI

--*/

#ifndef _LINUX_PEI_LOAD_FILE_H_
#define _LINUX_PEI_LOAD_FILE_H_

#include "Tiano.h"
#include "PeiHob.h"

#define LINUX_PEI_LOAD_FILE_GUID \
  { \
    0xfd0c65eb, 0x405, 0x4cd2, {0x8a, 0xee, 0xf4, 0x0, 0xef, 0x13, 0xba, 0xc2} \
  }

typedef
EFI_STATUS
(EFIAPI *LINUX_PEI_LOAD_FILE) (
  VOID                  *Pe32Data,
  EFI_PHYSICAL_ADDRESS  *ImageAddress,
  UINT64                *ImageSize,
  EFI_PHYSICAL_ADDRESS  *EntryPoint
  );

/*++

Routine Description:
  Loads and relocates a PE/COFF image into memory.

Arguments:
  Pe32Data         - The base address of the PE/COFF file that is to be loaded and relocated
  ImageAddress     - The base address of the relocated PE/COFF image
  ImageSize        - The size of the relocated PE/COFF image
  EntryPoint       - The entry point of the relocated PE/COFF image

Returns:
  EFI_SUCCESS   - The file was loaded and relocated
  EFI_OUT_OF_RESOURCES - There was not enough memory to load and relocate the PE/COFF file

--*/
typedef struct {
  LINUX_PEI_LOAD_FILE  PeiLoadFileService;
} LINUX_PEI_LOAD_FILE_PPI;

extern EFI_GUID gLinuxPeiLoadFileGuid;

#endif
