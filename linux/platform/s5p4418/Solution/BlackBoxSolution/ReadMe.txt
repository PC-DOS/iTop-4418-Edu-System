//------------------------------------------------------------------------------
//
//	Revision History and 
//              Simple Library Package Architecture & Build Order
//


//------------------------------------------------------------------------------
 -. Revision History
   2014.08.25 ( v1.0.12 )
     Support MP4 Container. ( bug fixed )
     Bug fixed nxmp4encsol. ( Start / Stop Bug )

   2014.07.14 ( v1.0.11 )
     Support Real Time Protocol(RTP).

   2014.06.16 ( v1.0.10 )
     Support HDMI output.
   
   2014.05.23
     Add GUI Solution.  

   2014.04.21 
     Stable Filter solution. ( Various Bug Fixed )
	   Broken data bug fixed (Text Overlay / TS Stream)
     Add Micom Senario.

   2014.02.18
     Modify Writing Time Handling. ( bug fixed )
     Add Rate control paramter handling. ( bug fixed )
     Add Message Queue. (apps)
     Modify FileWriter. ( bug fixed - thread base)
     
   2014.02.11
     Add simple MP4 encoding library and test application.
     Suuport Motion Detection.
     Support MP3 Audio Codec.

   2014.01.17
     Support TS Container.
     Support HLS Component.
     Support 3D Image effect. ( 3D Scaler )
	 
   2013.12.02
     First Release.

 -. Last Revision Number
    libnxdvr : 1.0.12


//------------------------------------------------------------------------------
-. Directory Architecture

Solution
 |
 +-+-- apps    --+-- nxdvrmonitor : simple wireless network monitor program ( Support SoftAP & Station Mode )
   |			 |
   |             +-- nxdvrsol     : simple blackbox encoding application
   |             |
   |             +-- nxguisol     : simple GUI based blackbox / player application
   |             |
   |             +-- nxhlssol     : simple HLS test application
   |             |
   |             +-- nxmp4encsol  : simple MP4 encoding test application
   |             |
   |             +-- nxrtpsol     : simple RTP test application
   |
   +-- bin                        : build result & resource files
   |
   +-- build                      : library & application build script
   |
   +-- include                    : include files
   |
   +-- lib                        : private static library, library build result
   |
   +-- src     --+-- libnxdvr     : blackbox encoding manager library
                 |
                 +-- libnxfilters : base filter components
                 |
                 +-- libnxhls     : HLS manager library
                 |
                 +-- libnxmp4manager : simple MP4 encoding manager library
                 |
                 +-- libnxrtp     : simple HLS manager library


//------------------------------------------------------------------------------
-. Build Prepare. 

 Step 1. Modify "build.env" file for each system.

      $ vi [nexell_linux]/pyrope/build.env
        ...
        ARCHDIR   := /home/doriya/working/nexell_linux/pyrope/
        KERNDIR   := /home/doriya/working/nexell_linux/kernel/kernel-3.4.39/
        ..

 Step 2. Build system library.
      [ARCHDIR]/library/src/libion
      [ARCHDIR]/library/src/libnxmalloc
      [ARCHDIR]/library/src/libnxv4l2
      [ARCHDIR]/library/src/libnxvpu
      [ARCHDIR]/library/src/libnxgraphictools
      [ARCHDIR]/library/src/libnxnmeaparser
      [ARCHDIR]/library/src/libnxadc
      [ARCHDIR]/library/src/libnxgpio

      $ cd [ARCHDIR]/library/src/[Each library directory]
      $ make
      $ make install

 Step 3. Build driver module. ( must build kernel )
      Coda960
      $ cd [ARCHDIR]/modules/coda960
      $ make ARCH=arm

      $ cd [ARCHDIR]/module/
      $ make ARCH=arm


//------------------------------------------------------------------------------
-. Build Application

 1. Default Mathod. 
 Step 1. Build Filter

      $ cd [ARCHDIR]/Solution/BlackBoxSolution/src/libfilters
      $ make

 Step 2. Build Manager

      $ cd [ARCHDIR]/Solution/BlackBoxSolution/src/libnxdvr
      $ make
      $ make install

 Step 3. Build Application

      $ cd [ARCHDIR]/Solution/BlackBoxSoltion/apps/nxdvrsol
      $ make
      $ make install

 2. Another Mathod. (using script)
 Step 1. Build all

      $ cd [ARCHDIR]/Solution/BlackBoxSoltion/build/
      $ ./build-blackbox.sh


//------------------------------------------------------------------------------
-. Run Sequence.

 Step 1. Prepare root file system ( [ARCHDIR]/fs/buildroot/ )
	 buildroot configuration : br.2013.11.cortex_a9_glibc_gst_sdl_dfb_wifi.config
	  
 Step 2. Copy system dependent shared objects.

      $ cp -a [ARCHDIR]/library/lib/*.so [ROOTDIR]/usr/lib

 Step 3. Copy Blackbox solution shared objects.

      $ cp -a [SOLUTION]/lib/*.so [ROOTDIR]/usr/lib
 
 Step 4. Copy Encoder module driver.

      $ cp -a [ARCHDIR]/modules/code960/nx_vpu.ko [ROOTDIR]/root

 Step 5. Copy Application & Resouce file to root file system.

      $ cp -a [SOLUTION]/bin * [ROOTDIR]/root
 
 Step 6. System Booting.

 Step 7. Load Encoder Driver ( at Target board )

      $ insmod /root/nx_vpu.ko

 Step 8. Run Blackbox Solution applications ( at Target board)

      $ /root/nxdvrsol

//------------------------------------------------------------------------------
-. Etc.

 Support HLS at nxdvrsol/nxguisol.
 ( Lynx Board + Wired LAN )
 Step 1. Make Temperary Directory. (at Target Board )

      $ mkdir /tmp/www
      $ ln -s /tmp/www /www
      $ cp /root/www/index.html /www"

 Step 2. Running http demon

      $ httpd

 Step 3. Running Application

      $ /root/nxdvrsol -n

 Step 4. Connection 
      Connetection Address : http://[Target Board IP]/test.m3u8

 UI Base Blackbox Solution.
 Step 1. Running BlackBox GUI Solution.

      $ /root/nxguisol

      For use player in nxguisol, 
               need "libnxmovieplayer"

