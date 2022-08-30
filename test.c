#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb_image_write.h"

#define ETCDEC_IMPLEMENTATION 1
#include "etcdec.h"

#define GL_RGB                                      0x1907
#define GL_RGBA                                     0x1908

#define COMPRESSED_RGB8_ETC1                        0x8D64

#define COMPRESSED_R11_EAC                          0x9270
#define COMPRESSED_SIGNED_R11_EAC                   0x9271
#define COMPRESSED_RG11_EAC                         0x9272
#define COMPRESSED_SIGNED_RG11_EAC                  0x9273
#define COMPRESSED_RGB8_ETC2                        0x9274
#define COMPRESSED_SRGB8_ETC2                       0x9275
#define COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2    0x9276
#define COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2   0x9277
#define COMPRESSED_RGBA8_ETC2_EAC                   0x9278
#define COMPRESSED_SRGB8_ALPHA8_ETC2_EAC            0x9279

typedef struct KTX1HEADER {
    unsigned char   identifier[12];
    unsigned int    endianness;
    unsigned int    glType;
    unsigned int    glTypeSize;
    unsigned int    glFormat;
    unsigned int    glInternalFormat;
    unsigned int    glBaseInternalFormat;
    unsigned int    pixelWidth;
    unsigned int    pixelHeight;
    unsigned int    pixelDepth;
    unsigned int    numberOfArrayElements;
    unsigned int    numberOfFaces;
    unsigned int    numberOfMipmapLevels;
    unsigned int    bytesOfKeyValueData;
} KTX1HEADER_t;

static unsigned char kKTX1Signature[12] = {
    '«', 'K', 'T', 'X', ' ', '1', '1', '»', '\r', '\n', '\x1A', '\n'
};

int load_ktx1(const char* filePath, int* w, int* h, unsigned int* internalFormat, void** compressedData) {
    unsigned int imageSize, compressedSize;
    KTX1HEADER_t ktxHeader;
    FILE* f;

#if defined(__STDC_LIB_EXT1__) || (_MSC_VER >= 1900)
    if (0 != fopen_s(&f, filePath, "rb")) {
        return 0;
    }
#else
    f = fopen(filePath, "rb");
#endif

    if (!f) {
        return 0;
    }

    if (fread(&ktxHeader, 1, sizeof(ktxHeader), f) != sizeof(ktxHeader)) {
        return 0;
    }

    if (memcmp(ktxHeader.identifier, kKTX1Signature, sizeof(kKTX1Signature)) != 0) {
        return 0;
    }

    if (ktxHeader.bytesOfKeyValueData > 0) {
        fseek(f, ktxHeader.bytesOfKeyValueData, SEEK_CUR);
    }

    if (fread(&imageSize, 1, 4, f) != 4) {
        return 0;
    }

    *w = ktxHeader.pixelWidth;
    *h = ktxHeader.pixelHeight;
    *internalFormat = ktxHeader.glInternalFormat;

    if (ktxHeader.glInternalFormat == COMPRESSED_RGB8_ETC1 ||
        ktxHeader.glInternalFormat == COMPRESSED_RGB8_ETC2 ||
        ktxHeader.glInternalFormat == COMPRESSED_SRGB8_ETC2) {
        compressedSize = ETCDEC_ETC_RGB_COMPRESSED_SIZE(ktxHeader.pixelWidth, ktxHeader.pixelHeight);
    } else if (ktxHeader.glInternalFormat == COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 ||
               ktxHeader.glInternalFormat == COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2) {
        compressedSize = ETCDEC_ETC_RGB_A1_COMPRESSED_SIZE(ktxHeader.pixelWidth, ktxHeader.pixelHeight);
    } else if (ktxHeader.glInternalFormat == COMPRESSED_RGBA8_ETC2_EAC ||
               ktxHeader.glInternalFormat == COMPRESSED_SRGB8_ALPHA8_ETC2_EAC) {
        compressedSize = ETCDEC_EAC_RGBA_COMPRESSED_SIZE(ktxHeader.pixelWidth, ktxHeader.pixelHeight);
    } else if (ktxHeader.glInternalFormat == COMPRESSED_R11_EAC ||
               ktxHeader.glInternalFormat == COMPRESSED_SIGNED_R11_EAC) {
        compressedSize = ETCDEC_EAC_R11_COMPRESSED_SIZE(ktxHeader.pixelWidth, ktxHeader.pixelHeight);
    } else if (ktxHeader.glInternalFormat == COMPRESSED_RG11_EAC ||
               ktxHeader.glInternalFormat == COMPRESSED_SIGNED_RG11_EAC) {
        compressedSize = ETCDEC_EAC_RG11_COMPRESSED_SIZE(ktxHeader.pixelWidth, ktxHeader.pixelHeight);
    } else {
        return 0;
    }

    if (imageSize != compressedSize) {
        return 0;
    }

    *compressedData = malloc(compressedSize);
    if (fread(*compressedData, 1, compressedSize, f) != compressedSize) {
        free(*compressedData);
        *compressedData = 0;
        return 0;
    }
    fclose(f);

    return 1;
}

