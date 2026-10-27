#pragma once

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

void gr2_decompress(uint32_t compressed_size, const uint8_t* compressed_buffer,
                    uint32_t step1, uint32_t step2,
                    uint32_t decompressed_size, uint8_t* decompressed_buffer);

#ifdef __cplusplus
}
#endif
