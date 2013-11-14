/******************************************************************************

   Module Name: cvbmp
   Description: Converts a Windows 3.0 color bitmap to the OS/2 PM bitmap format
   Version:     v1.00
   Authors:     Mike Robert
   Date:        7/27/90

   Copyright (C) 1990 by Michael C. Robert.  All rights reserved.

   Revision History:

   Bugs:

   Enhancements:

   Note:

      This program was written before I could obtain a copy of the
   Windows 3.0 SDK, so it may not work for all possible bitmaps.  I had to
   decode the header of the Windows bitmap and I may not have been 100%
   accurate in this.
      
   So be warned!

      Also, the library used to compile this program is LLIBCEC, which may be
   named differently on your computer depending on how you installed the
   compiler.

      Oh, and I forgot to mention, I'm using the Microsoft C v5.1 compiler.

      To recompile just use "MAKE CVBMP".

******************************************************************************/

/* INCLUDE FILES
*/

#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/* SYMBOLIC CONSTANTS
*/

#define  SZB_FILE             128

/* MACROS
*/

/* TYPEDEFS
*/

typedef char BYTE;

typedef unsigned char UBYTE;

typedef short WORD;

typedef unsigned short UWORD;

typedef short SHORT;

typedef unsigned short USHORT;

typedef long LONG;

typedef unsigned long ULONG;

typedef int INT;

typedef unsigned int UINT;

typedef struct _OS2BITMAPINFOHEADER          /* os2bmp */
{
   ULONG          cbFix;
   USHORT         cx;
   USHORT         cy;
   USHORT         cPlanes;
   USHORT         cBitCount;
}
   OS2BITMAPINFOHEADER;

typedef struct _OS2BITMAPFILEHEADER          /* os2bfh */
{
   USHORT         usType;
   ULONG          cbSize;
   INT            xHotspot;
   INT            yHotspot;
   ULONG          offBits;
   OS2BITMAPINFOHEADER os2bmp;
}
   OS2BITMAPFILEHEADER;

typedef struct _OS2RGB                       /* os2rgb */
{
   BYTE           bBlue;
   BYTE           bGreen;
   BYTE           bRed;
}
   OS2RGB;

typedef struct _BITMAPHDR                    /* bmp */
{
   ULONG          ulBitMapHdrSize;

   ULONG          ulWidth,
                  ulHeight;

   USHORT         usPlanes,
                  usBitsPerPlane;

   ULONG          ulFoo1,
                  ulFoo2,
                  ulFoo3,
                  ulFoo4,
                  ulFoo5,
                  ulFoo6;
}
   BITMAPHDR;

typedef struct _BITMAPFILEHDR                /* bmfh */
{
   USHORT         usType;

   ULONG          ulBitmapLength;

   USHORT         usHotSpotX,
                  usHotSpotY;

   ULONG          ulBitsOffset;

   BITMAPHDR      bmp;

}
   BITMAPFILEHDR;

/* GLOBAL VARIABLES
*/

/* FUNCTION PROTOYPES
*/

int main(

   int iArgC,           /* Argument count.
                        */

   char *apszArgV[]     /* Argument string pointer array.
                        */

   );

/******************************************************************************

   Function Name: main

   Description:

      Performs main processing.

   Control Flow:

   Return Values:

      Success: 0
      Failure: 1

   Notes:

******************************************************************************/

int main(

   int iArgC,           /* Argument count.
                        */

   char *apszArgV[]     /* Argument string pointer array.
                        */

   )

