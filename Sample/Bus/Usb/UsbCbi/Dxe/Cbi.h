/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    cbi.h

Abstract:

    USB CBI transportation protocol definitions.
--*/

#ifndef _CBI_H
#define _CBI_H
//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (UsbAtapi)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define bit(a)                  (1 << (a))

#define MASS_STORAGE_CLASS      0x08
#define CBI0_INTERFACE_PROTOCOL 0x00
#define CBI1_INTERFACE_PROTOCOL 0x01

//
// in millisecond unit
//
#define STALL_1_SECOND          1000  

#pragma pack(1)
//
// Data block definition for transportation through interrupt endpoint
//
typedef struct {
  UINT8 bType;
  UINT8 bValue;
} INTERRUPT_DATA_BLOCK;

#pragma pack()

#define USB_CBI_DEVICE_SIGNATURE  EFI_SIGNATURE_32 ('u', 'c', 'b', 'i')

//
// Device structure for CBI, interrupt endpoint may be not used in
// CBI1 Protocol
//
typedef struct {
  UINT32                        Signature;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_USB_ATAPI_PROTOCOL        UsbAtapiProtocol;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   BulkInEndpointDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   BulkOutEndpointDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   InterruptEndpointDescriptor;
  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
} USB_CBI_DEVICE;

#define USB_CBI_DEVICE_FROM_THIS(a) \
    CR(a, USB_CBI_DEVICE, UsbAtapiProtocol, USB_CBI_DEVICE_SIGNATURE)

#endif
