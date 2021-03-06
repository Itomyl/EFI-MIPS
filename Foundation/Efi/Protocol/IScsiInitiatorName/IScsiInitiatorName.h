/*++
  Copyright (c) 2006, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    IScsiInitatorName.h
    
Abstract: 
  EFI_ISCSI_INITIATOR_NAME_PROTOCOL as defined in UEFI 2.0.
  It rovides the ability to get and set the iSCSI Initiator Name.                                                  

Revision History

--*/

#ifndef __ISCSI_INITIATOR_NAME_H__
#define __ISCSI_INITIATOR_NAME_H__

#define EFI_ISCSI_INITIATOR_NAME_PROTOCOL_GUID \
{ \
  0xa6a72875, 0x2962, 0x4c18, {0x9f, 0x46, 0x8d, 0xa6, 0x44, 0xcc, 0xfe } \
}

typedef struct _EFI_ISCSI_INITIATOR_NAME_PROTOCOL EFI_ISCSI_INITIATOR_NAME_PROTOCOL;

typedef 
EFI_STATUS
(EFIAPI *EFI_ISCSI_INITIATOR_NAME_GET) (
  IN EFI_ISCSI_INITIATOR_NAME_PROTOCOL *This,
  IN OUT UINTN                         *BufferSize,
  OUT VOID                             *Buffer
  )
/*++

  Routine Description:
    Retrieves the current set value of iSCSI Initiator Name.
  
  Arguments:
    This         - Pointer to the EFI_ISCSI_INITIATOR_NAME_PROTOCOL instance.
    BufferSize   - Size of the buffer in bytes pointed to by Buffer / Actual size of the
                   variable data buffer.
    Buffer       - Pointer to the buffer for data to be read.

  Returns:
    EFI_SUCCESS           - Data was successfully retrieved into the provided buffer and the
                            BufferSize was sufficient to handle the iSCSI initiator name
    EFI_BUFFER_TOO_SMALL  - BufferSize is too small for the result.
    EFI_INVALID_PARAMETER - BufferSize or Buffer is NULL.
    EFI_DEVICE_ERROR      - The iSCSI initiator name could not be retrieved due to a hardware error.
    
--*/
;

  

typedef EFI_STATUS
(EFIAPI *EFI_ISCSI_INITIATOR_NAME_SET) (
  IN EFI_ISCSI_INITIATOR_NAME_PROTOCOL *This,
  IN OUT UINTN                         *BufferSize,
  IN VOID                              *Buffer
  )
/*++

  Routine Description:
    Sets the iSCSI Initiator Name.

  Arguments:
    This       - Pointer to the EFI_ISCSI_INITIATOR_NAME_PROTOCOL instance.
    BufferSize - Size of the buffer in bytes pointed to by Buffer.
    Buffer     - Pointer to the buffer for data to be written.

  Returns:
    EFI_SUCCESS           - Data was successfully stored by the protocol.
    EFI_UNSUPPORTED       - Platform policies do not allow for data to be written.
    EFI_INVALID_PARAMETER - BufferSize or Buffer is NULL, or BufferSize exceeds the maximum allowed limit.
    EFI_DEVICE_ERROR      - The data could not be stored due to a hardware error.
    EFI_OUT_OF_RESOURCES  - Not enough storage is available to hold the data.
    EFI_PROTOCOL_ERROR    - Input iSCSI initiator name does not adhere to RFC 3720
                            (and other related protocols)
    
--*/
;  

struct _EFI_ISCSI_INITIATOR_NAME_PROTOCOL {
  EFI_ISCSI_INITIATOR_NAME_GET         Get;
  EFI_ISCSI_INITIATOR_NAME_SET         Set;
};

extern EFI_GUID gEfiIScsiInitiatorNameProtocolGuid;

#endif