{
   /*  Variable definition.  */

   char           szFile[ SZB_FILE ];        /* File name */

   int            fhIn, fhOut;               /* File handles */

   BITMAPFILEHDR  bmfh;                      /* Windows bitmap file header */

   OS2BITMAPFILEHEADER os2bfh;               /* OS/2 bitmap file header */

   int            iC, iMax;                  /* Counting variables */

   ULONG          ulColor;                   /* Color entry */

   USHORT         usScan;                    /* Scan line length */

   char           *pBuffer;                  /* Buffer pointer */

   /*  Check for invalid number of arguments.  */

   if ( iArgC != 2 )
   {
      printf( "CVBMP.EXE Invokation: \n\n" );

      printf( "   CVBMP bitmapfile (no extension)\n" );

      return ( 1 );

   }

   /*  Open the bitmap files.  */

   strcpy( szFile, apszArgV[ 1 ] );
   strcat( szFile, ".BMP" );

   strupr( szFile );

   fhIn = open( szFile, O_RDONLY | O_BINARY );
   if ( fhIn == -1 )
   {
      printf( "Can't open input file: %s\n", szFile );
      return ( 1 );
   }

   strcpy( szFile, apszArgV[ 1 ] );
   strcat( szFile, ".OS2" );

   strupr( szFile );

   fhOut = open( szFile, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
   if ( fhOut == -1 )
   {
      close( fhIn );
      printf( "Can't open output file: %s\n", szFile );
      return ( 1 );
   }

   /*  Read, translate, and write the header.  */

   if ( read( fhIn,
              (char *) &bmfh,
              sizeof ( BITMAPFILEHDR )
            ) != sizeof ( BITMAPFILEHDR )
      )
   {
      close( fhIn );
      close( fhOut );
      printf( "Can't read bitmap file header\n" );
      return ( 1 );
   }

   os2bfh.usType   = bmfh.usType;
   os2bfh.cbSize   = bmfh.ulBitmapLength - bmfh.ulBitsOffset;
   os2bfh.xHotspot = bmfh.usHotSpotX;
   os2bfh.yHotspot = bmfh.usHotSpotY;
   os2bfh.offBits  = sizeof ( OS2BITMAPFILEHEADER );

   if ( bmfh.bmp.usBitsPerPlane < 24 )
      os2bfh.offBits += sizeof ( OS2RGB ) * ( 1 << bmfh.bmp.usBitsPerPlane );

   os2bfh.os2bmp.cbFix     = sizeof ( OS2BITMAPINFOHEADER );
   os2bfh.os2bmp.cx        = (USHORT) bmfh.bmp.ulWidth;
   os2bfh.os2bmp.cy        = (USHORT) bmfh.bmp.ulHeight;
   os2bfh.os2bmp.cPlanes   = bmfh.bmp.usPlanes;
   os2bfh.os2bmp.cBitCount = bmfh.bmp.usBitsPerPlane;

   if ( write( fhOut,
               (char *) &os2bfh,
               sizeof ( OS2BITMAPFILEHEADER )
             ) != sizeof ( OS2BITMAPFILEHEADER )
      )
   {
      close( fhIn );
      close( fhOut );
      printf( "Can't write bitmap file header\n" );
      return ( 1 );
   }

   /*  Read and translate the color table if one is present.  */

   if ( bmfh.bmp.usBitsPerPlane < 24 )
   {
      /*  Calculate the number of entries.  */

      iMax = 1 << bmfh.bmp.usBitsPerPlane;

      for ( iC = 0; iC < iMax; ++iC )
      {
         /*  Read the color and write the new one.  */

         if ( read( fhIn,
                    (char *) &ulColor,
                    sizeof ( ULONG )
                  ) != sizeof ( ULONG )
            )
         {
            close( fhIn );
            close( fhOut );
            printf( "Can't read bitmap color table entry\n" );
            return ( 1 );
         }

         if ( write( fhOut,
                     (char *) &ulColor,
                     sizeof ( OS2RGB )
                   ) != sizeof ( OS2RGB )
            )
         {
            close( fhIn );
            close( fhOut );
            printf( "Can't write bitmap color table entry\n" );
            return ( 1 );
         }

      }

   }

   /*  Finally, read and translate the bits.  */

   usScan = ( (USHORT) ( bmfh.bmp.ulWidth * bmfh.bmp.usBitsPerPlane / 8 ) + 3 ) / 4 * 4;

   pBuffer = malloc( usScan );
   if ( pBuffer == NULL )
   {
      close( fhIn );
      close( fhOut );
      printf( "Can't allocate scan line buffer\n" );
      return ( 1 );
   }

   iMax = (USHORT) bmfh.bmp.ulHeight * bmfh.bmp.usPlanes;

   for ( iC = 0; iC < iMax; ++iC )
   {
      /*  Read a scan line in and then write it out.  */

      printf( "%d \r", iMax - iC );

      if ( read( fhIn,
                 pBuffer,
                 usScan
               ) != usScan
         )
      {
         close( fhIn );
         close( fhOut );
         printf( "Can't read bitmap scan line\n" );
         return ( 1 );
      }

      if ( write( fhOut,
                  pBuffer,
                  usScan
                ) != usScan
         )
      {
         close( fhIn );
         close( fhOut );
         printf( "Can't write bitmap scan line\n" );
         return ( 1 );
      }

   }

   free( pBuffer );

   /*  Close the files and exit.  */

   close( fhIn );
   close( fhOut );

   printf( "Conversion complete.  File %s created.\n", szFile );

   return ( 0 );

}
