/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SystemLib.c

Abstract:

  This protocol for POSIX lib for Linux emulation envirnment.

--*/

#include "Tiano.h"

#include EFI_PROTOCOL_DEFINITION (SystemLib)

EFI_GUID gEfiSystemLibProtocolGuid = EFI_SYSTEM_LIB_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiSystemLibProtocolGuid, "EFI System Library", "POSIX API protocol");
