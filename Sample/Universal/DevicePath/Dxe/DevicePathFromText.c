/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DevicePathFromText.c

Abstract:

  DevicePathFromText protocol as defined in the UEFI 2.0 specification.

--*/

#include "DevicePathDriver.h"

CHAR16 *
StrDuplicate (
  IN CONST CHAR16  *Src
  )
/*++

  Routine Description:
    Duplicate a string

  Arguments:
    Src - Source string

  Returns:
    Duplicated string

--*/
{
  UINTN   Length;
  CHAR16  *ReturnStr;

  Length = EfiStrLen ((CHAR16 *) Src);

  ReturnStr = EfiLibAllocateCopyPool ((Length + 1) * sizeof (CHAR16), (VOID *) Src);

  return ReturnStr;
}

CHAR16 *
GetParamByNodeName (
  IN CHAR16 *Str,
  IN CHAR16 *NodeName
  )
/*++

  Routine Description:
    Get parameter in a pair of parentheses follow the given node name.
    For example, given the "Pci(0,1)" and NodeName "Pci", it returns "0,1".

  Arguments:
    Str      - Device Path Text
    NodeName - Name of the node

  Returns:
    Parameter text for the node

--*/
{
  CHAR16  *ParamStr;
  CHAR16  *StrPointer;
  UINTN   NodeNameLength;
  UINTN   ParameterLength;

  //
  // Check whether the node name matchs
  //
  NodeNameLength = EfiStrLen (NodeName);
  if (EfiCompareMem (Str, NodeName, NodeNameLength * sizeof (CHAR16)) != 0) {
    return NULL;
  }

  ParamStr = Str + NodeNameLength;
  if (!IS_LEFT_PARENTH (*ParamStr)) {
    return NULL;
  }

  //
  // Skip the found '(' and find first occurrence of ')'
  //
  ParamStr++;
  ParameterLength = 0;
  StrPointer = ParamStr;
  while (!IS_NULL (*StrPointer)) {
    if (IS_RIGHT_PARENTH (*StrPointer)) {
      break;
    }
    StrPointer++;
    ParameterLength++;
  }
  if (IS_NULL (*StrPointer)) {
    //
    // ')' not found
    //
    return NULL;
  }

  ParamStr = EfiLibAllocateCopyPool ((ParameterLength + 1) * sizeof (CHAR16), ParamStr);
  if (ParamStr == NULL) {
    return NULL;
  }
  //
  // Terminate the parameter string
  //
  ParamStr[ParameterLength] = L'\0';

  return ParamStr;
}

CHAR16 *
SplitStr (
  IN OUT CHAR16 **List,
  IN     CHAR16 Separator
  )
/*++

  Routine Description:
    Get current sub-string from a string list, before return
    the list header is moved to next sub-string. The sub-string is separated
    by the specified character. For example, the separator is ',', the string
    list is "2,0,3", it returns "2", the remain list move to "2,3"

  Arguments:
    List       - A string list separated by the specified separator
    Separator  - The separator character

  Returns:
    pointer    - The current sub-string

--*/
{
  CHAR16  *Str;
  CHAR16  *ReturnStr;

  Str = *List;
  ReturnStr = Str;

  if (IS_NULL (*Str)) {
    return ReturnStr;
  }

  //
  // Find first occurrence of the separator
  //
  while (!IS_NULL (*Str)) {
    if (*Str == Separator) {
      break;
    }
    Str++;
  }

  if (*Str == Separator) {
    //
    // Find a sub-string, terminate it
    //
    *Str = L'\0';
    Str++;
  }

  //
  // Move to next sub-string
  //
  *List = Str;

  return ReturnStr;
}

CHAR16 *
GetNextParamStr (
  IN OUT CHAR16 **List
  )
{
  //
  // The separator is comma
  //
  return SplitStr (List, L',');
}

CHAR16 *
GetNextDeviceNodeStr (
  IN OUT CHAR16   **DevicePath,
  OUT    BOOLEAN  *IsInstanceEnd
  )
/*++

  Routine Description:
    Get one device node from entire device path text.

  Arguments:
    Str           - The entire device path text string
    IsInstanceEnd - This node is the end of a device path instance

  Returns:
    a pointer     - A device node text
    NULL          - No more device node available

--*/
{
  CHAR16  *Str;
  CHAR16  *ReturnStr;
  UINTN   ParenthesesStack;

  Str = *DevicePath;
  if (IS_NULL (*Str)) {
    return NULL;
  }

  //
  // Skip the leading '/', '(', ')' and ','
  //
  while (!IS_NULL (*Str)) {
    if (!IS_SLASH (*Str) &&
        !IS_COMMA (*Str) &&
        !IS_LEFT_PARENTH (*Str) &&
        !IS_RIGHT_PARENTH (*Str)) {
      break;
    }
    Str++;
  }

  ReturnStr = Str;

  //
  // Scan for the separator of this device node, '/' or ','
  //
  ParenthesesStack = 0;
  while (!IS_NULL (*Str)) {
    if ((IS_COMMA (*Str) || IS_SLASH (*Str)) && (ParenthesesStack == 0)) {
      break;
    }

    if (IS_LEFT_PARENTH (*Str)) {
      ParenthesesStack++;
    } else if (IS_RIGHT_PARENTH (*Str)) {
      ParenthesesStack--;
    }

    Str++;
  }

  if (ParenthesesStack != 0) {
    //
    // The '(' doesn't pair with ')', invalid device path text
    //
    return NULL;
  }

  if (IS_COMMA (*Str)) {
    *IsInstanceEnd = TRUE;
    *Str = L'\0';
    Str++;
  } else {
    *IsInstanceEnd = FALSE;
    if (!IS_NULL (*Str)) {
      *Str = L'\0';
      Str++;
    }
  }

  *DevicePath = Str;

  return ReturnStr;
}

CHAR16 *
TrimHexStr (
  IN CHAR16  *Str
  )
/*++

  Routine Description:
    Skip the leading white space and '0x' or '0X' of a hex string

  Arguments:
    Str  -  The hex string

  Returns:

--*/
{
  //
  // skip preceeding white space
  //
  while (*Str && *Str == ' ') {
    Str += 1;
  }
  //
  // skip preceeding zeros
  //
  while (*Str && *Str == '0') {
    Str += 1;
  }
  //
  // skip preceeding character 'x'
  //
  if (*Str && (*Str == 'x' || *Str == 'X')) {
    Str += 1;
  }

  return Str;
}

UINTN
Xtoi (
  IN CHAR16  *Str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  UINTN   Length;

  ASSERT (Str != NULL);

  //
  // convert hex digits
  //
  u = 0;
  Length = sizeof (UINTN);
  HexStringToBuf ((UINT8 *) &u, &Length, TrimHexStr (Str), NULL);

  return u;
}

VOID
Xtoi64 (
  IN CHAR16  *Str,
  IN UINT64  *Data
  )
