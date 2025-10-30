#include "bigint.h"
#include <vector>
#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
using namespace std;

void BigInt::initBigEndianRepr(const std::vector<size_t>& bigEndianRepr, size_t base) {
    if (!base) throw std::invalid_argument("base was 0");
    if (base > (1ul << 32)) throw std::invalid_argument("base was > 2**32");
    size_t n = bigEndianRepr.size();
    if (!n) return;
    if (base & (base - 1))  // not a power of 2
        for (int i = n - 1; i >= 0; i--) {
            size_t cur = bigEndianRepr[i];
            if (cur >= base)
                throw std::invalid_argument("bigEndianRepr[" + to_string(i) + "] >= base (" + to_string(cur) +
                                            " >= " + to_string(base) + ")");
            *this *= base;
            *this += cur;
        }
    else {  // base is a power of 2
        size_t shift = std::countr_zero(base);
        v.clear();
        v.reserve((n * shift + 31) / 32);
        uint64_t buf = 0; int bits = 0;
        for (int i = n - 1; i >= 0; i--) {
            uint64_t cur = bigEndianRepr[i];
            if (cur >= base)
                throw std::invalid_argument("bigEndianRepr[" + to_string(i) + "] >= base (" + to_string(cur) +
                                            " >= " + to_string(base) + ")");
            buf |= cur << bits;
            bits += shift;
            if (bits >= 32) {
                v.push_back(static_cast<uint32_t>(buf));
                buf >>= 32;
                bits -= 32;
            }
        }
        if (bits)
            v.push_back(static_cast<uint32_t>(buf));
    }
}

BigInt::BigInt(long val) {
    if (val == std::numeric_limits<long>::min()) {
        sign = true;
        v = {0u, 0x10000000u};
        return;
    }
    sign = val < 0;
    val = abs(val);
    v = {static_cast<uint32_t>(val), static_cast<uint32_t>(val >> 32)};
}

BigInt::BigInt() : v({0u}), sign(false) {}

BigInt::BigInt(const std::string& repr, size_t base) : BigInt() {
    size_t n = repr.size();
    vector<size_t> bigEndianRepr(n);
    for (size_t i = 0; i < n; i++) {
        char cur = repr[i];
        if ('0' <= cur && cur <= '9') bigEndianRepr[i] = cur - '0';
        if ('a' <= cur && cur <= 'z') bigEndianRepr[i] = cur - 'a' + 10;
        if ('A' <= cur && cur <= 'Z') bigEndianRepr[i] = cur - 'A' + 10;
        throw std::invalid_argument("repr[" + to_string(i) + "] is not an alphanumeric character ('" + string(1, cur) +
                                    "')");
    }
    initBigEndianRepr(bigEndianRepr, base);
}

BigInt::BigInt(const std::vector<size_t>& bigEndianRepr, size_t base) : BigInt() {
    initBigEndianRepr(bigEndianRepr, base);
}

BigInt::BigInt(long double val) {
std::
}

BigInt& BigInt::operator=(long val) {}

std::vector<size_t> BigInt::Repr(size_t base) {}

std::string BigInt::ToString(size_t base) {}

BigInt& BigInt::operator+=(const BigInt& other) {}

BigInt BigInt::operator+(const BigInt& other) const {}

BigInt& BigInt::operator-=(const BigInt& other) {}

BigInt BigInt::operator-(const BigInt& other) const {}

BigInt& BigInt::operator*=(const BigInt& other) {}

BigInt BigInt::operator*(const BigInt& other) const {}

BigInt& BigInt::operator/=(const BigInt& other) {}

BigInt BigInt::operator/(const BigInt& other) const {}

BigInt& BigInt::operator%=(const BigInt& other) {}

BigInt BigInt::operator%(const BigInt& other) const {}

std::pair<BigInt, BigInt> BigInt::DivMod(const BigInt& other) const {}

BigInt BigInt::operator-() const {

}

BigInt& BigInt::operator++() {

}

BigInt BigInt::operator++(int) {

}

BigInt& BigInt::operator--() {

}

BigInt BigInt::operator--(int) {

}


int BigInt::operator<=>(const BigInt& other) const {}

int BigInt::operator<=>(long other) const {}

int BigInt::operator<=>(long double other) const {}

long double BigInt::ToFloat() const {}

int operator<=>(long a, const BigInt& b) {}

int operator<=>(long double a, const BigInt& b) {}
