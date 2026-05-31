#include <iostream>
#include <cinttypes>
#include <iomanip>
#include <ios>
#include <bit>

uint32_t P = 0xB7E15163;
uint32_t Q = 0x9E3779B9;

void print32Hexa(uint32_t n) {
    unsigned int buf[4];
    buf[0] = n & 0xFF;
    buf[1] = (n >> 8) & 0xFF;
    buf[2] = (n >> 16) & 0xFF;
    buf[3] = (n >> 24) & 0xFF;
    std::cout << std::hex << std::setfill('0')
    << std::setw(2) << buf[0]
    << std::setw(2) << buf[1]
    << std::setw(2) << buf[2]
    << std::setw(2) << buf[3];
}

uint32_t barrel_shift(uint32_t op, uint32_t shift) {
    shift = shift % 32;
    return shift == 0 ? op : (op << shift) | (op >> (32 - shift));
}

int main() {
    uint32_t len = 4;
    uint32_t L[4] = {0, 0, 0, 0};

    uint32_t S[44];
    for (uint32_t & i : S) {
        i = 0;
    }

    S[0] = P;
    for (int i = 1; i < 44; i++) {
        S[i] = S[i-1] + Q;
    }

    uint32_t A = 0, B = 0, i = 0, j = 0;
    for (uint32_t round = 1; round <= 132; round++) {
        A = S[i] = barrel_shift(S[i] + A + B, 3);
        B = L[j] = barrel_shift(L[j] + A + B, A + B);
        i = (i + 1) % 44;
        j = (j + 1) % len;
    }

    std::string ref = "4b38ac8305b4016ac5bd8db755fcc1b3c7b27ae363018004359b2c7d8e6d2107c8af1e9159abcdcafd97734962a03756848939e09fd534b1774b9ee58690aee884bfed4261b5d3901db66d3abd468f9e5a199fce512b019f11b98fb8a6b4a4be7cb6d7600025068d66"
                      "e7755f15ee39a1abbc779b5ec06b9b56f4da3c75524dd99306e49ac955b6bf429c272466f7fa9c4e6ee8c6b1d887264bed6bcd9c64df5f39d141858108b2079b17852d2a66311c";

    for (int str_i = ref.length() - 1; str_i >= 0; str_i -= 2) {
        char first, second;
        first = ref[str_i - 1];
        second = ref[str_i];
        std::cout << first << second;
    }
    std::cout << std::endl;

    uint32_t A1 = 0, B1 = 0, C1 = 0, D1 = 0;

    B1 = B1 + S[0];
    D1 = D1 + S[1];
    for (uint32_t round = 1; round <= 20; round++) {
        std::cout << "Start of round " << round << ":" << std::endl;
        std::cout << "A: " << std::setfill('0') << std::setw(8) << std::hex << A1 << std::dec << std::endl;
        std::cout << "B: " << std::setfill('0') << std::setw(8) << std::hex << B1 << std::dec << std::endl;
        std::cout << "C: " << std::setfill('0') << std::setw(8) << std::hex << C1 << std::dec << std::endl;
        std::cout << "D: " << std::setfill('0') << std::setw(8) << std::hex << D1 << std::dec << std::endl;

        uint32_t t = std::rotl(B1 * (2 * B1 + 1), 5);
        uint32_t u = barrel_shift(D1 * (2 * D1 + 1), 5);
        A1 = std::rotl(A1 ^ t, u & 31) + S[2*round];
        C1 = std::rotl(C1 ^ u, t & 31) + S[2*round+1];
        uint32_t tmp = A1;
        A1 = B1;
        B1 = C1;
        C1 = D1;
        D1 = tmp;


    }
    A1 = A1 + S[42];
    C1 = C1 + S[43];


    return 0;
}