/*++

Routine Description:

  Convert hex string to 64 bit data.

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN  Length;

  *Data = 0;
  Length = sizeof (UINT64);
  HexStringToBuf ((UINT8 *) Data, &Length, TrimHexStr (Str), NULL);
}

UINTN
Atoi (
  IN CHAR16  *str
  )
/*++

Routine Description:

  Convert decimal string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR16  c;
  UINTN   m;
  UINTN   n;

  ASSERT (str != NULL);

  m = (UINTN) -1 / 10;
  n = (UINTN) -1 % 10;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // convert digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= '0' && c <= '9') {
      if (u > m || u == m && c - '0' > (INTN) n) {
        return (UINTN) -1;
      }

      u = (u * 10) + c - '0';
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

EFI_STATUS 
StrToBuf (
  OUT UINT8    *Buf,
  IN  UINTN    BufferLength,
  IN  CHAR16   *Str
  )
{
  UINTN       Index;
  UINTN       StrLength;
  UINT8       Digit;
  UINT8       Byte;

  //
  // Two hex char make up one byte
  //
  StrLength = BufferLength * sizeof (CHAR16);

  for(Index = 0; Index < StrLength; Index++, Str++) {

    IsHexDigit (&Digit, *Str);

    //
    // For odd charaters, write the upper nibble for each buffer byte,
    // and for even characters, the lower nibble.
    //
    if ((Index & 1) == 0) {
      Byte = Digit << 4;
    } else {
      Byte = Buf[Index / 2];
      Byte &= 0xF0;
      Byte |= Digit;
    }

    Buf[Index / 2] = Byte;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
StrToGuid (
  IN  CHAR16   *Str,
  OUT EFI_GUID *Guid
  )
{
  UINTN       BufferLength;
  UINTN       ConvertedStrLen;
  EFI_STATUS  Status;

  BufferLength = sizeof (Guid->Data1);
  Status = HexStringToBuf ((UINT8 *) &Guid->Data1, &BufferLength, Str, &ConvertedStrLen);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Str += ConvertedStrLen;
  if (IS_HYPHEN (*Str)) {
    Str++;   
  } else {
    return EFI_UNSUPPORTED;
  }

  BufferLength = sizeof (Guid->Data2);
  Status = HexStringToBuf ((UINT8 *) &Guid->Data2, &BufferLength, Str, &ConvertedStrLen);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Str += ConvertedStrLen;
  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  BufferLength = sizeof (Guid->Data3);
  Status = HexStringToBuf ((UINT8 *) &Guid->Data3, &BufferLength, Str, &ConvertedStrLen);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Str += ConvertedStrLen;
  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  StrToBuf (&Guid->Data4[0], 2, Str);
  //
  // Skip 2 byte hex chars
  //
  Str += 2 * 2;

  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }
  StrToBuf (&Guid->Data4[2], 6, Str);

  return EFI_SUCCESS;
}

VOID
StrToIPv4Addr (
  IN OUT CHAR16           **Str,
  OUT    EFI_IPv4_ADDRESS *IPv4Addr
  )
{
  UINTN  Index;

  for (Index = 0; Index < 4; Index++) {
    IPv4Addr->Addr[Index] = (UINT8) Atoi (SplitStr (Str, L'.'));
  }
}

VOID
StrToIPv6Addr (
  IN OUT CHAR16           **Str,
  OUT    EFI_IPv6_ADDRESS *IPv6Addr
  )
{
  UINTN  Index;
  UINT16 Data;

  for (Index = 0; Index < 8; Index++) {
    Data = (UINT16) Xtoi (SplitStr (Str, L':'));
    IPv6Addr->Addr[Index * 2] = (UINT8) (Data >> 8);
    IPv6Addr->Addr[Index * 2 + 1] = (UINT8) (Data & 0xff);
  }
}

VOID
StrToAscii (
  IN     CHAR16 *Str,
  IN OUT CHAR8  **AsciiStr
  )
{
  CHAR8 *Dest;

  Dest = *AsciiStr;
  while (!IS_NULL (*Str)) {
    *(Dest++) = (CHAR8) *(Str++);
  }
  *Dest = 0;

  //
  // Return the string next to it
  //
  *AsciiStr = Dest + 1;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextPci (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16          *FunctionStr;
  CHAR16          *DeviceStr;
  PCI_DEVICE_PATH *Pci;

  FunctionStr = GetNextParamStr (&TextDeviceNode);
  DeviceStr   = GetNextParamStr (&TextDeviceNode);
  Pci         = (PCI_DEVICE_PATH *) CreateDeviceNode (
                                      HARDWARE_DEVICE_PATH,
                                      HW_PCI_DP,
                                      sizeof (PCI_DEVICE_PATH)
                                      );

  Pci->Function = (UINT8) Xtoi (FunctionStr);
  Pci->Device   = (UINT8) Xtoi (DeviceStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Pci;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextPcCard (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16              *FunctionNumberStr;
  PCCARD_DEVICE_PATH  *Pccard;

  FunctionNumberStr = GetNextParamStr (&TextDeviceNode);
  Pccard            = (PCCARD_DEVICE_PATH *) CreateDeviceNode (
                                               HARDWARE_DEVICE_PATH,
                                               HW_PCCARD_DP,
                                               sizeof (PCCARD_DEVICE_PATH)
                                               );

  Pccard->FunctionNumber  = (UINT8) Xtoi (FunctionNumberStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Pccard;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextMemoryMapped (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16              *StartingAddressStr;
  CHAR16              *EndingAddressStr;
  MEMMAP_DEVICE_PATH  *MemMap;

  StartingAddressStr = GetNextParamStr (&TextDeviceNode);
  EndingAddressStr   = GetNextParamStr (&TextDeviceNode);
  MemMap             = (MEMMAP_DEVICE_PATH *) CreateDeviceNode (
                                               HARDWARE_DEVICE_PATH,
                                               HW_MEMMAP_DP,
                                               sizeof (MEMMAP_DEVICE_PATH)
                                               );

  MemMap->MemoryType  = 0;

  Xtoi64 (StartingAddressStr, &MemMap->StartingAddress);
  Xtoi64 (EndingAddressStr, &MemMap->EndingAddress);

  return (EFI_DEVICE_PATH_PROTOCOL *) MemMap;
}

EFI_DEVICE_PATH_PROTOCOL *
ConvertFromTextVendor (
  IN CHAR16 *TextDeviceNode,
  IN UINT8  Type,
  IN UINT8  SubType
  )
{
  CHAR16              *GuidStr;
  CHAR16              *DataStr;
  UINTN               Length;
  VENDOR_DEVICE_PATH  *Vendor;

  GuidStr = GetNextParamStr (&TextDeviceNode);

  DataStr = GetNextParamStr (&TextDeviceNode);
  Length = EfiStrLen (DataStr);
  //
  // Two hex characters make up 1 buffer byte
  //
  Length = (Length + 1) / 2;

  Vendor = (VENDOR_DEVICE_PATH *) CreateDeviceNode (
                                    Type,
                                    SubType,
                                    sizeof (VENDOR_DEVICE_PATH) + (UINT16) Length
                                    );

  StrToGuid (GuidStr, &Vendor->Guid);
  StrToBuf (((UINT8 *) Vendor) + sizeof (VENDOR_DEVICE_PATH), Length, DataStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Vendor;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenHw (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextVendor (
           TextDeviceNode,
           HARDWARE_DEVICE_PATH,
           HW_VENDOR_DP
           );
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextCtrl (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                  *ControllerStr;
  CONTROLLER_DEVICE_PATH  *Controller;

  ControllerStr = GetNextParamStr (&TextDeviceNode);
  Controller    = (CONTROLLER_DEVICE_PATH *) CreateDeviceNode (
                                               HARDWARE_DEVICE_PATH,
                                               HW_CONTROLLER_DP,
                                               sizeof (CONTROLLER_DEVICE_PATH)
                                               );
  Controller->Controller  = (UINT32) Xtoi (ControllerStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Controller;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextAcpi (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                *HIDStr;
  CHAR16                *UIDStr;
  ACPI_HID_DEVICE_PATH  *Acpi;

  HIDStr = GetNextParamStr (&TextDeviceNode);
  UIDStr = GetNextParamStr (&TextDeviceNode);
  Acpi   = (ACPI_HID_DEVICE_PATH *) CreateDeviceNode (
                                      ACPI_DEVICE_PATH,
                                      ACPI_DP,
                                      sizeof (ACPI_HID_DEVICE_PATH)
                                      );

  if ((HIDStr[0] == L'P') && (HIDStr[1] == L'N') && (HIDStr[2] == L'P')) {
    HIDStr += 3;
  }

  Acpi->HID = EISA_PNP_ID (Xtoi (HIDStr));
  Acpi->UID = (UINT32) Xtoi (UIDStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Acpi;
}

EFI_DEVICE_PATH_PROTOCOL *
ConvertFromTextAcpi (
  IN CHAR16 *TextDeviceNode,
  IN UINT32  Hid
  )
{
  CHAR16                *UIDStr;
  ACPI_HID_DEVICE_PATH  *Acpi;

  UIDStr = GetNextParamStr (&TextDeviceNode);
  Acpi   = (ACPI_HID_DEVICE_PATH *) CreateDeviceNode (
                                      ACPI_DEVICE_PATH,
                                      ACPI_DP,
                                      sizeof (ACPI_HID_DEVICE_PATH)
                                      );

  Acpi->HID = Hid;
  Acpi->UID = (UINT32) Xtoi (UIDStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Acpi;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextPciRoot (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextAcpi (TextDeviceNode, 0x0a0341d0);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextFloppy (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextAcpi (TextDeviceNode, 0x060441d0);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextKeyboard (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextAcpi (TextDeviceNode, 0x030141d0);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextSerial (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextAcpi (TextDeviceNode, 0x050141d0);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextParallelPort (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextAcpi (TextDeviceNode, 0x040141d0);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextAcpiEx (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                                  *HIDStr;
  CHAR16                                  *CIDStr;
  CHAR16                                  *UIDStr;
  CHAR16                                  *HIDSTRStr;
  CHAR16                                  *CIDSTRStr;
  CHAR16                                  *UIDSTRStr;
  CHAR8                                   *AsciiStr;
  UINT16                                  Length;
  ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR  *AcpiExt;

  HIDStr    = GetNextParamStr (&TextDeviceNode);
  CIDStr    = GetNextParamStr (&TextDeviceNode);
  UIDStr    = GetNextParamStr (&TextDeviceNode);
  HIDSTRStr = GetNextParamStr (&TextDeviceNode);
  CIDSTRStr = GetNextParamStr (&TextDeviceNode);
  UIDSTRStr = GetNextParamStr (&TextDeviceNode);
  Length    = sizeof (ACPI_EXTENDED_HID_DEVICE_PATH) + 
                  (UINT16) EfiStrLen (HIDSTRStr) + 1 +
                  (UINT16) EfiStrLen (UIDSTRStr) + 1 +
                  (UINT16) EfiStrLen (CIDSTRStr) + 1;
  AcpiExt = (ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR *) CreateDeviceNode (
                                                         ACPI_DEVICE_PATH,
                                                         ACPI_EXTENDED_DP,
                                                         Length
                                                         );

  if ((HIDStr[0] == L'P') && (HIDStr[1] == L'N') && (HIDStr[2] == L'P')) {
    HIDStr += 3;
    AcpiExt->HID = EISA_PNP_ID (Xtoi (HIDStr));
  } else {
    AcpiExt->HID = (UINT32) Xtoi (HIDStr);
  }

  AcpiExt->UID  = (UINT32) Xtoi (UIDStr);
  AcpiExt->CID  = (UINT32) Xtoi (CIDStr);

  AsciiStr = AcpiExt->HidUidCidStr;
  StrToAscii (HIDSTRStr, &AsciiStr);
  StrToAscii (UIDSTRStr, &AsciiStr);
  StrToAscii (CIDSTRStr, &AsciiStr);
  
  return (EFI_DEVICE_PATH_PROTOCOL *) AcpiExt;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextAcpiExp (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                                  *HIDStr;
  CHAR16                                  *CIDStr;
  CHAR16                                  *UIDSTRStr;
  CHAR8                                   *AsciiStr;
  UINT16                                  Length;
  ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR  *AcpiExt;

  HIDStr    = GetNextParamStr (&TextDeviceNode);
  CIDStr    = GetNextParamStr (&TextDeviceNode);
  UIDSTRStr = GetNextParamStr (&TextDeviceNode);
  Length    = sizeof (ACPI_EXTENDED_HID_DEVICE_PATH) + (UINT16) EfiStrLen (UIDSTRStr) + 3;
  AcpiExt   = (ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR *) CreateDeviceNode (
                                                           ACPI_DEVICE_PATH,
                                                           ACPI_EXTENDED_DP,
                                                           Length
                                                           );

  if ((HIDStr[0] == L'P') && (HIDStr[1] == L'N') && (HIDStr[2] == L'P')) {
    HIDStr += 3;
    AcpiExt->HID = EISA_PNP_ID (Xtoi (HIDStr));
  } else {
    AcpiExt->HID = (UINT32) Xtoi (HIDStr);
  }

  AcpiExt->UID = 0;
  AcpiExt->CID = (UINT32) Xtoi (CIDStr);

  AsciiStr = AcpiExt->HidUidCidStr;
  //
  // HID string is NULL
  //
  *AsciiStr = 0;
  //
  // Convert UID string
  //
  AsciiStr++;
  StrToAscii (UIDSTRStr, &AsciiStr);
  //
  // CID string is NULL
  //
  *AsciiStr = 0;

  return (EFI_DEVICE_PATH_PROTOCOL *) AcpiExt;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextAta (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *PrimarySecondaryStr;
  CHAR16            *SlaveMasterStr;
  CHAR16            *LunStr;
  ATAPI_DEVICE_PATH *Atapi;

  Atapi = (ATAPI_DEVICE_PATH *) CreateDeviceNode (
                                  MESSAGING_DEVICE_PATH,
                                  MSG_ATAPI_DP,
                                  sizeof (ATAPI_DEVICE_PATH)
                                  );

  PrimarySecondaryStr     = GetNextParamStr (&TextDeviceNode);
  SlaveMasterStr          = GetNextParamStr (&TextDeviceNode);
  LunStr                  = GetNextParamStr (&TextDeviceNode);

  Atapi->PrimarySecondary = (EfiStrCmp (PrimarySecondaryStr, L"Primary") == 0) ? (UINT8) 0 : (UINT8) 1;
  Atapi->SlaveMaster      = (EfiStrCmp (SlaveMasterStr, L"Master") == 0) ? (UINT8) 0 : (UINT8) 1;
  Atapi->Lun              = (UINT16) Xtoi (LunStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Atapi;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextScsi (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *PunStr;
  CHAR16            *LunStr;
  SCSI_DEVICE_PATH  *Scsi;

  PunStr = GetNextParamStr (&TextDeviceNode);
  LunStr = GetNextParamStr (&TextDeviceNode);
  Scsi   = (SCSI_DEVICE_PATH *) CreateDeviceNode (
                                   MESSAGING_DEVICE_PATH,
                                   MSG_SCSI_DP,
                                   sizeof (SCSI_DEVICE_PATH)
                                   );

  Scsi->Pun = (UINT16) Xtoi (PunStr);
  Scsi->Lun = (UINT16) Xtoi (LunStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Scsi;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextFibre (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                    *WWNStr;
  CHAR16                    *LunStr;
  FIBRECHANNEL_DEVICE_PATH  *Fibre;

  WWNStr = GetNextParamStr (&TextDeviceNode);
  LunStr = GetNextParamStr (&TextDeviceNode);
  Fibre  = (FIBRECHANNEL_DEVICE_PATH *) CreateDeviceNode (
                                   MESSAGING_DEVICE_PATH,
                                   MSG_FIBRECHANNEL_DP,
                                   sizeof (FIBRECHANNEL_DEVICE_PATH)
                                   );

  Fibre->Reserved = 0;
  Xtoi64 (WWNStr, &Fibre->WWN);
  Xtoi64 (LunStr, &Fibre->Lun);

  return (EFI_DEVICE_PATH_PROTOCOL *) Fibre;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromText1394 (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *GuidStr;
  F1394_DEVICE_PATH *F1394;

  GuidStr = GetNextParamStr (&TextDeviceNode);
  F1394  = (F1394_DEVICE_PATH *) CreateDeviceNode (
                                   MESSAGING_DEVICE_PATH,
                                   MSG_1394_DP,
                                   sizeof (F1394_DEVICE_PATH)
                                   );

  F1394->Reserved = 0;
  Xtoi64 (GuidStr, &F1394->Guid);

  return (EFI_DEVICE_PATH_PROTOCOL *) F1394;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsb (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16          *PortStr;
  CHAR16          *InterfaceStr;
  USB_DEVICE_PATH *Usb;

  PortStr               = GetNextParamStr (&TextDeviceNode);
  InterfaceStr          = GetNextParamStr (&TextDeviceNode);
  Usb = (USB_DEVICE_PATH *) CreateDeviceNode (
                                MESSAGING_DEVICE_PATH,
                                MSG_USB_DP,
                                sizeof (USB_DEVICE_PATH)
                                );

  Usb->ParentPortNumber = (UINT8) Xtoi (PortStr);
  Usb->InterfaceNumber  = (UINT8) Xtoi (InterfaceStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Usb;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextI2O (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16          *TIDStr;
  I2O_DEVICE_PATH *I2O;

  TIDStr    = GetNextParamStr (&TextDeviceNode);
  I2O = (I2O_DEVICE_PATH *) CreateDeviceNode (
                                MESSAGING_DEVICE_PATH,
                                MSG_I2O_DP,
                                sizeof (I2O_DEVICE_PATH)
                                );

  I2O->Tid  = (UINT32) Xtoi (TIDStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) I2O;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextInfiniband (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                  *FlagsStr;
  CHAR16                  *GuidStr;
  CHAR16                  *SidStr;
  CHAR16                  *TidStr;
  CHAR16                  *DidStr;
  EFI_GUID                PortGid;
  INFINIBAND_DEVICE_PATH  *InfiniBand;

  FlagsStr   = GetNextParamStr (&TextDeviceNode);
  GuidStr    = GetNextParamStr (&TextDeviceNode);
  SidStr     = GetNextParamStr (&TextDeviceNode);
  TidStr     = GetNextParamStr (&TextDeviceNode);
  DidStr     = GetNextParamStr (&TextDeviceNode);
  InfiniBand = (INFINIBAND_DEVICE_PATH *) CreateDeviceNode (
                                MESSAGING_DEVICE_PATH,
                                MSG_INFINIBAND_DP,
                                sizeof (INFINIBAND_DEVICE_PATH)
                                );

  InfiniBand->ResourceFlags = (UINT32) Xtoi (FlagsStr);
  StrToGuid (GuidStr, &PortGid);
  EfiCopyMem (InfiniBand->PortGid, &PortGid, sizeof (EFI_GUID));
  Xtoi64 (SidStr, &InfiniBand->ServiceId);
  Xtoi64 (TidStr, &InfiniBand->TargetPortId);
  Xtoi64 (DidStr, &InfiniBand->DeviceId);

  return (EFI_DEVICE_PATH_PROTOCOL *) InfiniBand;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenMsg (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextVendor (
            TextDeviceNode,
            MESSAGING_DEVICE_PATH,
            MSG_VENDOR_DP
            );
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenPcAnsi (
  IN CHAR16 *TextDeviceNode
  )
{
  VENDOR_DEVICE_PATH  *Vendor;

  Vendor = (VENDOR_DEVICE_PATH *) CreateDeviceNode (
                                    MESSAGING_DEVICE_PATH,
                                    MSG_VENDOR_DP,
                                    sizeof (VENDOR_DEVICE_PATH));
  Vendor->Guid = gEfiPcAnsiGuid;

  return (EFI_DEVICE_PATH_PROTOCOL *) Vendor;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenVt100 (
  IN CHAR16 *TextDeviceNode
  )
{
  VENDOR_DEVICE_PATH  *Vendor;

  Vendor = (VENDOR_DEVICE_PATH *) CreateDeviceNode (
                                    MESSAGING_DEVICE_PATH,
                                    MSG_VENDOR_DP,
                                    sizeof (VENDOR_DEVICE_PATH));
  Vendor->Guid = gEfiVT100Guid;

  return (EFI_DEVICE_PATH_PROTOCOL *) Vendor;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenVt100Plus (
  IN CHAR16 *TextDeviceNode
  )
{
  VENDOR_DEVICE_PATH  *Vendor;

  Vendor = (VENDOR_DEVICE_PATH *) CreateDeviceNode (
                                    MESSAGING_DEVICE_PATH,
                                    MSG_VENDOR_DP,
                                    sizeof (VENDOR_DEVICE_PATH));
  Vendor->Guid = gEfiVT100PlusGuid;

  return (EFI_DEVICE_PATH_PROTOCOL *) Vendor;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenUtf8 (
  IN CHAR16 *TextDeviceNode
  )
{
  VENDOR_DEVICE_PATH  *Vendor;

  Vendor = (VENDOR_DEVICE_PATH *) CreateDeviceNode (
                                    MESSAGING_DEVICE_PATH,
                                    MSG_VENDOR_DP,
                                    sizeof (VENDOR_DEVICE_PATH));
  Vendor->Guid = gEfiVTUTF8Guid;

  return (EFI_DEVICE_PATH_PROTOCOL *) Vendor;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUartFlowCtrl (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                        *ValueStr;
  UART_FLOW_CONTROL_DEVICE_PATH *UartFlowControl;

  ValueStr = GetNextParamStr (&TextDeviceNode);
  UartFlowControl = (UART_FLOW_CONTROL_DEVICE_PATH *) CreateDeviceNode (
                                                        MESSAGING_DEVICE_PATH,
                                                        MSG_VENDOR_DP,
                                                        sizeof (UART_FLOW_CONTROL_DEVICE_PATH)
                                                        );

  UartFlowControl->Guid = mEfiDevicePathMessagingUartFlowControlGuid;
  if (EfiStrCmp (ValueStr, L"XonXoff") == 0) {
    UartFlowControl->FlowControlMap = 2;
  } else if (EfiStrCmp (ValueStr, L"Hardware") == 0) {
    UartFlowControl->FlowControlMap = 1;
  } else {
    UartFlowControl->FlowControlMap = 0;

  }

  return (EFI_DEVICE_PATH_PROTOCOL *) UartFlowControl;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextSAS (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16          *AddressStr;
  CHAR16          *LunStr;
  CHAR16          *RTPStr;
  CHAR16          *SASSATAStr;
  CHAR16          *LocationStr;
  CHAR16          *ConnectStr;
  CHAR16          *DriveBayStr;
  CHAR16          *ReservedStr;
  UINT16          Info;
  SAS_DEVICE_PATH *Sas;

  AddressStr  = GetNextParamStr (&TextDeviceNode);
  LunStr      = GetNextParamStr (&TextDeviceNode);
  RTPStr      = GetNextParamStr (&TextDeviceNode);
  SASSATAStr  = GetNextParamStr (&TextDeviceNode);
  LocationStr = GetNextParamStr (&TextDeviceNode);
  ConnectStr  = GetNextParamStr (&TextDeviceNode);
  DriveBayStr = GetNextParamStr (&TextDeviceNode);
  ReservedStr = GetNextParamStr (&TextDeviceNode);
  Info        = 0x0000;
  Sas         = (SAS_DEVICE_PATH *) CreateDeviceNode (
                                       MESSAGING_DEVICE_PATH,
                                       MSG_VENDOR_DP,
                                       sizeof (SAS_DEVICE_PATH)
                                       );

  Sas->Guid         = mEfiDevicePathMessagingSASGuid;
  Xtoi64 (AddressStr, &Sas->SasAddress);
  Xtoi64 (LunStr, &Sas->Lun);
  Sas->RelativeTargetPort = (UINT16) Xtoi (RTPStr);
  if (EfiStrCmp (SASSATAStr, L"NoTopology") == 0)
    ;
  else {
    if (EfiStrCmp (DriveBayStr, L"0") == 0) {
      Info |= 0x0001;
    } else {
      Info |= 0x0002;
      Info |= (Xtoi (DriveBayStr) << 8);
    }

    if (EfiStrCmp (SASSATAStr, L"SATA") == 0) {
      Info |= 0x0010;
    }

    if (EfiStrCmp (LocationStr, L"External") == 0) {
      Info |= 0x0020;
    }

    if (EfiStrCmp (ConnectStr, L"Expanded") == 0) {
      Info |= 0x0040;
    }
  }

  Sas->DeviceTopology = Info;
  Sas->Reserved       = (UINT32) Xtoi (ReservedStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Sas;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextDebugPort (
  IN CHAR16 *TextDeviceNode
  )
{
  VENDOR_DEFINED_MESSAGING_DEVICE_PATH  *Vend;

  Vend = (VENDOR_DEFINED_MESSAGING_DEVICE_PATH *) CreateDeviceNode (
                                                    MESSAGING_DEVICE_PATH,
                                                    MSG_VENDOR_DP,
                                                    sizeof (VENDOR_DEFINED_MESSAGING_DEVICE_PATH)
                                                    );

  Vend->Guid = gEfiDebugPortProtocolGuid;

  return (EFI_DEVICE_PATH_PROTOCOL *) Vend;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextMAC (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                *AddressStr;
  CHAR16                *IfTypeStr;
  UINTN                 Length;
  MAC_ADDR_DEVICE_PATH  *MAC;

  AddressStr    = GetNextParamStr (&TextDeviceNode);
  IfTypeStr     = GetNextParamStr (&TextDeviceNode);
  MAC           = (MAC_ADDR_DEVICE_PATH *) CreateDeviceNode (
                                              MESSAGING_DEVICE_PATH,
                                              MSG_MAC_ADDR_DP,
                                              sizeof (MAC_ADDR_DEVICE_PATH)
                                              );

  MAC->IfType   = (UINT8) Xtoi (IfTypeStr);

  Length = sizeof (EFI_MAC_ADDRESS);
  StrToBuf (&MAC->MacAddress.Addr[0], Length, AddressStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) MAC;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextIPv4 (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *RemoteIPStr;
  CHAR16            *ProtocolStr;
  CHAR16            *TypeStr;
  CHAR16            *LocalIPStr;
  IPv4_DEVICE_PATH  *IPv4;

  RemoteIPStr           = GetNextParamStr (&TextDeviceNode);
  ProtocolStr           = GetNextParamStr (&TextDeviceNode);
  TypeStr               = GetNextParamStr (&TextDeviceNode);
  LocalIPStr            = GetNextParamStr (&TextDeviceNode);
  IPv4                  = (IPv4_DEVICE_PATH *) CreateDeviceNode (
                                                 MESSAGING_DEVICE_PATH,
                                                 MSG_IPv4_DP,
                                                 sizeof (IPv4_DEVICE_PATH)
                                                 );

  StrToIPv4Addr (&RemoteIPStr, &IPv4->RemoteIpAddress);
  IPv4->Protocol = (EfiStrCmp (ProtocolStr, L"UDP") == 0) ? (UINT16) 0 : (UINT16) 1;
  if (EfiStrCmp (TypeStr, L"Static") == 0) {
    IPv4->StaticIpAddress = TRUE;
  } else {
    IPv4->StaticIpAddress = FALSE;
  }

  StrToIPv4Addr (&LocalIPStr, &IPv4->LocalIpAddress);

  IPv4->LocalPort       = 0;
  IPv4->RemotePort      = 0;

  return (EFI_DEVICE_PATH_PROTOCOL *) IPv4;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextIPv6 (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *RemoteIPStr;
  CHAR16            *ProtocolStr;
  CHAR16            *TypeStr;
  CHAR16            *LocalIPStr;
  IPv6_DEVICE_PATH  *IPv6;

  RemoteIPStr           = GetNextParamStr (&TextDeviceNode);
  ProtocolStr           = GetNextParamStr (&TextDeviceNode);
  TypeStr               = GetNextParamStr (&TextDeviceNode);
  LocalIPStr            = GetNextParamStr (&TextDeviceNode);
  IPv6                  = (IPv6_DEVICE_PATH *) CreateDeviceNode (
                                                 MESSAGING_DEVICE_PATH,
                                                 MSG_IPv6_DP,
                                                 sizeof (IPv6_DEVICE_PATH)
                                                 );

  StrToIPv6Addr (&RemoteIPStr, &IPv6->RemoteIpAddress);
  IPv6->Protocol        = (EfiStrCmp (ProtocolStr, L"UDP") == 0) ? (UINT16) 0 : (UINT16) 1;
  if (EfiStrCmp (TypeStr, L"Static") == 0) {
    IPv6->StaticIpAddress = TRUE;
  } else {
    IPv6->StaticIpAddress = FALSE;
  }

  StrToIPv6Addr (&LocalIPStr, &IPv6->LocalIpAddress);

  IPv6->LocalPort       = 0;
  IPv6->RemotePort      = 0;

  return (EFI_DEVICE_PATH_PROTOCOL *) IPv6;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUart (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *BaudStr;
  CHAR16            *DataBitsStr;
  CHAR16            *ParityStr;
  CHAR16            *StopBitsStr;
  UART_DEVICE_PATH  *Uart;

  BaudStr         = GetNextParamStr (&TextDeviceNode);
  DataBitsStr     = GetNextParamStr (&TextDeviceNode);
  ParityStr       = GetNextParamStr (&TextDeviceNode);
  StopBitsStr     = GetNextParamStr (&TextDeviceNode);
  Uart            = (UART_DEVICE_PATH *) CreateDeviceNode (
                                           MESSAGING_DEVICE_PATH,
                                           MSG_UART_DP,
                                           sizeof (UART_DEVICE_PATH)
                                           );

  Uart->BaudRate  = (EfiStrCmp (BaudStr, L"DEFAULT") == 0) ? 115200 : Atoi (BaudStr);
  Uart->DataBits  = (EfiStrCmp (DataBitsStr, L"DEFAULT") == 0) ? (UINT8) 8 : (UINT8) Atoi (DataBitsStr);
  switch (*ParityStr) {
  case L'D':
    Uart->Parity = 0;
    break;

  case L'N':
    Uart->Parity = 1;
    break;

  case L'E':
    Uart->Parity = 2;
    break;

  case L'O':
    Uart->Parity = 3;
    break;

  case L'M':
    Uart->Parity = 4;
    break;

  case L'S':
    Uart->Parity = 5;

  default:
    Uart->Parity = 0xff;
  }

  if (EfiStrCmp (StopBitsStr, L"D") == 0) {
    Uart->StopBits = (UINT8) 0;
  } else if (EfiStrCmp (StopBitsStr, L"1") == 0) {
    Uart->StopBits = (UINT8) 1;
  } else if (EfiStrCmp (StopBitsStr, L"1.5") == 0) {
    Uart->StopBits = (UINT8) 2;
  } else if (EfiStrCmp (StopBitsStr, L"2") == 0) {
    Uart->StopBits = (UINT8) 3;
  } else {
    Uart->StopBits = 0xff;

  }

  return (EFI_DEVICE_PATH_PROTOCOL *) Uart;
}

EFI_DEVICE_PATH_PROTOCOL *
ConvertFromTextUsbClass (
  IN CHAR16         *TextDeviceNode,
  IN USB_CLASS_TEXT *UsbClassText
  )
{
  CHAR16                *VIDStr;
  CHAR16                *PIDStr;
  CHAR16                *ClassStr;
  CHAR16                *SubClassStr;
  CHAR16                *ProtocolStr;
  USB_CLASS_DEVICE_PATH *UsbClass;

  UsbClass    = (USB_CLASS_DEVICE_PATH *) CreateDeviceNode (
                                            MESSAGING_DEVICE_PATH,
                                            MSG_USB_CLASS_DP,
                                            sizeof (USB_CLASS_DEVICE_PATH)
                                            );

  VIDStr      = GetNextParamStr (&TextDeviceNode);
  PIDStr      = GetNextParamStr (&TextDeviceNode);
  if (UsbClassText->ClassExist) {
    ClassStr = GetNextParamStr (&TextDeviceNode);
    UsbClass->DeviceClass = (UINT8) Xtoi (ClassStr);
  } else {
    UsbClass->DeviceClass = UsbClassText->Class;
  }
  if (UsbClassText->SubClassExist) {
    SubClassStr = GetNextParamStr (&TextDeviceNode);
    UsbClass->DeviceSubClass = (UINT8) Xtoi (SubClassStr);
  } else {
    UsbClass->DeviceSubClass = UsbClassText->SubClass;
  }  

  ProtocolStr = GetNextParamStr (&TextDeviceNode);

  UsbClass->VendorId        = (UINT16) Xtoi (VIDStr);
  UsbClass->ProductId       = (UINT16) Xtoi (PIDStr);
  UsbClass->DeviceProtocol  = (UINT8) Xtoi (ProtocolStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) UsbClass;
}


EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbClass (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = TRUE;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbAudio (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_AUDIO;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbCDCControl (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_CDCCONTROL;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbHID (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_HID;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbImage (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_IMAGE;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbPrinter (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_PRINTER;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbMassStorage (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_MASS_STORAGE;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbHub (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_HUB;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbCDCData (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_CDCDATA;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbSmartCard (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_SMART_CARD;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbVideo (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_VIDEO;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbDiagnostic (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_DIAGNOSTIC;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbWireless (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_WIRELESS;
  UsbClassText.SubClassExist = TRUE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbDeviceFirmwareUpdate (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_RESERVE;
  UsbClassText.SubClassExist = FALSE;
  UsbClassText.SubClass      = USB_SUBCLASS_FW_UPDATE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbIrdaBridge (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_RESERVE;
  UsbClassText.SubClassExist = FALSE;
  UsbClassText.SubClass      = USB_SUBCLASS_IRDA_BRIDGE;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbTestAndMeasurement (
  IN CHAR16 *TextDeviceNode
  )
{
  USB_CLASS_TEXT  UsbClassText;

  UsbClassText.ClassExist    = FALSE;
  UsbClassText.Class         = USB_CLASS_RESERVE;
  UsbClassText.SubClassExist = FALSE;
  UsbClassText.SubClass      = USB_SUBCLASS_TEST;

  return ConvertFromTextUsbClass (TextDeviceNode, &UsbClassText);
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUsbWwid (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                *VIDStr;
  CHAR16                *PIDStr;
  CHAR16                *InterfaceNumStr;
  USB_WWID_DEVICE_PATH  *UsbWwid;

  VIDStr                    = GetNextParamStr (&TextDeviceNode);
  PIDStr                    = GetNextParamStr (&TextDeviceNode);
  InterfaceNumStr           = GetNextParamStr (&TextDeviceNode);
  UsbWwid                   = (USB_WWID_DEVICE_PATH *) CreateDeviceNode (
                                                         MESSAGING_DEVICE_PATH,
                                                         MSG_USB_WWID_DP,
                                                         sizeof (USB_WWID_DEVICE_PATH)
                                                         );

  UsbWwid->VendorId         = (UINT16) Xtoi (VIDStr);
  UsbWwid->ProductId        = (UINT16) Xtoi (PIDStr);
  UsbWwid->InterfaceNumber  = (UINT16) Xtoi (InterfaceNumStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) UsbWwid;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextUnit (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                          *LunStr;
  DEVICE_LOGICAL_UNIT_DEVICE_PATH *LogicalUnit;

  LunStr      = GetNextParamStr (&TextDeviceNode);
  LogicalUnit = (DEVICE_LOGICAL_UNIT_DEVICE_PATH *) CreateDeviceNode (
                                                      MESSAGING_DEVICE_PATH,
                                                      MSG_DEVICE_LOGICAL_UNIT_DP,
                                                      sizeof (DEVICE_LOGICAL_UNIT_DEVICE_PATH)
                                                      );

  LogicalUnit->Lun  = (UINT8) Xtoi (LunStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) LogicalUnit;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextiSCSI (
  IN CHAR16 *TextDeviceNode
  )
{
  UINT16                      Options;
  CHAR16                      *NameStr;
  CHAR16                      *PortalGroupStr;
  CHAR16                      *LunStr;
  CHAR16                      *HeaderDigestStr;
  CHAR16                      *DataDigestStr;
  CHAR16                      *AuthenticationStr;
  CHAR16                      *ProtocolStr;
  ISCSI_DEVICE_PATH_WITH_NAME *iSCSI;

  NameStr           = GetNextParamStr (&TextDeviceNode);
  PortalGroupStr    = GetNextParamStr (&TextDeviceNode);
  LunStr            = GetNextParamStr (&TextDeviceNode);
  HeaderDigestStr   = GetNextParamStr (&TextDeviceNode);
  DataDigestStr     = GetNextParamStr (&TextDeviceNode);
  AuthenticationStr = GetNextParamStr (&TextDeviceNode);
  ProtocolStr       = GetNextParamStr (&TextDeviceNode);
  iSCSI             = (ISCSI_DEVICE_PATH_WITH_NAME *) CreateDeviceNode (
                                                        MESSAGING_DEVICE_PATH,
                                                        MSG_ISCSI_DP,
                                                        sizeof (ISCSI_DEVICE_PATH_WITH_NAME) + (UINT16) (EfiStrLen (NameStr) * 2)
                                                        );

  EfiStrCpy (iSCSI->iSCSITargetName, NameStr);
  iSCSI->TargetPortalGroupTag = (UINT16) Xtoi (PortalGroupStr);
  Xtoi64 (LunStr, &iSCSI->Lun);

  Options = 0x0000;
  if (EfiStrCmp (HeaderDigestStr, L"CRC32C") == 0) {
    Options |= 0x0002;
  }

  if (EfiStrCmp (DataDigestStr, L"CRC32C") == 0) {
    Options |= 0x0008;
  }

  if (EfiStrCmp (AuthenticationStr, L"None") == 0) {
    Options |= 0x0800;
  }

  if (EfiStrCmp (AuthenticationStr, L"CHAP_UNI") == 0) {
    Options |= 0x1000;
  }

  iSCSI->LoginOption      = (UINT16) Options;

  iSCSI->NetworkProtocol  = (UINT16) EfiStrCmp (ProtocolStr, L"TCP");
  iSCSI->Reserved         = (UINT16) 0;

  return (EFI_DEVICE_PATH_PROTOCOL *) iSCSI;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextHD (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                *PartitionStr;
  CHAR16                *TypeStr;
  CHAR16                *SignatureStr;
  CHAR16                *StartStr;
  CHAR16                *SizeStr;
  UINT32                Signature32;
  EFI_GUID              SignatureGuid;
  HARDDRIVE_DEVICE_PATH *Hd;

  PartitionStr        = GetNextParamStr (&TextDeviceNode);
  TypeStr             = GetNextParamStr (&TextDeviceNode);
  SignatureStr        = GetNextParamStr (&TextDeviceNode);
  StartStr            = GetNextParamStr (&TextDeviceNode);
  SizeStr             = GetNextParamStr (&TextDeviceNode);
  Hd                  = (HARDDRIVE_DEVICE_PATH *) CreateDeviceNode (
                                                    MEDIA_DEVICE_PATH,
                                                    MEDIA_HARDDRIVE_DP,
                                                    sizeof (HARDDRIVE_DEVICE_PATH)
                                                    );

  Hd->PartitionNumber = (UINT32) Atoi (PartitionStr);

  EfiZeroMem (Hd->Signature, 16);
  Hd->MBRType = (UINT8) 0;

  if (EfiStrCmp (TypeStr, L"None") == 0) {
    Hd->SignatureType = (UINT8) 0;
  } else if (EfiStrCmp (TypeStr, L"MBR") == 0) {
    Hd->SignatureType = SIGNATURE_TYPE_MBR;
    Hd->MBRType       = 0x01;

    Signature32       = (UINT32) Xtoi (SignatureStr);
    EfiCopyMem (Hd->Signature, &Signature32, sizeof (UINT32));
  } else if (EfiStrCmp (TypeStr, L"GUID") == 0) {
    Hd->SignatureType = SIGNATURE_TYPE_GUID;
    Hd->MBRType       = 0x02;

    StrToGuid (SignatureStr, &SignatureGuid);
    EfiCopyMem (Hd->Signature, &SignatureGuid, sizeof (EFI_GUID));
  } else {
    Hd->SignatureType = 0xff;

  }

  Xtoi64 (StartStr, &Hd->PartitionStart);
  Xtoi64 (SizeStr, &Hd->PartitionSize);

  return (EFI_DEVICE_PATH_PROTOCOL *) Hd;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextCDROM (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16            *EntryStr;
  CHAR16            *StartStr;
  CHAR16            *SizeStr;
  CDROM_DEVICE_PATH *CDROM;

  EntryStr              = GetNextParamStr (&TextDeviceNode);
  StartStr              = GetNextParamStr (&TextDeviceNode);
  SizeStr               = GetNextParamStr (&TextDeviceNode);
  CDROM                 = (CDROM_DEVICE_PATH *) CreateDeviceNode (
                                                  MEDIA_DEVICE_PATH,
                                                  MEDIA_CDROM_DP,
                                                  sizeof (CDROM_DEVICE_PATH)
                                                  );

  CDROM->BootEntry      = (UINT32) Xtoi (EntryStr);
  Xtoi64 (StartStr, &CDROM->PartitionStart);
  Xtoi64 (SizeStr, &CDROM->PartitionSize);

  return (EFI_DEVICE_PATH_PROTOCOL *) CDROM;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextVenMEDIA (
  IN CHAR16 *TextDeviceNode
  )
{
  return ConvertFromTextVendor (
           TextDeviceNode,
           MEDIA_DEVICE_PATH,
           MEDIA_VENDOR_DP
           );
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextFilePath (
  IN CHAR16 *TextDeviceNode
  )
{
  FILEPATH_DEVICE_PATH  *File;

  File = (FILEPATH_DEVICE_PATH *) CreateDeviceNode (
                                    MEDIA_DEVICE_PATH,
                                    MEDIA_FILEPATH_DP,
                                    sizeof (FILEPATH_DEVICE_PATH) + (UINT16) (EfiStrLen (TextDeviceNode) * 2)
                                    );

  EfiStrCpy (File->PathName, TextDeviceNode);

  return (EFI_DEVICE_PATH_PROTOCOL *) File;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextMedia (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16                      *GuidStr;
  MEDIA_PROTOCOL_DEVICE_PATH  *Media;

  GuidStr = GetNextParamStr (&TextDeviceNode);
  Media   = (MEDIA_PROTOCOL_DEVICE_PATH *) CreateDeviceNode (
                                             MEDIA_DEVICE_PATH,
                                             MEDIA_PROTOCOL_DP,
                                             sizeof (MEDIA_PROTOCOL_DEVICE_PATH)
                                             );

  StrToGuid (GuidStr, &Media->Protocol);

  return (EFI_DEVICE_PATH_PROTOCOL *) Media;
}

EFI_DEVICE_PATH_PROTOCOL *
DevPathFromTextBBS (
  IN CHAR16 *TextDeviceNode
  )
{
  CHAR16              *TypeStr;
  CHAR16              *IdStr;
  CHAR16              *FlagsStr;
  UINT8               *AsciiStr;
  BBS_BBS_DEVICE_PATH *Bbs;

  TypeStr   = GetNextParamStr (&TextDeviceNode);
  IdStr     = GetNextParamStr (&TextDeviceNode);
  FlagsStr  = GetNextParamStr (&TextDeviceNode);
  Bbs       = (BBS_BBS_DEVICE_PATH *) CreateDeviceNode (
                                        BBS_DEVICE_PATH,
                                        BBS_BBS_DP,
                                        sizeof (BBS_BBS_DEVICE_PATH) + (UINT16) (EfiStrLen (IdStr))
                                        );

  if (EfiStrCmp (TypeStr, L"Floppy") == 0) {
    Bbs->DeviceType = BBS_TYPE_FLOPPY;
  } else if (EfiStrCmp (TypeStr, L"HD") == 0) {
    Bbs->DeviceType = BBS_TYPE_HARDDRIVE;
  } else if (EfiStrCmp (TypeStr, L"CDROM") == 0) {
    Bbs->DeviceType = BBS_TYPE_CDROM;
  } else if (EfiStrCmp (TypeStr, L"PCMCIA") == 0) {
    Bbs->DeviceType = BBS_TYPE_PCMCIA;
  } else if (EfiStrCmp (TypeStr, L"USB") == 0) {
    Bbs->DeviceType = BBS_TYPE_USB;
  } else if (EfiStrCmp (TypeStr, L"Network") == 0) {
    Bbs->DeviceType = BBS_TYPE_EMBEDDED_NETWORK;
  } else {
    Bbs->DeviceType = BBS_TYPE_UNKNOWN;
  }

  AsciiStr = Bbs->String;
  StrToAscii (IdStr, &AsciiStr);

  Bbs->StatusFlag = (UINT16) Xtoi (FlagsStr);

  return (EFI_DEVICE_PATH_PROTOCOL *) Bbs;
}

DEVICE_PATH_FROM_TEXT_TABLE DevPathFromTextTable[] = {
  L"Pci",
  DevPathFromTextPci,
  L"PcCard",
  DevPathFromTextPcCard,
  L"MemoryMapped",
  DevPathFromTextMemoryMapped,
  L"VenHw",
  DevPathFromTextVenHw,
  L"Ctrl",
  DevPathFromTextCtrl,
  L"Acpi",
  DevPathFromTextAcpi,
  L"PciRoot",
  DevPathFromTextPciRoot,
  L"Floppy",
  DevPathFromTextFloppy,
  L"Keyboard",
  DevPathFromTextKeyboard,
  L"Serial",
  DevPathFromTextSerial,
  L"ParallelPort",
  DevPathFromTextParallelPort,
  L"AcpiEx",
  DevPathFromTextAcpiEx,
  L"AcpiExp",
  DevPathFromTextAcpiExp,
  L"Ata",
  DevPathFromTextAta,
  L"Scsi",
  DevPathFromTextScsi,
  L"Fibre",
  DevPathFromTextFibre,
  L"I1394",
  DevPathFromText1394,
  L"USB",
  DevPathFromTextUsb,
  L"I2O",
  DevPathFromTextI2O,
  L"Infiniband",
  DevPathFromTextInfiniband,
  L"VenMsg",
  DevPathFromTextVenMsg,
  L"VenPcAnsi",
  DevPathFromTextVenPcAnsi,
  L"VenVt100",
  DevPathFromTextVenVt100,
  L"VenVt100Plus",
  DevPathFromTextVenVt100Plus,
  L"VenUtf8",
  DevPathFromTextVenUtf8,
  L"UartFlowCtrl",
  DevPathFromTextUartFlowCtrl,
  L"SAS",
  DevPathFromTextSAS,
  L"DebugPort",
  DevPathFromTextDebugPort,
  L"MAC",
  DevPathFromTextMAC,
  L"IPv4",
  DevPathFromTextIPv4,
  L"IPv6",
  DevPathFromTextIPv6,
  L"Uart",
  DevPathFromTextUart,
  L"UsbClass",
  DevPathFromTextUsbClass,
  L"UsbAudio",
  DevPathFromTextUsbAudio,
  L"UsbCDCControl",
  DevPathFromTextUsbCDCControl,
  L"UsbHID",
  DevPathFromTextUsbHID,
  L"UsbImage",
  DevPathFromTextUsbImage,
  L"UsbPrinter",
  DevPathFromTextUsbPrinter,
  L"UsbMassStorage",
  DevPathFromTextUsbMassStorage,
  L"UsbHub",
  DevPathFromTextUsbHub,
  L"UsbCDCData",
  DevPathFromTextUsbCDCData,
  L"UsbSmartCard",
  DevPathFromTextUsbSmartCard,
  L"UsbVideo",
  DevPathFromTextUsbVideo,
  L"UsbDiagnostic",
  DevPathFromTextUsbDiagnostic,
  L"UsbWireless",
  DevPathFromTextUsbWireless,
  L"UsbDeviceFirmwareUpdate",
  DevPathFromTextUsbDeviceFirmwareUpdate,
  L"UsbIrdaBridge",
  DevPathFromTextUsbIrdaBridge,
  L"UsbTestAndMeasurement",
  DevPathFromTextUsbTestAndMeasurement,
  L"UsbWwid",
  DevPathFromTextUsbWwid,
  L"Unit",
  DevPathFromTextUnit,
  L"iSCSI",
  DevPathFromTextiSCSI,
  L"HD",
  DevPathFromTextHD,
  L"CDROM",
  DevPathFromTextCDROM,
  L"VenMEDIA",
  DevPathFromTextVenMEDIA,
  L"Media",
  DevPathFromTextMedia,
  L"BBS",
  DevPathFromTextBBS,
  NULL,
  NULL
};

EFI_DEVICE_PATH_PROTOCOL *
ConvertTextToDeviceNode (
  IN CONST CHAR16 *TextDeviceNode
  )
/*++

  Routine Description:
    Convert text to the binary representation of a device node.

  Arguments:
    TextDeviceNode   -   TextDeviceNode points to the text representation of a device
                         node. Conversion starts with the first character and continues
                         until the first non-device node character.

  Returns:
    A pointer        -   Pointer to the EFI device node.
    NULL             -   If TextDeviceNode is NULL or there was insufficient memory or text unsupported.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL * (*DumpNode) (CHAR16 *);
  CHAR16                   *ParamStr;
  EFI_DEVICE_PATH_PROTOCOL *DeviceNode;
  CHAR16                   *DeviceNodeStr;
  UINTN                    Index;

  if ((TextDeviceNode == NULL) || (IS_NULL (*TextDeviceNode))) {
    return NULL;
  }

  ParamStr      = NULL;
  DumpNode      = NULL;
  DeviceNodeStr = StrDuplicate (TextDeviceNode);

  for (Index = 0; DevPathFromTextTable[Index].Function; Index++) {
    ParamStr = GetParamByNodeName (DeviceNodeStr, DevPathFromTextTable[Index].DevicePathNodeText);
    if (ParamStr != NULL) {
      DumpNode = DevPathFromTextTable[Index].Function;
      break;
    }
  }

  if (DumpNode == NULL) {
    //
    // A file path
    //
    DumpNode = DevPathFromTextFilePath;
    DeviceNode = DumpNode (DeviceNodeStr);
  } else {
    DeviceNode = DumpNode (ParamStr);
    gBS->FreePool (ParamStr);
  }

  gBS->FreePool (DeviceNodeStr);

  return DeviceNode;
}

EFI_DEVICE_PATH_PROTOCOL *
ConvertTextToDevicePath (
  IN CONST CHAR16 *TextDevicePath
  )
/*++

  Routine Description:
    Convert text to the binary representation of a device path.

  Arguments:
    TextDevicePath   -   TextDevicePath points to the text representation of a device
                         path. Conversion starts with the first character and continues
                         until the first non-device node character.

  Returns:
    A pointer        -   Pointer to the allocated device path.
    NULL             -   If TextDeviceNode is NULL or there was insufficient memory.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL * (*DumpNode) (CHAR16 *);
  CHAR16                   *ParamStr;
  EFI_DEVICE_PATH_PROTOCOL *DeviceNode;
  UINTN                    Index;
  EFI_DEVICE_PATH_PROTOCOL *NewDevicePath;
  CHAR16                   *DevicePathStr;
  CHAR16                   *Str;
  CHAR16                   *DeviceNodeStr;
  UINT8                    IsInstanceEnd;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;

  if ((TextDevicePath == NULL) || (IS_NULL (*TextDevicePath))) {
    return NULL;
  }

  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) EfiLibAllocatePool (END_DEVICE_PATH_LENGTH);
  SetDevicePathEndNode (DevicePath);

  ParamStr            = NULL;
  DeviceNodeStr       = NULL;
  DevicePathStr       = StrDuplicate (TextDevicePath);

  Str                 = DevicePathStr;
  while ((DeviceNodeStr = GetNextDeviceNodeStr (&Str, &IsInstanceEnd)) != NULL) {
    DumpNode = NULL;
    for (Index = 0; DevPathFromTextTable[Index].Function; Index++) {
      ParamStr = GetParamByNodeName (DeviceNodeStr, DevPathFromTextTable[Index].DevicePathNodeText);
      if (ParamStr != NULL) {
        DumpNode = DevPathFromTextTable[Index].Function;
        break;
      }
    }

    if (DumpNode == NULL) {
      //
      // A file path
      //
      DumpNode  = DevPathFromTextFilePath;
      DeviceNode = DumpNode (DeviceNodeStr);
    } else {
      DeviceNode = DumpNode (ParamStr);
      gBS->FreePool (ParamStr);
    }

    NewDevicePath = EfiAppendDevicePathNode (DevicePath, DeviceNode);
    gBS->FreePool (DevicePath);
    gBS->FreePool (DeviceNode);
    DevicePath = NewDevicePath;

    if (IsInstanceEnd) {
      DeviceNode = (EFI_DEVICE_PATH_PROTOCOL *) EfiLibAllocatePool (END_DEVICE_PATH_LENGTH);
      SetDevicePathInstanceEndNode (DeviceNode);

      NewDevicePath = EfiAppendDevicePathNode (DevicePath, DeviceNode);
      gBS->FreePool (DevicePath);
      gBS->FreePool (DeviceNode);
      DevicePath = NewDevicePath;
    }
  }

  gBS->FreePool (DevicePathStr);
  return DevicePath;
}
