@echo off

echo "Testing ETC1 decompression -> kodim23_etc1.ktx"
etcdec.exe "./test_images/kodim23_etc1.ktx"
if %errorlevel% neq 0 (echo "Decompression failed :(") else (echo "Succeeded!")

echo "Testing ETC2_RGB decompression -> kodim23_etc2_rgb.ktx"
etcdec.exe "./test_images/kodim23_etc2_rgb.ktx"
if %errorlevel% neq 0 (echo "Decompression failed :(") else (echo "Succeeded!")

echo "Testing ETC2_RGB_A1 decompression -> testcard_etc2_rgb_a1.ktx"
etcdec.exe "./test_images/testcard_etc2_rgb_a1.ktx"
if %errorlevel% neq 0 (echo "Decompression failed :(") else (echo "Succeeded!")

echo "Testing ETC2_RGBA decompression -> dice_etc2_rgba.ktx"
etcdec.exe "./test_images/dice_etc2_rgba.ktx"
if %errorlevel% neq 0 (echo "Decompression failed :(") else (echo "Succeeded!")

echo "Testing EAC_R11 decompression -> kodim23_eac_r11.ktx"
etcdec.exe "./test_images/kodim23_eac_r11.ktx"
if %errorlevel% neq 0 (echo "Decompression failed :(") else (echo "Succeeded!")

echo "Testing EAC_RG11 decompression -> kodim23_eac_rg11.ktx"
etcdec.exe "./test_images/kodim23_eac_rg11.ktx"
if %errorlevel% neq 0 (echo "Decompression failed :(") else (echo "Succeeded!")
