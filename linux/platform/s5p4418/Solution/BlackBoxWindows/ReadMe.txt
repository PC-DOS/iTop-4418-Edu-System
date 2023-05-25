//------------------------------------------------------------------------------
//
//  Solution Directory Architecture / Simple Test Guide
//


//------------------------------------------------------------------------------
-. Directory Architecture

 --+-- bin                                   : Binary Files.
   |
   +-- build      - BlackboxSolution         : VisualStudio2010 Solution Files.
   |
   +-- doc                                   : Documents.
   |
   +-- include                               : FFMpeg / Nexell Include Files.
   |
   +-- lib                                   : FFMpeg / Nexell Library.
   |
   +-- src        + baseclasses              : DirectShow Base Classes.
   |              |
   |              + NX_ParserUserData_Test   : UserData Parser Test Application.
   |              |
   |              + NXBlackboxPlayer         : Blackbox Player Test Application.
   |
   +-- ReadMe.txt                            : Solution Directory Architecture.


//------------------------------------------------------------------------------
-. Simple Test Guide

 Step1. Windows Command "cmd". ( at Administrator Premision. (Only up to Windows Vista) )

 Step2. Register Filters.
   regsvr32.exe [TOP_DIRECTORY]\bin\NX_MP4ParserFilter.ax
   regsvr32.exe [TOP_DIRECTORY]\bin\NX_VideoDecoderFilter.ax
   regsvr32.exe [TOP_DIRECTORY]\bin\NX_AudioDecoderFilter.ax
   regsvr32.exe [TOP_DIRECTORY]\bin\NX_TsDemuxFilter.ax

 Step3. Copty to Test Map.
   copy "[TOP_DIRECTORY]\bin\map-google.html" to "C:\"

 Step4. Build NXBlackboxPlayer

 Step5. Enjoy!!


//------------------------------------------------------------------------------
 -. Revision History

   2014.02.11
     Support TS Container.

   2013.12.02
     First Release.


