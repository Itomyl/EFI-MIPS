/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LightConSplitter.h

Abstract:

  Private data structures for the Console Splitter driver

--*/

#ifndef SPLITTER_H_
#define SPLITTER_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiCompNameSupport.h"
#include "LightComponentName.h"

//
// Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (ConsoleControl)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)
#include EFI_PROTOCOL_DEFINITION (GraphicsOutput)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_GUID_DEFINITION (ConsoleInDevice)
#include EFI_GUID_DEFINITION (ConsoleOutDevice)
#include EFI_GUID_DEFINITION (StandardErrorDevice)

//
// Procduced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_GUID_DEFINITION (PrimaryConsoleInDevice)
#include EFI_GUID_DEFINITION (PrimaryConsoleOutDevice)
#include EFI_GUID_DEFINITION (PrimaryStandardErrorDevice)

//
// Private Data Structures
//
#define CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT  32
#define CONSOLE_SPLITTER_MODES_ALLOC_UNIT     32
#define MAX_STD_IN_PASSWORD                   80

typedef struct {
  UINTN Columns;
  UINTN Rows;
} TEXT_OUT_SPLITTER_QUERY_DATA;

//
// Private data for the EFI_SIMPLE_INPUT_PROTOCOL splitter
//
#define TEXT_IN_SPLITTER_PRIVATE_DATA_SIGNATURE EFI_SIGNATURE_32 ('T', 'i', 'S', 'L')

typedef struct {
  UINT64                      Signature;
  EFI_HANDLE                  VirtualHandle;

  EFI_SIMPLE_TEXT_IN_PROTOCOL TextIn;
  UINTN                       CurrentNumberOfConsoles;
  EFI_SIMPLE_TEXT_IN_PROTOCOL **TextInList;
  UINTN                       TextInListCount;

  BOOLEAN                     PasswordEnabled;
  CHAR16                      Password[MAX_STD_IN_PASSWORD];
  UINTN                       PwdIndex;
  CHAR16                      PwdAttempt[MAX_STD_IN_PASSWORD];
  EFI_EVENT                   LockEvent;

  BOOLEAN                     KeyEventSignalState;
} TEXT_IN_SPLITTER_PRIVATE_DATA;

#define TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      TEXT_IN_SPLITTER_PRIVATE_DATA, \
      TextIn, \
      TEXT_IN_SPLITTER_PRIVATE_DATA_SIGNATURE \
      )

//
// Private data for the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL splitter
//
#define TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('T', 'o', 'S', 'L')

typedef struct {
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  EFI_UGA_DRAW_PROTOCOL         *UgaDraw;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;
  BOOLEAN                       TextOutEnabled;
} TEXT_OUT_AND_GOP_DATA;

typedef struct {
  UINT64                          Signature;
  EFI_HANDLE                      VirtualHandle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL    TextOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE     TextOutMode;

#if (EFI_SPECIFICATION_VERSION < 0x00020000)
  EFI_UGA_DRAW_PROTOCOL           UgaDraw;
  UINT32                          UgaHorizontalResolution;
  UINT32                          UgaVerticalResolution;
  UINT32                          UgaColorDepth;
  UINT32                          UgaRefreshRate;
  EFI_UGA_PIXEL                   *UgaBlt;
#else
  EFI_GRAPHICS_OUTPUT_PROTOCOL    GraphicsOutput;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *GraphicsOutputBlt;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *GraphicsOutputModeBuffer;
  UINTN                           CurrentNumberOfGraphicsOutput;
  UINTN                           CurrentNumberOfUgaDraw;
  BOOLEAN                         HardwareNeedsStarting;
#endif

  EFI_CONSOLE_CONTROL_PROTOCOL    ConsoleControl;

  UINTN                           CurrentNumberOfConsoles;
  TEXT_OUT_AND_GOP_DATA           *TextOutList;
  UINTN                           TextOutListCount;
  TEXT_OUT_SPLITTER_QUERY_DATA    *TextOutQueryData;
  UINTN                           TextOutQueryDataCount;
  INT32                           *TextOutModeMap;

  EFI_CONSOLE_CONTROL_SCREEN_MODE ConsoleOutputMode;

  UINTN                           DevNullColumns;
  UINTN                           DevNullRows;
  CHAR16                          *DevNullScreen;
  INT32                           *DevNullAttributes;

} TEXT_OUT_SPLITTER_PRIVATE_DATA;

#define TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA, \
      TextOut, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE \
      )

#define GRAPHICS_OUTPUT_SPLITTER_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA, \
      GraphicsOutput, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE \
      )

#define UGA_DRAW_SPLITTER_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA, \
      UgaDraw, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE \
      )

#define CONSOLE_CONTROL_SPLITTER_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA, \
      ConsoleControl, \
      TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE \
      )

//
// Global variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gConSplitterConInDriverBinding;
extern EFI_DRIVER_BINDING_PROTOCOL  gConSplitterConOutDriverBinding;
extern EFI_DRIVER_BINDING_PROTOCOL  gConSplitterStdErrDriverBinding;

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
ConSplitterDriverEntry (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  )
;

