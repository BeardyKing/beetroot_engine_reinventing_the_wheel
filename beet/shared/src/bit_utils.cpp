#include <shared/bit_utils.h>

uint32_t count_set_bits(uint32_t n) {
    uint32_t count = 0;
    while (n) {
        count++;
        n >>= 1;
    }
    return count;
}