   ================================================================
   16-bit Headerless Images DataSeries Viewer MFC
   (c) MIT License; by S. Bochkarev
   ================================================================

// The scope
////////////
Basically this project:
  = Source exist as ~/Img16DatSer/*;
  = Executes:
    - as standalone exe;
  = Command line: 
    - c:\img\>Img16DatSer.exe W H wildcard
    - z:\img\>Img16DatSer.exe wildcard
where wildcard composes as "<directory_path\>file_name_wildcard",
e.g. ~"*.raw", ~"z:\img\subdir\????.raw".
  = Should behave "completely correct" under WinE.
  = The data-flow
    - read <un>signed 16bit pixels from file,
    - hold as 32bit signed ints,
    - convert to 8bit grayscale for WindowsGUI,
    - <in DirectcX2D build> extrapolate 8bit grayscale to 32-bit BGRA.

// Build configurations
///////////////////////
Debug, DebUns, Release, RelUns
Suffix 'Uns' doesn't change compiler/linker options; it only adds
suffix 'U' to binary's name (and alters target build's path). With sufix 'U'
in binary's name the initial state of [grayscale-interpretation combobox] will
be "unsigned".

Specific builds:
 - "WindowsGDI" is default,
 - "DirectX 2D" supports only "halftone bitblt-stretch"; this could become
   rational in ~old Linux-PC (slow WinE-GDI).

// Principal rectangles' layout
///////////////////////////////
Mutual <re>calculation of frame- & display-rects is the key of a viewer.
It is implemnted in "GScroll.cpp,.h".

'Frame'    : the whole org image "CRect(0,0,W,H)".
'Rframe'   : visible rectangle in the frame.
'Rclient'  : client area to display image.
'Rdisplay' : painted rectangle within Rclient.

= Integer 'zoom=1' : fits Frame into Rdisplay (with org aspectratio).
'*' : frame,                Rframe  Rclient    Rframe   Rclient
    : Rdisplay;             **      --**--              -----   
'-' : blanc area            **   => --**--     ***** => *****
      of Rclient.           **      --**--              -----
                                  
= Integer 'zoom>1' : stretches certain Rframe into Rclient (with org
                     aspectratio).
'*' : Rframe,               Rframe  Rclient    Rframe   Rclient
    : Rdisplay;             **--      ****     ----     ****  
'-' : invisible area        **--   => ****     --**  => ****
      of the Frame.         ----      ****     --**     ****


// SSE
//////
~SSE2 used only in GBMPSca.cpp,.h. It converts 32-bits signed to unsigned 8-bits.
                         
// [Configuration properties] | [Debugging] for "devenv.exe"
Command           ~: $(TargetPath)
Command arguments ~:
  empty
  256 256 *.dat
Working Directory ~: c:\UtTst