const char* format_name(int format) {
    switch (format) {
        case COMPRESSED_RGB8_ETC1:                      return "ETC1";
        case COMPRESSED_RGB8_ETC2:                      return "ETC2_RGB Linear";
        case COMPRESSED_SRGB8_ETC2:                     return "ETC2_RGB sRGB";
        case COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:  return "ETC2_RGB_A1 Linear";
        case COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return "ETC2_RGB_A1 sRGB";
        case COMPRESSED_RGBA8_ETC2_EAC:                 return "ETC2_RGBA Linear";
        case COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:          return "ETC2_RGBA sRGB";
        case COMPRESSED_R11_EAC:                        return "EAC_R11 UNorm";
        case COMPRESSED_SIGNED_R11_EAC:                 return "EAC_R11 SNorm";
        case COMPRESSED_RG11_EAC:                       return "EAC_RG11 UNorm";
        case COMPRESSED_SIGNED_RG11_EAC:                return "EAC_RG11 SNorm";
        default:                                        return "Unknown";
    }
}

int main(int argc, char** argv) {
    int w, h, format, i, j, k, m;
    char* compData, * uncompData, * src, * dst;
    float* uncompDataHDR, * dstHDR;
    float tempHDR[16 * 2];  // to hold R or RG
    char path[260];

    if (argc < 2) {
        printf("Usage: %s path/to/input.dds\n", argv[0]);
        printf("       the decompressed image will be written to\n");
        printf("       path/to/input.(tga, hdr)\n");
        return -1;
    }

#if defined(__STDC_LIB_EXT1__) || (_MSC_VER >= 1900)
    strcpy_s(path, sizeof(path), argv[1]);
#else
    strncpy(path, argv[1], sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
#endif

    uncompData = 0;
    uncompDataHDR = 0;
    if (load_ktx1(path, &w, &h, &format,(void**)&compData)) {
        printf("Successfully loaded %s\n", path);
        printf(" w = %d, h = %d, format = %s\n", w, h, format_name(format));

        switch (format) {
            case COMPRESSED_RGB8_ETC1:
            case COMPRESSED_RGB8_ETC2:
            case COMPRESSED_SRGB8_ETC2: {
                uncompData = (char*)malloc(w * h * 4);

                src = compData;
                dst = uncompData;

                for (i = 0; i < h; i += 4) {
                    for (j = 0; j < w; j += 4) {
                        dst = uncompData + (i * w + j) * 4;
                        etcdec_etc_rgb(src, dst, w * 4);
                        src += ETCDEC_ETC_RGB_BLOCK_SIZE;
                    }
                }

                i = (int)strlen(path);
                path[i - 3] = 't';
                path[i - 2] = 'g';
                path[i - 1] = 'a';

                printf("Writing output to %s\n", path);
                stbi_write_tga(path, w, h, 4, uncompData);
            } break;

            case COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            case COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: {
                uncompData = (char*)malloc(w * h * 4);

                src = compData;
                dst = uncompData;

                for (i = 0; i < h; i += 4) {
                    for (j = 0; j < w; j += 4) {
                        dst = uncompData + (i * w + j) * 4;
                        etcdec_etc_rgb_a1(src, dst, w * 4);
                        src += ETCDEC_ETC_RGB_A1_BLOCK_SIZE;
                    }
                }

                i = (int)strlen(path);
                path[i - 3] = 't';
                path[i - 2] = 'g';
                path[i - 1] = 'a';

                printf("Writing output to %s\n", path);
                stbi_write_tga(path, w, h, 4, uncompData);
            } break;

            case COMPRESSED_RGBA8_ETC2_EAC:
            case COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: {
                uncompData = (char*)malloc(w * h * 4);

                src = compData;
                dst = uncompData;

                for (i = 0; i < h; i += 4) {
                    for (j = 0; j < w; j += 4) {
                        dst = uncompData + (i * w + j) * 4;
                        etcdec_eac_rgba(src, dst, w * 4);
                        src += ETCDEC_EAC_RGBA_BLOCK_SIZE;
                    }
                }

                i = (int)strlen(path);
                path[i - 3] = 't';
                path[i - 2] = 'g';
                path[i - 1] = 'a';

                printf("Writing output to %s\n", path);
                stbi_write_tga(path, w, h, 4, uncompData);
            } break;

            case COMPRESSED_R11_EAC:
            case COMPRESSED_SIGNED_R11_EAC: {
                uncompDataHDR = (float*)malloc(w * h * 12);
                memset(uncompDataHDR, 0, w * h * 12);

                src = compData;
                dstHDR = uncompDataHDR;

                for (i = 0; i < h; i += 4) {
                    for (j = 0; j < w; j += 4) {
                        etcdec_eac_r11_float(src, tempHDR, 16, format == COMPRESSED_SIGNED_R11_EAC);
                        dstHDR = uncompDataHDR + (i * w + j) * 3;
                        for (k = 0; k < 4; ++k) {
                            for (m = 0; m < 4; ++m) {
                                dstHDR[m * 3] = tempHDR[m + k * 4];
                            }
                            dstHDR += w * 3;
                        }

                        src += ETCDEC_EAC_R11_BLOCK_SIZE;
                    }
                }

                i = (int)strlen(path);
                path[i - 3] = 'h';
                path[i - 2] = 'd';
                path[i - 1] = 'r';

                printf("Writing output to %s\n", path);
                stbi_write_hdr(path, w, h, 3, uncompDataHDR);
            } break;

            case COMPRESSED_RG11_EAC:
            case COMPRESSED_SIGNED_RG11_EAC: {
                uncompDataHDR = (float*)malloc(w * h * 12);
                memset(uncompDataHDR, 0, w * h * 12);

                src = compData;
                dstHDR = uncompDataHDR;

                for (i = 0; i < h; i += 4) {
                    for (j = 0; j < w; j += 4) {
                        etcdec_eac_rg11_float(src, tempHDR, 32, format == COMPRESSED_SIGNED_RG11_EAC);
                        dstHDR = uncompDataHDR + (i * w + j) * 3;
                        for (k = 0; k < 4; ++k) {
                            for (m = 0; m < 4; ++m) {
                                dstHDR[m * 3 + 0] = tempHDR[(m * 2) + (k * 8) + 0];
                                dstHDR[m * 3 + 1] = tempHDR[(m * 2) + (k * 8) + 1];
                            }
                            dstHDR += w * 3;
                        }

                        src += ETCDEC_EAC_RG11_BLOCK_SIZE;
                    }
                }

                i = (int)strlen(path);
                path[i - 3] = 'h';
                path[i - 2] = 'd';
                path[i - 1] = 'r';

                printf("Writing output to %s\n", path);
                stbi_write_hdr(path, w, h, 3, uncompDataHDR);
            } break;

            default:
                printf("Unknown compression format, terminating\n");
        }

        free(compData);
        free(uncompData);
        free(uncompDataHDR);
    } else {
        printf("Failed to load %s\n", path);
        return -1;
    }

    return (uncompData || uncompDataHDR) ? 0 : -1;
}
