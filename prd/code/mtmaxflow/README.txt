

USAGE
====================
 1. Run "startup"
 2. Run "make"
 3. Check out the "demo" directory



BOOST
=================================

To compile the source files, Boost is required. It can be downloaded from www.boost.org.


Windows 32-bit
--------------
Should be pretty straight-forward to dowload boost and bjam and 
compile it
1. Run "bjam" in the Boost directory

Windows 64-bit
-----------------
1. Install MSVC++ amd64 compiler according to Mathworks instructions
   Easiest is to install Visual Studio Professional (free for students)
2. Download boost and bjam executable
3. Start a MSVC++ **amd64/x64** command prompt. 
   You might need to edit and copy .cmd files if you're using the Express version.
4. Run "bjam address-model=64" in the Boost directory

Configuration of MATLAB with Boost
-----------------------------------
 1. Make sure you have selected the same compiler with "mex -setup".
 2. "cd(prefdir); edit mexopts.bat"
 3. Add Boost to include dir
 4. Add Boost/lib to library dir  (or Boost/stage/lib if using Windows 64-bit)


Petter Strandmark
petter@maths.lth.se

