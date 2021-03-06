/*++

Copyright 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    splitfile.c
    
Abstract:

--*/

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

void
helpmsg (
  void
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:


Returns:

  GC_TODO: add return values

--*/
{
  printf (
    "SplitFile Filename Offset\n""   Filename = Input file to split\n""   Offset = offset at which to split file\n"
    "\n\n""SplitFile will break a file in two pieces at the requested offset\n"
    "  outputting Filename1 and Filename2\n"
    );
}

int
main (
  int argc,
  char*argv[]
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  argc  - GC_TODO: add argument description
  argv  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  FILE          *In;

  FILE          *Out1;

  FILE          *Out2;
  char          OutName1[512];
  char          OutName2[512];
  unsigned long Index;
  unsigned long splitpoint;
  char          CharC;

  if (argc != 3) {
    helpmsg ();
    return -1;
  }

  In = fopen (argv[1], "rb");
  if (In == NULL) {
    printf ("Unable to open file \"%s\"\n", argv[1]);
    return -1;
  }

  strncpy (OutName1, argv[1], 510);
  strncpy (OutName2, argv[1], 510);
  strcat (OutName1, "1");
  strcat (OutName2, "2");

  Out1 = fopen (OutName1, "wb");
  if (Out1 == NULL) {
    printf ("Unable to open file \"%s\"\n", OutName1);
    return -1;
  }

  Out2 = fopen (OutName2, "wb");
  if (Out2 == NULL) {
    printf ("Unable to open file \"%s\"\n", OutName2);
    return -1;
  }

  splitpoint = atoi (argv[2]);

  for (Index = 0; Index < splitpoint; Index++) {
    CharC = (char) fgetc (In);
    if (feof (In)) {
      break;
    }

    fputc (CharC, Out1);
  }

  for (;;) {
    CharC = (char) fgetc (In);
    if (feof (In)) {
      break;
    }

    fputc (CharC, Out2);
  }

  fclose (In);
  fclose (Out1);
  fclose (Out2);

  return 0;
}
