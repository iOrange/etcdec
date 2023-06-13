# etcdec
Small header-only C library to decompress any ETC/EAC compressed image inspired by incredible stb libaries (<http://nothings.org/stb>)

Written by Sergii *"iOrange"* Kudlai in 2022.

This library provides functions to decompress blocks of ETC/EAC compressed images.  

This library does not allocate memory and is trying to use as less stack as possible.

The library was never optimized specifically for speed but for the overall size.  
It has zero external dependencies and is not using any runtime functions.

### Supported formats:
- ETC1
- ETC2 (RGB, RGB + A1 aka "punchthrough alpha", RGBA)
- EAC R11 / RG11

---

ETC1/ETC2_RGB/ETC2_RGB_A1/ETC2_RGBA are expected to decompress into 4\*4 RGBA blocks 8bit per component (32bit pixel)
EAC_R11/EAC_RG11 are expected to decompress into 4\*4 R/RG blocks of either 32bit float or 16bit unsigned int16 per
component (32/16bit (R11) and 64/32bit (RG11) pixel)

---

You will also find included test program that converts compressed KTX1 files into TGA/HDR.  
It is a good start to learn on how to use the **etcdec** library.  
It comes with some test images in the *test_images* folder and a batch script *test_bcdec.bat* to run over them.

---

### CREDITS:
 - Vladimir Vondrus (@mosra)
    - fixes for platforms that define char as unsigned type
