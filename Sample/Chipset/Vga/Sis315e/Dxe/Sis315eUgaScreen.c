/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    Sis315eUgaScreen.c

Abstract:

  This file produces the graphics abstration of UGA. It is called by
  LinuxUgaDriver.c file which deals with the EFI 1.1 driver model.
  This file just does graphics.

--*/

#include "Sis315eUga.h"

EFI_LINUX_THUNK_PROTOCOL *mLinux;
static EFI_EVENT          mUgaScreenExitBootServicesEvent;

STATIC
EFI_STATUS
Sis315eUgaStartWindow (
  IN  UGA_PRIVATE_DATA    *Private,
  IN  UINT32              HorizontalResolution,
  IN  UINT32              VerticalResolution,
  IN  UINT32              ColorDepth,
  IN  UINT32              RefreshRate
  );

STATIC
VOID
EFIAPI
KillNtUgaThread (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

//
// UGA Protocol Member Functions
//

EFI_STATUS
EFIAPI
Sis315eUgaGetMode (
  EFI_UGA_DRAW_PROTOCOL *This,
  UINT32                *HorizontalResolution,
  UINT32                *VerticalResolution,
  UINT32                *ColorDepth,
  UINT32                *RefreshRate
  )
/*++

  Routine Description:
    Return the current video mode information.

  Arguments:
    This                  - Protocol instance pointer.
    HorizontalResolution  - Current video horizontal resolution in pixels
    VerticalResolution    - Current video Vertical resolution in pixels
    ColorDepth            - Current video color depth in bits per pixel
    RefreshRate           - Current video refresh rate in Hz.

  Returns:
    EFI_SUCCESS     - Mode information returned.
    EFI_NOT_STARTED - Video display is not initialized. Call SetMode ()
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
// TODO:    ADD IN/OUT description here
{
  UGA_PRIVATE_DATA  *Private;

  DEBUG((EFI_D_ERROR,"Sis315eUgaGetMode\n"));
  Private = UGA_DRAW_PRIVATE_DATA_FROM_THIS (This);

  if (Private->HardwareNeedsStarting) {
    return EFI_NOT_STARTED;
  }

  if ((HorizontalResolution == NULL) ||
      (VerticalResolution   == NULL) ||
      (ColorDepth           == NULL) ||
      (RefreshRate          == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *HorizontalResolution = Private->HorizontalResolution;
  *VerticalResolution   = Private->VerticalResolution;
  *ColorDepth           = Private->ColorDepth;
  *RefreshRate          = Private->RefreshRate;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Sis315eUgaSetMode (
  EFI_UGA_DRAW_PROTOCOL *This,
  UINT32                HorizontalResolution,
  UINT32                VerticalResolution,
  UINT32                ColorDepth,
  UINT32                RefreshRate
  )
/*++

  Routine Description:
    Return the current video mode information.

  Arguments:
    This                  - Protocol instance pointer.
    HorizontalResolution  - Current video horizontal resolution in pixels
    VerticalResolution    - Current video Vertical resolution in pixels
    ColorDepth            - Current video color depth in bits per pixel
    RefreshRate           - Current video refresh rate in Hz.

  Returns:
    EFI_SUCCESS     - Mode information returned.
    EFI_NOT_STARTED - Video display is not initialized. Call SetMode ()
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
// TODO:    EFI_DEVICE_ERROR - add return value to function comment
// TODO:    EFI_DEVICE_ERROR - add return value to function comment
// TODO:    ADD IN/OUT description here
{
  EFI_STATUS        Status;
  UGA_PRIVATE_DATA  *Private;
  EFI_UGA_PIXEL     Fill;

  DEBUG((EFI_D_ERROR,"Sis315eUgaSetMode\n"));
  Private = UGA_DRAW_PRIVATE_DATA_FROM_THIS (This);

  if (Private->HardwareNeedsStarting) {
    Status = Sis315eUgaStartWindow (
              Private,
              HorizontalResolution,
              VerticalResolution,
              ColorDepth,
              RefreshRate
              );
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }

    Private->HardwareNeedsStarting = FALSE;
  }
  Status = Private->UgaIo->UgaSize(Private->UgaIo,
             HorizontalResolution,
             VerticalResolution);

  Private->HorizontalResolution = HorizontalResolution;
  Private->VerticalResolution   = VerticalResolution;
  Private->ColorDepth           = ColorDepth;
  Private->RefreshRate          = RefreshRate;

  Fill.Red                      = 0x0;
  Fill.Green                    = 0x0;
  Fill.Blue                     = 0x0;
  Fill.Reserved                 = 0x0;
  This->Blt (
          This,
          &Fill,
          EfiUgaVideoFill,
          0,
          0,
          0,
          0,
          HorizontalResolution,
          VerticalResolution,
          HorizontalResolution * sizeof (EFI_UGA_PIXEL)
          );
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Sis315eUgaBlt (
  IN  EFI_UGA_DRAW_PROTOCOL                   *This,
  IN  EFI_UGA_PIXEL                           *BltBuffer, OPTIONAL
  IN  EFI_UGA_BLT_OPERATION                   BltOperation,
  IN  UINTN                                   SourceX,
  IN  UINTN                                   SourceY,
  IN  UINTN                                   DestinationX,
  IN  UINTN                                   DestinationY,
  IN  UINTN                                   Width,
  IN  UINTN                                   Height,
  IN  UINTN                                   Delta         OPTIONAL
  )
/*++

  Routine Description:
    Blt pixels from the rectangle (Width X Height) formed by the BltBuffer
    onto the graphics screen starting a location (X, Y). (0, 0) is defined as
    the upper left hand side of the screen. (X, Y) can be outside of the
    current screen geometry and the BltBuffer will be cliped when it is
    displayed. X and Y can be negative or positive. If Width or Height is
    bigger than the current video screen the image will be clipped.

  Arguments:
    This          - Protocol instance pointer.
    X             - X location on graphics screen.
    Y             - Y location on the graphics screen.
    Width         - Width of BltBuffer.
    Height        - Hight of BltBuffer
    BltOperation  - Operation to perform on BltBuffer and video memory
    BltBuffer     - Buffer containing data to blt into video buffer. This
                    buffer has a size of Width*Height*sizeof(EFI_UGA_PIXEL)
    SourceX       - If the BltOperation is a EfiCopyBlt this is the source
                    of the copy. For other BLT operations this argument is not
                    used.
    SourceX       - If the BltOperation is a EfiCopyBlt this is the source
                    of the copy. For other BLT operations this argument is not
                    used.

  Returns:
    EFI_SUCCESS           - The palette is updated with PaletteArray.
    EFI_INVALID_PARAMETER - BltOperation is not valid.
    EFI_DEVICE_ERROR      - A hardware error occured writting to the video
                             buffer.

--*/
// TODO:    SourceY - add argument and description to function comment
// TODO:    DestinationX - add argument and description to function comment
// TODO:    DestinationY - add argument and description to function comment
// TODO:    Delta - add argument and description to function comment
{
  UGA_PRIVATE_DATA  *Private;
  EFI_TPL           OriginalTPL;
  EFI_STATUS        Status;

  DEBUG((EFI_D_ERROR,"Sis315eUgaBlt\n"));
  Private = UGA_DRAW_PRIVATE_DATA_FROM_THIS (This);

  if ((BltOperation < 0) || (BltOperation >= EfiUgaBltMax)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_UGA_PIXEL);
  }

  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  Status = Private->UgaIo->UgaBlt (Private->UgaIo,
             BltBuffer,
             BltOperation,
             SourceX, SourceY,
             DestinationX, DestinationY,
             Width, Height,
             Delta);

  DEBUG((EFI_D_ERROR,"Sis315eUgaBlt OK\n"));
  gBS->RestoreTPL (OriginalTPL);

  return Status;
}


