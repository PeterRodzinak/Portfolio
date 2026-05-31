#include <iostream>
#include <set>
#include <bitset>

std::bitset<79> hexToBitset(const std::string& hex_str) {
    std::bitset<79> res;
    // Voliteľné: preskočenie "0x" na začiatku, ak si to s tým skopíroval
    size_t start = (hex_str.rfind("0x", 0) == 0 || hex_str.rfind("0X", 0) == 0) ? 2 : 0;

    for (size_t i = start; i < hex_str.length(); ++i) {
        char c = hex_str[i];
        res <<= 4; // Posunieme doterajší výsledok o 4 bity doľava

        if (c >= '0' && c <= '9')      res |= (c - '0');
        else if (c >= 'A' && c <= 'F') res |= (c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') res |= (c - 'a' + 10);
        else throw std::invalid_argument("Neplatny znak v hex stringu!");
    }
    return res;
}

std::string bitsetToHex(const std::bitset<79>& bs) {
    std::string hex_str = "";

    // 79 bitov sa zmestí do 20 hexadecimálnych znakov (20 * 4 = 80)
    // Ideme od najvýznamnejšieho (najvyššieho) nibblu po najmenší
    for (int i = 19; i >= 0; --i) {
        int nibble = 0;
        for (int j = 0; j < 4; ++j) {
            int bit_idx = i * 4 + j;
            if (bit_idx < 79 && bs[bit_idx]) {
                nibble |= (1 << j);
            }
        }

        // Prevod 4 bitov na Hex znak
        if (nibble < 10) hex_str += char('0' + nibble);
        else             hex_str += char('A' + nibble - 10);
    }

    // Žiadne orezávanie, vrátime rovno prefix "0x" a komplet 20-znakový string
    return "0x" + hex_str;
}

std::bitset<79> POWER_1_BIT(const std::bitset<79> & OP) {
    std::bitset<8> b0, b1, b2, b3, b4, b5, b6, b7, tmp;


    std::bitset<79> res;
    res[0]  = OP[0];
    res[1]  = OP[40] ^ OP[75];
    res[2]  = OP[1];
    res[3]  = OP[41] ^ OP[76];
    res[4]  = OP[2];
    res[5]  = OP[42] ^ OP[77];
    res[6]  = OP[3];
    res[7]  = OP[43] ^ OP[78];
    res[8]  = OP[4];
    res[9]  = OP[44];
    res[10] = OP[5] ^ OP[40] ^ OP[75];
    res[11] = OP[45];
    res[12] = OP[6] ^ OP[41] ^ OP[76];
    res[13] = OP[46];
    res[14] = OP[7] ^ OP[42] ^ OP[77];
    res[15] = OP[47];
    res[16] = OP[8] ^ OP[43] ^ OP[78];
    res[17] = OP[48];
    res[18] = OP[9] ^ OP[44];
    res[19] = OP[49];
    res[20] = OP[10] ^ OP[45];
    res[21] = OP[50];
    res[22] = OP[11] ^ OP[46];
    res[23] = OP[51];
    res[24] = OP[12] ^ OP[47];
    res[25] = OP[52];
    res[26] = OP[13] ^ OP[48];
    res[27] = OP[53];
    res[28] = OP[14] ^ OP[49];
    res[29] = OP[54];
    res[30] = OP[15] ^ OP[50];
    res[31] = OP[55];
    res[32] = OP[16] ^ OP[51];
    res[33] = OP[56];
    res[34] = OP[17] ^ OP[52];
    res[35] = OP[57];
    res[36] = OP[18] ^ OP[53];
    res[37] = OP[58];
    res[38] = OP[19] ^ OP[54];
    res[39] = OP[59];
    res[40] = OP[20] ^ OP[55];
    res[41] = OP[60];
    res[42] = OP[21] ^ OP[56];
    res[43] = OP[61];
    res[44] = OP[22] ^ OP[57];
    res[45] = OP[62];
    res[46] = OP[23] ^ OP[58];
    res[47] = OP[63];
    res[48] = OP[24] ^ OP[59];
    res[49] = OP[64];
    res[50] = OP[25] ^ OP[60];
    res[51] = OP[65];
    res[52] = OP[26] ^ OP[61];
    res[53] = OP[66];
    res[54] = OP[27] ^ OP[62];
    res[55] = OP[67];
    res[56] = OP[28] ^ OP[63];
    res[57] = OP[68];
    res[58] = OP[29] ^ OP[64];
    res[59] = OP[69];
    res[60] = OP[30] ^ OP[65];
    res[61] = OP[70];
    res[62] = OP[31] ^ OP[66];
    res[63] = OP[71];
    res[64] = OP[32] ^ OP[67];
    res[65] = OP[72];
    res[66] = OP[33] ^ OP[68];
    res[67] = OP[73];
    res[68] = OP[34] ^ OP[69];
    res[69] = OP[74];
    res[70] = OP[35] ^ OP[70];
    res[71] = OP[75];
    res[72] = OP[36] ^ OP[71];
    res[73] = OP[76];
    res[74] = OP[37] ^ OP[72];
    res[75] = OP[77];
    res[76] = OP[38] ^ OP[73];
    res[77] = OP[78];
    res[78] = OP[39] ^ OP[74];

    return res;
}

std::bitset<79> POWER_2_BIT(const std::bitset<79> & op) {
    return POWER_1_BIT(POWER_1_BIT(op));
}

std::bitset<79> POWER_4_BIT(const std::bitset<79> & op) {
    return POWER_2_BIT(POWER_2_BIT(op));
}

std::bitset<79> POWER_8_BIT(const std::bitset<79> & op) {
    return POWER_4_BIT(POWER_4_BIT(op));
}

std::bitset<79> POWER_16_BIT(const std::bitset<79> & op) {
    return POWER_8_BIT(POWER_8_BIT(op));
}

std::bitset<79> POWER_32_BIT(const std::bitset<79> & op) {
    return POWER_16_BIT(POWER_16_BIT(op));
}

std::bitset<86> DIGIT_MULT(std::bitset<79> n1, std::bitset<8> n2) {
    std::bitset<86> res(false);
    for (auto i = 0; i < 8; ++i) {
        if (n2[0]) {
            std::bitset<86> tmp(false);
            for (auto j = 0; j < 79; ++j) {
                tmp[j + i] = n1[j];
            }
            res ^= tmp;
        }
        n2 >>= 1;
    }
    return res;
}

std::bitset<79> MULT2(std::bitset<79> n1, std::bitset<79> n2) {
    std::bitset<80> a2(false);
    for (auto i = 0; i < 79; i++) {
        a2[i] = n2[i];
    }

    std::bitset<87> acc(false);
    for (auto i = 9; i >= 0; i--) {
        acc <<= 8;
        std::bitset<8> digit;
        for (auto j = 0; j < 8; j++) {
            digit[j] = a2[72 + j];
        }
        std::bitset<86> tmpres(DIGIT_MULT(n1, digit));
        for (auto j = 0; j < 86; j++) acc[j] = acc[j] ^ tmpres[j];

        for (auto j = 7; j >= 0; j--) {
            acc[j] = acc[j] ^ acc[79 + j];
            acc[9 + j] = acc[9 + j] ^ acc[79 + j];
        }
        a2 <<= 8;
    }

    std::bitset<79> res;
    for (auto i = 0; i < 79; i++) {
        res[i] = acc[i];
    }
    return res;
}

std::bitset<79> MULT(std::bitset<79> n1, std::bitset<79> n2) {
    std::bitset<79> res;
    for (auto i = 0; i < 79; ++i) {
        res = n1[0] ? res ^ n2 : res;
        n1 = n1 >> 1;

        bool save_n2_78 = n2[78];
        bool save_n2_8= n2[8];
        for (auto j = 78; j >= 10; --j) {
            n2[j] = n2[j-1];
        }
        n2[9] = save_n2_8 ^ save_n2_78;
        for (auto j = 8; j >= 1; --j) {
            n2[j] = n2[j-1];
        }
        n2[0] = save_n2_78;
    }
    return res;
}

std::bitset<79> INV(std::bitset<79> op) {
    auto L1 = op;
    //std::cout << "L1: " << bitsetToHex(L1) << std::endl;
    auto L2 = MULT2(L1, POWER_1_BIT(L1));
    //std::cout << "L2: " << bitsetToHex(L2) << std::endl;
    auto L4 = MULT2(L2, POWER_2_BIT(L2));
    //std::cout << "L4: " << bitsetToHex(L4) << std::endl;
    auto L8 = MULT2(L4, POWER_4_BIT(L4));
    //std::cout << "L8: " << bitsetToHex(L8) << std::endl;
    auto L16 = MULT2(L8, POWER_8_BIT(L8));
    //std::cout << "L16: " << bitsetToHex(L16) << std::endl;
    auto L32 = MULT2(L16, POWER_16_BIT(L16));
    //std::cout << "L32: " << bitsetToHex(L32) << std::endl;
    auto L64 = MULT2(L32, POWER_32_BIT(L32));
    //std::cout << "L64: " << bitsetToHex(L64) << std::endl;
    auto L72 = MULT2(L8, POWER_8_BIT(L64));
    //std::cout << "L72: " << bitsetToHex(L72) << std::endl;
    auto L76 = MULT2(L4, POWER_4_BIT(L72));
    //std::cout << "L76: " << bitsetToHex(L76) << std::endl;
    auto L78 = MULT2(L2, POWER_2_BIT(L76));
    //std::cout << "L78: " << bitsetToHex(L78) << std::endl;
    //std::cout << "L79: " << bitsetToHex(POWER_1_BIT(L78)) << std::endl;

    return POWER_1_BIT(L78); // Pridanie poslednej nuly
}

enum State{REGULAR, O_STATE};

struct Point {
    std::bitset<79> X;
    std::bitset<79> Y;
    State state;

    bool operator == (const Point & p) const {
        if (state == O_STATE && p.state == O_STATE) return true;
        if (state != p.state) return false;

        return X == p.X && Y == p.Y;
    }
};

bool isOnCurve(
    const std::bitset<79> & e_A,
    const std::bitset<79> & e_B,
    const Point & p
    ) {

    if (p.state == O_STATE) return true;

    auto lS = POWER_1_BIT(p.Y) ^ MULT(p.X, p.Y);
    auto rS = MULT(POWER_1_BIT(p.X), p.X) ^ MULT(POWER_1_BIT(p.X), e_A) ^ e_B;
    return lS == rS;
}

Point ec_add(
    const std::bitset<79> & e_A,
    const std::bitset<79> & e_B,
    const Point & p0,
    const Point & p1
    ) {

    if (!isOnCurve(e_A, e_B, p0) || !isOnCurve(e_A, e_B, p1))
        throw std::invalid_argument("Point 1 or 2 is not on the elliptic curve");

    if (p0.state == O_STATE) return p1;
    if (p1.state == O_STATE) return p0;

    // Krok 3: Ak x0 != x1 (klasické sčítanie dvoch rôznych bodov)
    if (p0.X != p1.X) {
        // 3.1: lambda = (y0 + y1) / (x0 + x1)
        auto lambda = MULT2(p0.Y ^ p1.Y, INV(p0.X ^ p1.X));

        // 3.2: x2 = a + lambda^2 + lambda + x0 + x1
        auto x2 = e_A ^ POWER_1_BIT(lambda) ^ lambda ^ p0.X ^ p1.X;

        // 3.3 -> Ideme na krok 7: y2 = (x1 + x2) * lambda + x2 + y1
        auto y2 = MULT2(p1.X ^ x2, lambda) ^ x2 ^ p1.Y;

        // Krok 8
        return Point{x2, y2, REGULAR};
    }

    // --- Ak sme sa dostali sem, znamená to, že p0.X == p1.X ---

    // Krok 4: Ak y0 != y1, body sú navzájom inverzné (P a -P), výsledok je O
    if (p0.Y != p1.Y) {
        return Point{std::bitset<79>(), std::bitset<79>(), O_STATE};
    }

    // Krok 5: Ak x1 == 0, dotyčnica je vertikálna, výsledok je O
    // (std::bitset.none() zistí, či sú všetky bity nuly)
    if (p1.X.none()) {
        return Point{std::bitset<79>(), std::bitset<79>(), O_STATE};
    }

    // Krok 6: Zdvojnásobenie bodu (P0 a P1 sú ten istý bod)
    // 6.1: lambda = x1 + y1 / x1
    auto lambda = p1.X ^ MULT2(p1.Y, INV(p1.X));

    // 6.2: x2 = a + lambda^2 + lambda
    auto x2 = e_A ^ POWER_1_BIT(lambda) ^ lambda;

    // Krok 7: y2 = (x1 + x2) * lambda + x2 + y1
    auto y2 = MULT2(p1.X ^ x2, lambda) ^ x2 ^ p1.Y;

    // Krok 8
    return Point{x2, y2, REGULAR};
}

Point ec_double(
    const std::bitset<79> & e_A,
    const std::bitset<79> & e_B,
    const Point & p
) {
    if (p.state == O_STATE || p.X == 0) return Point({false},{false}, O_STATE);

    Point res;
    res.state = REGULAR;
    std::bitset<79> a = p.X ^ MULT2(INV(p.X), p.Y);
    res.X = POWER_1_BIT(a) ^ a ^ e_A;
    a[0] = !a[0];
    res.Y = POWER_1_BIT(p.X) ^ MULT2(a, res.X);
    return res;
}

int main() {



    std::bitset<79> a = hexToBitset("0x4A2E38A8F66D7F4C385F");
    //std::cout << "A: " << bitsetToHex(a) << std::endl;
    std::bitset<79> b = hexToBitset("0x2C0BB31C6BECC03D68A7");
    //std::cout << "B: " << bitsetToHex(b) << std::endl << std::endl;
    Point p0, p1;
    p0.state = State::REGULAR;
    p0.X = hexToBitset("0x30CB127B63E42792F10F");
    p0.Y = hexToBitset("0x547B2C88266BB04F713B");
    p1.state = State::REGULAR;
    p1.X = hexToBitset("0x00202A9F035014497325");
    p1.Y = hexToBitset("0x5175A64859552F97C129");

    auto res = ec_add(a, b, p0, p1);
    std::cout << "X: " << bitsetToHex(res.X) << std::endl;
    std::cout << "Y: " << bitsetToHex(res.Y) << std::endl;

    return 0;
}