STATIC
EFI_STATUS
ConSplitterTextInConstructor (
  TEXT_IN_SPLITTER_PRIVATE_DATA       *Private
  )
;

STATIC
EFI_STATUS
ConSplitterTextOutConstructor (
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private
  )
;

//
// Driver Binding Functions
//
STATIC
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
;

STATIC
EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
;

//
// TextIn Constructor/Destructor functions
//
EFI_STATUS
ConSplitterTextInAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *TextIn
  )
;

EFI_STATUS
ConSplitterTextInDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *TextIn
  )
;

//
// TextOut Constuctor/Destructor functions
//
EFI_STATUS
ConSplitterTextOutAddDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *TextOut,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
;

EFI_STATUS
ConSplitterTextOutDeleteDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *TextOut
  )
;

//
// TextIn I/O Functions
//
EFI_STATUS
EFIAPI
ConSplitterTextInReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *This,
  IN  BOOLEAN                         ExtendedVerification
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL     *This,
  OUT EFI_INPUT_KEY                   *Key
  )
;

VOID
EFIAPI
ConSplitterTextInWaitForKey (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
;

BOOLEAN
ConSpliterConssoleControlStdInLocked (
  VOID
  )
;

VOID
EFIAPI
ConSpliterConsoleControlLockStdInEvent (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
;

EFI_STATUS
EFIAPI
ConSpliterConsoleControlLockStdIn (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN  CHAR16                          *Password
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextInPrivateReadKeyStroke (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  OUT EFI_INPUT_KEY                   *Key
  )
;

//
// TextOut I/O Functions
//
VOID
ConSplitterSynchronizeModeData (
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *Private
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         ExtendedVerification
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  )
;

EFI_STATUS
EFIAPI
ConSplitterTextOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         Visible
  )
;

EFI_STATUS
ConSplitterGrowBuffer (
  IN  UINTN                           SizeOfCount,
  IN  UINTN                           *Count,
  IN OUT  VOID                        **Buffer
  )
;

EFI_STATUS
EFIAPI
ConSpliterConsoleControlGetMode (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  OUT EFI_CONSOLE_CONTROL_SCREEN_MODE *Mode,
  OUT BOOLEAN                         *GopExists,
  OUT BOOLEAN                         *StdInLocked
  )
;

EFI_STATUS
EFIAPI
ConSpliterConsoleControlSetMode (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode
  )
;

EFI_STATUS
EFIAPI
ConSpliterGraphicsOutputQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL		    *This,
  IN  UINT32		                        ModeNumber,
  OUT UINTN		                           *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	**Info
  )
;

EFI_STATUS
EFIAPI
ConSpliterGraphicsOutputSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
;

EFI_STATUS
EFIAPI
ConSpliterGraphicsOutputBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL                  *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL                 *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION             BltOperation,
  IN  UINTN                                         SourceX,
  IN  UINTN                                         SourceY,
  IN  UINTN                                         DestinationX,
  IN  UINTN                                         DestinationY,
  IN  UINTN                                         Width,
  IN  UINTN                                         Height,
  IN  UINTN                                         Delta         OPTIONAL
  )
;

EFI_STATUS
DevNullGopSync (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
;

#if (EFI_SPECIFICATION < 0x00020000)
EFI_STATUS
EFIAPI
ConSpliterUgaDrawGetMode (
  IN  EFI_UGA_DRAW_PROTOCOL           *This,
  OUT UINT32                          *HorizontalResolution,
  OUT UINT32                          *VerticalResolution,
  OUT UINT32                          *ColorDepth,
  OUT UINT32                          *RefreshRate
  )
;

EFI_STATUS
EFIAPI
ConSpliterUgaDrawSetMode (
  IN  EFI_UGA_DRAW_PROTOCOL           *This,
  IN UINT32                           HorizontalResolution,
  IN UINT32                           VerticalResolution,
  IN UINT32                           ColorDepth,
  IN UINT32                           RefreshRate
  )
;

EFI_STATUS
EFIAPI
ConSpliterUgaDrawBlt (
  IN  EFI_UGA_DRAW_PROTOCOL                         *This,
  IN  EFI_UGA_PIXEL                                 *BltBuffer, OPTIONAL
  IN  EFI_UGA_BLT_OPERATION                         BltOperation,
  IN  UINTN                                         SourceX,
  IN  UINTN                                         SourceY,
  IN  UINTN                                         DestinationX,
  IN  UINTN                                         DestinationY,
  IN  UINTN                                         Width,
  IN  UINTN                                         Height,
  IN  UINTN                                         Delta         OPTIONAL
  )
;

EFI_STATUS
DevNullUgaSync (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
;
#endif

EFI_STATUS
DevNullTextOutOutputString (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  CHAR16                          *WString
  )
;

EFI_STATUS
DevNullTextOutSetMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  UINTN                           ModeNumber
  )
;

EFI_STATUS
DevNullTextOutClearScreen (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
;

EFI_STATUS
DevNullTextOutSetCursorPosition (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  )
;

EFI_STATUS
DevNullTextOutEnableCursor (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  BOOLEAN                         Visible
  )
;

EFI_STATUS
DevNullSyncGopStdOut (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
;

#endif