//
// Construction and Destruction functions
//

EFI_STATUS
Sis315eUgaSupported (
  IN  EFI_LINUX_IO_PROTOCOL  *LinuxIo
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    LinuxIo - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  //
  // Check to see if the IO abstraction represents a device type we support.
  //
  // This would be replaced a check of PCI subsystem ID, etc.
  //
  if (!EfiCompareGuid (LinuxIo->TypeGuid, &gEfiLinuxUgaGuid)) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
Sis315eUgaStartWindow (
  IN  UGA_PRIVATE_DATA    *Private,
  IN  UINT32              HorizontalResolution,
  IN  UINT32              VerticalResolution,
  IN  UINT32              ColorDepth,
  IN  UINT32              RefreshRate
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private               - TODO: add argument description
  HorizontalResolution  - TODO: add argument description
  VerticalResolution    - TODO: add argument description
  ColorDepth            - TODO: add argument description
  RefreshRate           - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  EFI_STATUS          Status;

  Private->HorizontalResolution = HorizontalResolution;
  Private->VerticalResolution   = VerticalResolution;

  //
  // Register to be notified on exit boot services so we can destroy the window.
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES,
                  EFI_TPL_CALLBACK,
                  KillNtUgaThread,
                  Private,
                  &mUgaScreenExitBootServicesEvent
                  );

  Status = UgaCreate(&Private->UgaIo);
  return Status;
}

EFI_STATUS
Sis315eUgaConstructor (
  UGA_PRIVATE_DATA    *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    Private - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{

  Private->UgaDraw.GetMode        = Sis315eUgaGetMode;
  Private->UgaDraw.SetMode        = Sis315eUgaSetMode;
  Private->UgaDraw.Blt            = Sis315eUgaBlt;

  Private->HardwareNeedsStarting  = TRUE;
  Private->UgaIo = NULL;

  return EFI_SUCCESS;
}

EFI_STATUS
Sis315eUgaDestructor (
  UGA_PRIVATE_DATA     *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    Private - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  if (!Private->HardwareNeedsStarting) {
    Private->UgaIo->UgaClose(Private->UgaIo);
    Private->UgaIo = NULL;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
KillNtUgaThread (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  This is the UGA screen's callback notification function for exit-boot-services.
  All we do here is call LinuxUgaDestructor().

Arguments:

  Event   - not used
  Context - pointer to the Private structure.

Returns:

  None.

--*/
{
  EFI_STATUS  Status;
  Status = Sis315eUgaDestructor (Context);
}
