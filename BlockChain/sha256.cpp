#include "sha256.h"

typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define RIGHTROTATE(x, c) (((x) >> (c)) | ((x) << (32 - (c))))

static const uint32 k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

std::string sha256(const std::string& data) {
    uint32 h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

    uint64 bitlen = data.size() * 8;
    std::vector<uint8> bytes(data.begin(), data.end());
    bytes.push_back(0x80);

    while ((bytes.size() * 8 + 64) % 512 != 0) {
        bytes.push_back(0x00);
    }

    for (int i = 0; i < 8; i++) {
        bytes.push_back((bitlen >> ((7 - i) * 8)) & 0xff);
    }

    for (size_t i = 0; i < bytes.size(); i += 64) {
        uint32 w[64] = { 0 };
        for (int j = 0; j < 16; j++) {
            w[j] = (bytes[i + j * 4] << 24) | (bytes[i + j * 4 + 1] << 16) | (bytes[i + j * 4 + 2] << 8) | bytes[i + j * 4 + 3];
        }

        for (int j = 16; j < 64; j++) {
            uint32 s0 = RIGHTROTATE(w[j - 15], 7) ^ RIGHTROTATE(w[j - 15], 18) ^ (w[j - 15] >> 3);
            uint32 s1 = RIGHTROTATE(w[j - 2], 17) ^ RIGHTROTATE(w[j - 2], 19) ^ (w[j - 2] >> 10);
            w[j] = w[j - 16] + s0 + w[j - 7] + s1;
        }

        uint32 a = h[0];
        uint32 b = h[1];
        uint32 c = h[2];
        uint32 d = h[3];
        uint32 e = h[4];
        uint32 f = h[5];
        uint32 g = h[6];
        uint32 h0 = h[7];

        for (int j = 0; j < 64; j++) {
            uint32 s1 = RIGHTROTATE(e, 6) ^ RIGHTROTATE(e, 11) ^ RIGHTROTATE(e, 25);
            uint32 ch = (e & f) ^ ((~e) & g);
            uint32 temp1 = h0 + s1 + ch + k[j] + w[j];
            uint32 s0 = RIGHTROTATE(a, 2) ^ RIGHTROTATE(a, 13) ^ RIGHTROTATE(a, 22);
            uint32 maj = (a & b) ^ (a & c) ^ (b & c);
            uint32 temp2 = s0 + maj;

            h0 = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += h0;
    }

    std::stringstream ss;
    for (int i = 0; i < 8; i++) {
        ss << std::hex << std::setw(8) << std::setfill('0') << h[i];
    }

    return ss.str();
}