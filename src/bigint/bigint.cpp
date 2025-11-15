#include "bigint.h"

#include <ieee754.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <compare>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace std;

/*
template<class T>
static ostream& operator<<(ostream& out, const vector<T>& v) {
    out << '{';
    bool first = true;
    for (auto& i : v) {
        if (!first) out << ", ";
        first = false;
        out << i;
    }
    return out << '}';
}
*/

ZeroDivisionException::ZeroDivisionException() : runtime_error("Tried to divide by BigInt(0)") {}

BigInt::BigInt(const vector<uint32_t>& v, bool sign) : v(v), sign(sign) { normalize(); }

static void assert_base_ge2(size_t base) {
    if (!base || base == 1) throw std::invalid_argument("base cannot be 0 or 1");
}

bool BigInt::IsNegative() const { return sign; }

void BigInt::normalize() {
    while (v.size() && !v.back()) v.pop_back();
    if (v.empty()) {
        v.push_back(0u);
        sign = false;
    }
}

void BigInt::initBigEndianRepr(const std::vector<size_t>& bigEndianRepr, size_t base) {
    assert_base_ge2(base);
    if (base > (1ul << 32)) throw std::invalid_argument("base was > 2**32");
    size_t n = bigEndianRepr.size();
    if (!n) return;
    if (base & (base - 1))  // not a power of 2
        for (size_t i = 0; i < n; i++) {
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
        uint64_t buf = 0;
        int bits = 0;
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
        if (bits) v.push_back(static_cast<uint32_t>(buf));
    }
    normalize();
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
    normalize();
}

BigInt::BigInt(size_t val) : v(2, 0), sign(false) {
    v[0] = static_cast<uint32_t>(val);
    v[1] = static_cast<uint32_t>(val >> 32);
    normalize();
}

BigInt::BigInt(int val) : v(1, abs(val)), sign(val < 0) { normalize(); }

BigInt::BigInt() : v({0u}), sign(false) {}

BigInt::BigInt(const std::string& repr, size_t base) : BigInt() {
    assert_base_ge2(base);
    size_t n = repr.size();
    vector<size_t> bigEndianRepr(n);
    if (repr.empty()) return;
    bool minus = false;
    if (repr[0] == '-') minus = true;
    for (size_t i = minus; i < n; i++) {
        char cur = repr[i];
        if ('0' <= cur && cur <= '9')
            bigEndianRepr[i] = cur - '0';
        else if ('a' <= cur && cur <= 'z')
            bigEndianRepr[i] = cur - 'a' + 10;
        else if ('A' <= cur && cur <= 'Z')
            bigEndianRepr[i] = cur - 'A' + 10;
        else
            throw std::invalid_argument("repr[" + to_string(i) + "] is not an alphanumeric character ('" +
                                        string(1, cur) + "')");
    }
    initBigEndianRepr(bigEndianRepr, base);
    sign = minus;
    normalize();
}

BigInt::BigInt(const std::vector<size_t>& bigEndianRepr, size_t base) : BigInt() {
    assert_base_ge2(base);
    initBigEndianRepr(bigEndianRepr, base);
}

BigInt::BigInt(long double val) : BigInt() {
    if (isnan(val) || isinf(val)) return;
    ieee854_long_double ld;
    ld.d = val;
    int bits = static_cast<int>(ld.ieee.exponent - IEEE854_LONG_DOUBLE_BIAS + 1);
    if (bits <= 0) return;
    uint64_t man_whole = (static_cast<uint64_t>(ld.ieee.mantissa0) << 32) | ld.ieee.mantissa1;
    int rshift = max(0, 64 - bits);
    man_whole >>= rshift;
    int zeros = max(0, bits - 64);
    v.assign(zeros / 32, 0u);
    zeros %= 32;
    v.push_back(static_cast<uint32_t>(man_whole) << zeros);
    man_whole >>= 32 - zeros;
    v.push_back(static_cast<uint32_t>(man_whole));
    v.push_back(static_cast<uint32_t>(man_whole >> 32));
    sign = ld.ieee.negative;
    normalize();
}

BigInt& BigInt::operator=(long val) {
    this->~BigInt();
    new (this) BigInt(val);
    return *this;
}

BigInt& BigInt::operator=(size_t val) {
    sign = false;
    v.resize(2);
    v[0] = static_cast<uint32_t>(val);
    v[1] = static_cast<uint32_t>(val >> 32);
    return *this;
}

std::vector<size_t> BigInt::Repr(size_t base) const {
    assert_base_ge2(base);
    vector<size_t> res;
    if (base & (base - 1)) {  // not a power of 2
        BigInt cur = *this;
        cur.sign = false;
        while (cur) {
            auto [d, r] = *cur.DivMod(base);
            res.push_back(0);
            size_t& resi = res.back();
            if (r.v.size() == 2)
                resi = static_cast<uint64_t>(r.v[1]) << 32;
            else
                resi = 0;
            resi |= static_cast<uint64_t>(r.v[0]);
            cur = d;
        }
        if (res.empty()) res.push_back(0u);
    } else {  // a power of 2
        int bits = countr_zero(base);
        uint64_t buf = 0;
        int buflen = 0;
        for (uint32_t num : v) {
            buf |= num << buflen;
            buflen += 32;
            while (buflen >= bits) {
                res.push_back(buf & (base - 1));
                buf >>= bits;
                buflen -= bits;
            }
        }
        if (buflen) res.push_back(buf);
    }
    reverse(res.begin(), res.end());
    return res;
}

std::string BigInt::ToString(size_t base) const {
    assert_base_ge2(base);
    if (base > 10 + 26) throw std::invalid_argument("base > 36 for string representation");
    auto repr = Repr(base);
    size_t n = repr.size();
    string res(n, '.');
    for (size_t i = 0; i < n; i++) {
        size_t val = repr[i];
        char& dest = res[i];
        if (val < 10)
            dest = '0' + val;
        else
            dest = 'A' + val - 10;
    }
    if (sign) res.insert(res.begin(), '-');
    return res;
}

struct VectorView {
    vector<uint32_t>& v;
    size_t start, end;
    VectorView(vector<uint32_t>& v, size_t start, size_t end) : v(v), start(start), end(end) {}
    VectorView(vector<uint32_t>& v) : v(v), start(0), end(v.size()) {}
    size_t size() const { return end - start; }
    uint32_t& operator[](int index) { return v[index + start]; }
    uint32_t& get(int index) { return v[index + start]; }
    vector<uint32_t> Cut() const { return vector<uint32_t>(v.begin() + start, v.begin() + end); }
    pair<VectorView, VectorView> Split(size_t lowsize) const {
        return {{v, start, start + lowsize}, {v, start + lowsize, end}};
    }
    VectorView Subview(size_t from, size_t to) const { return {v, start + from, start + to}; }
};

struct ConstVectorView {
    const vector<uint32_t>& v;
    size_t start, end;
    ConstVectorView(const vector<uint32_t>& v, size_t start, size_t end) : v(v), start(start), end(end) {}
    ConstVectorView(const vector<uint32_t>& v) : v(v), start(0), end(v.size()) {}
    ConstVectorView(const VectorView& mut) : v(mut.v), start(mut.start), end(mut.end) {}
    size_t size() const { return end - start; }
    uint32_t operator[](int index) const { return v[index + start]; }
    uint32_t get(int index) const { return v[index + start]; }
    vector<uint32_t> Cut() const { return vector<uint32_t>(v.begin() + start, v.begin() + end); }
    pair<ConstVectorView, ConstVectorView> Split(size_t lowsize) const {
        return {{v, start, start + lowsize}, {v, start + lowsize, end}};
    }
    ConstVectorView Subview(size_t from, size_t to) const { return {v, start + from, start + to}; }
};

/*
static ostream& operator<<(ostream& out, ConstVectorView a) {
    out << '{';
    for (size_t i = 0; i < a.size(); i++) {
        if (i) out << ", ";
        out << a[i];
    }
    return out << '}';
}
*/

static int UnsignedBigCompare(ConstVectorView a, ConstVectorView b) {
    size_t an = a.size(), bn = b.size();
    ConstVectorView *pa = &a, *pb = &b;
    int sign = 1;
    if (an < bn) {
        swap(pa, pb);
        swap(an, bn);
        sign = -1;
    }
    for (int i = static_cast<int>(an) - 1; i >= static_cast<int>(bn); i--) {
        if (pa->get(i)) return sign;
    }
    for (int i = static_cast<int>(bn) - 1; i >= 0; i--) {
        auto av = a[i], bv = b[i];
        if (av > bv) return 1;
        if (av < bv) return -1;
    }
    return 0;
}

static int UnsignedBigCompare(const vector<uint32_t>& a, const vector<uint32_t>& b) {
    return UnsignedBigCompare({a, 0, a.size()}, {b, 0, b.size()});
}

static uint32_t BigAdd(VectorView dest, ConstVectorView src) {
    uint64_t buf = 0;
    size_t srcn = src.size();
    for (size_t i = 0; i < srcn; i++) {
        buf += static_cast<uint64_t>(dest[i]) + src[i];
        dest[i] = static_cast<uint32_t>(buf);
        buf >>= 32;
    }
    return buf;
}

static void BigAdd(vector<uint32_t>& dest, ConstVectorView src) {
    if (dest.size() < src.size()) dest.resize(src.size());
    uint32_t carry = BigAdd({dest, 0, dest.size()}, src);
    if (carry) {
        if (dest.size() == src.size())
            dest.push_back(static_cast<uint32_t>(carry));
        else
            dest[src.size()] += static_cast<uint32_t>(carry);
    }
}

static void BigAdd(vector<uint32_t>& dest, const vector<uint32_t>& src) { BigAdd(dest, {src, 0, src.size()}); }

static void BigSub(VectorView dest, ConstVectorView src) {
    long buf = 0;
    size_t n = dest.size();
    size_t srcn = src.size();
    for (size_t i = 0; i < n; i++) {
        buf += dest[i];
        if (i < srcn) buf -= src[i];
        dest[i] = static_cast<uint32_t>(bit_cast<uint64_t>(buf));
        buf >>= 32;
    }
}

static void BigSub(vector<uint32_t>& dest, const vector<uint32_t>& src) {
    BigSub({dest, 0, dest.size()}, {src, 0, src.size()});
}

BigInt& BigInt::operator+=(const BigInt& other) {
    if (sign == other.sign)
        BigAdd(v, other.v);
    else {
        int comp = UnsignedBigCompare(v, other.v);
        if (comp == -1) {
            auto buf = other.v;
            BigSub(buf, v);
            v = buf;
            sign = other.sign;
        } else if (comp == 0) {
            v.assign(1, 0u);
            sign = false;
        } else
            BigSub(v, other.v);
    }
    normalize();
    return *this;
}

BigInt BigInt::operator+(const BigInt& other) const {
    BigInt res = *this;
    return res += other;
}

BigInt& BigInt::operator-=(const BigInt& other) {
    if (sign == other.sign) {
        int comp = UnsignedBigCompare(v, other.v);
        if (comp == -1) {
            auto buf = other.v;
            BigSub(buf, v);
            v = buf;
            sign = !sign;
        } else if (comp == 0) {
            v.assign(1, 0u);
            sign = false;
        } else
            BigSub(v, other.v);
    } else
        BigAdd(v, other.v);
    normalize();
    return *this;
}

BigInt BigInt::operator-(const BigInt& other) const {
    BigInt res = *this;
    return res -= other;
}

static uint32_t BigMul(VectorView a, uint32_t b) {
    uint64_t buf = 0;
    size_t n = a.size();
    for (size_t i = 0; i < n; i++) {
        buf += static_cast<uint64_t>(a[i]) * b;
        a[i] = static_cast<uint32_t>(buf);
        buf >>= 32;
    }
    return static_cast<uint32_t>(buf);
}

static void BigMul(vector<uint32_t>& a, uint32_t b) {
    uint32_t carry = BigMul({a, 0, a.size()}, b);
    if (carry) a.push_back(carry);
}

static void AddFrom(vector<uint32_t>& dest, size_t startadd, ConstVectorView src) {
    dest.resize(max(startadd + src.size(), dest.size()));
    uint32_t carry = BigAdd({dest, startadd, dest.size()}, src);
    if (carry) {
        size_t ind = startadd + src.size();
        if (ind == dest.size()) dest.push_back(0);
        dest[ind] += carry;
    }
}

static vector<uint32_t> KaratsubaMul(ConstVectorView a, ConstVectorView b) {
    // a = wc + x; b = yc + z
    // a * b = wycc + (wz + xy)c + xz
    // wz + xy = (w + x)(y + z) - wy - xz
    ConstVectorView *pa = &a, *pb = &b;
    if (pa->size() < pb->size()) swap(pa, pb);
    if (pb->size() == 1) {
        auto v = pa->Cut();
        BigMul(v, pb->get(0));
        return v;
    }
    size_t half = pa->size() / 2;
    if (half >= pb->size()) {
        auto lh = pa->Split(half);
        auto low = KaratsubaMul(lh.first, *pb);
        auto high = KaratsubaMul(lh.second, *pb);
        AddFrom(low, half, high);
        return low;
    }
    auto x_w = pa->Split(half);
    auto z_y = pb->Split(half);
    ConstVectorView w = x_w.second, x = x_w.first, y = z_y.second, z = z_y.first;
    auto wy = KaratsubaMul(w, y);
    auto xz = KaratsubaMul(x, z);
    auto wplusx = w.Cut();
    BigAdd(wplusx, x);
    auto yplusz = y.Cut();
    BigAdd(yplusz, z);
    auto mid = KaratsubaMul(wplusx, yplusz);
    BigSub(mid, wy);
    BigSub(mid, xz);
    AddFrom(xz, half, mid);
    AddFrom(xz, half * 2, wy);
    return xz;
}

BigInt& BigInt::operator*=(const BigInt& other) { return *this = *this * other; }

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt res;
    res.v = KaratsubaMul(v, other.v);
    res.sign = sign != other.sign;
    res.normalize();
    return res;
}

BigInt& BigInt::operator/=(const BigInt& other) {
    auto optres = DivLeaveMod(other);
    if (optres)
        *this = *optres;
    else
        throw ZeroDivisionException();
    return *this;
}

BigInt BigInt::operator/(const BigInt& other) const {
    auto optres = DivMod(other);
    if (!optres) throw ZeroDivisionException();
    return optres->first;
}

BigInt& BigInt::operator%=(const BigInt& other) {
    auto optres = DivLeaveMod(other);
    if (!optres) throw ZeroDivisionException();
    return *this;
}

BigInt BigInt::operator%(const BigInt& other) const {
    auto optres = DivMod(other);
    if (!optres) throw ZeroDivisionException();
    return optres->second;
}

static void OutOfPlaceMul(VectorView dest, ConstVectorView a, uint32_t b) {
    uint64_t buf = 0;
    size_t n = a.size();
    for (size_t i = 0; i < n; i++) {
        buf += static_cast<uint64_t>(a[i]) * b;
        dest[i] = static_cast<uint32_t>(buf);
        buf >>= 32;
    }
    size_t dn = dest.size();
    for (size_t i = n; i < dn; i++) {
        dest[i] = buf;
        buf = 0;
    }
}

static vector<uint32_t> BigDiv(VectorView a, ConstVectorView b) {
    size_t bn = b.size();
    if (bn > a.size()) return {};
    vector<uint32_t> res(a.size() - bn + 1);
    int ahigh = a.size();
    vector<uint32_t> buf(bn + 1);
    VectorView bufview = buf;
    for (int i = static_cast<int>(res.size()) - 1; i >= 0; i--) {
        // alow = i
        if (ahigh - i < static_cast<long>(bn)) continue;
        auto aview = a.Subview(i, ahigh);
        uint32_t& digit = res[i];
        for (int bit = 31; bit >= 0; bit--) {
            digit |= 1u << bit;
            OutOfPlaceMul(bufview, b, digit);
            if (UnsignedBigCompare(bufview, aview) > 0)  // too much
                digit ^= 1u << bit;
        }
        OutOfPlaceMul(bufview, b, digit);
        ConstVectorView cview = bufview;
        if (!buf.back()) cview.end--;
        BigSub(aview, cview);
        while (!a[ahigh - 1] && ahigh) --ahigh;
    }
    return res;
}

optional<BigInt> BigInt::DivLeaveMod(const BigInt& other) {
    if (other.v.size() == 1 && !other.v[0]) return {};
    if (v.size() == 1 && !v[0]) return {BigInt()};
    auto resv = BigDiv(v, other.v);
    if (sign == other.sign) {
        normalize();
        return BigInt(resv, false);
    }
    bool ressign = sign != other.sign;
    normalize();
    if (*this) {
        BigAdd(resv, vector<uint32_t>{1u});
        vector<uint32_t> invrem = other.v;
        BigSub(invrem, v);
        v = invrem;
        sign = other.sign;
        normalize();
    }
    return BigInt(resv, ressign);
}

std::optional<std::pair<BigInt, BigInt>> BigInt::DivMod(const BigInt& other) const {
    BigInt rem = *this;
    auto optquot = rem.DivLeaveMod(other);
    if (optquot.has_value()) return {{*optquot, rem}};
    return {};
}

BigInt BigInt::operator-() const { return BigInt(v, !sign); }

BigInt& BigInt::Negate() {
    if (*this) sign = !sign;
    return *this;
}

BigInt& BigInt::operator++() {
    if (!sign) {
        BigAdd(v, vector<uint32_t>{1u});
        return *this;
    }
    BigSub(v, vector<uint32_t>{1u});
    normalize();
    return *this;
}

BigInt BigInt::operator++(int) {
    BigInt res;
    return ++res;
}

BigInt& BigInt::operator--() {
    if (sign) {
        BigAdd(v, vector<uint32_t>{1u});
        return *this;
    }
    if (!*this) {
        sign = true;
        v[0] = 1;
        return *this;
    }
    BigSub(v, vector<uint32_t>{1u});
    normalize();
    return *this;
}

BigInt BigInt::operator--(int) {
    BigInt res = *this;
    return --res;
}

int BigInt::operator<=>(const BigInt& other) const {
    if (sign != other.sign) {
        if (sign) return -1;
        return 1;
    }
    if (sign) return -UnsignedBigCompare(v, other.v);
    return UnsignedBigCompare(v, other.v);
}

int BigInt::operator<=>(long other) const { return *this <=> BigInt(other); }
int BigInt::operator<=>(size_t other) const { return *this <=> BigInt(other); }
int BigInt::operator<=>(int other) const { return *this <=> BigInt(other); }

std::partial_ordering BigInt::operator<=>(long double other) const {
    if (isnan(other)) return partial_ordering::unordered;
    if (isinf(other)) {
        if (other < 0) return partial_ordering::greater;
        return partial_ordering::less;
    }
    ieee854_long_double ld;
    ld.d = other;
    if (sign != ld.ieee.negative) {
        if (sign) return partial_ordering::less;
        return partial_ordering::greater;
    }
    int otherbits = ld.ieee.exponent - IEEE854_LONG_DOUBLE_BIAS + 1;
    if (otherbits <= 0) {
        if (sign) return partial_ordering::less;
        if (*this) return partial_ordering::greater;
        if (other == 0.0) return partial_ordering::equivalent;
        if (other < 0) return partial_ordering::greater;
        return partial_ordering::less;
    }
    size_t otherbits_ = static_cast<size_t>(otherbits);
    size_t mybits = SignificantBits();
    if (mybits < otherbits_) {
        if (sign) return partial_ordering::greater;
        return partial_ordering::less;
    }
    if (mybits > otherbits_) {
        if (sign) return partial_ordering::less;
        return partial_ordering::greater;
    }
    int comp = *this <=> BigInt(other);
    switch (comp) {
        case -1:
            return partial_ordering::less;
        case 1:
            return partial_ordering::greater;
    }
    uint32_t man0 = ld.ieee.mantissa0, man1 = ld.ieee.mantissa1;
    int take = min(32, otherbits);
    man0 >>= take;
    otherbits -= take;
    take = min(32, otherbits);
    man1 >>= take;
    if (!man0 && !man1) return partial_ordering::equivalent;
    if (sign) return partial_ordering::greater;
    return partial_ordering::less;
}

long BigInt::ClampToLong() const {
    if (*this >= numeric_limits<long>::max()) return numeric_limits<long>::max();
    if (*this <= numeric_limits<long>::min()) return numeric_limits<long>::min();
    unsigned long res = static_cast<unsigned long>(v[0]);
    if (v.size() > 1) res |= static_cast<unsigned long>(v[1]) << 32;
    if (sign) res *= -1;
    return res;
}

long double BigInt::ToFloat() const {
    if (!*this) return 0.0;
    size_t bits = SignificantBits();
    size_t biased = bits + IEEE854_LONG_DOUBLE_BIAS - 1;
    if (biased >= (1u << 15) - 1) return INFINITY;
    ieee854_long_double ld;
    ld.ieee.negative = sign;
    ld.ieee.exponent = biased;
    uint64_t man = v.back();
    int manlen = 32 - countl_zero(v.back());
    for (int i = static_cast<int>(v.size()) - 2; i >= 0 && manlen < 64; i--) {
        int take = min(32, 64 - manlen);
        uint32_t cur = v[i];
        man = (man << take) | (cur >> (32 - take));
        manlen += take;
    }
    man <<= 64 - manlen;
    ld.ieee.empty = 0;
    ld.ieee.mantissa1 = static_cast<uint32_t>(man);
    ld.ieee.mantissa0 = static_cast<uint32_t>(man >> 32);
    return ld.d;
}

bool BigInt::operator<(const BigInt& other) const { return (*this <=> other) < 0; }
bool BigInt::operator<(long other) const { return (*this <=> other) < 0; }
bool BigInt::operator<(size_t other) const { return (*this <=> other) < 0; }
bool BigInt::operator<(int other) const { return (*this <=> other) < 0; }
bool BigInt::operator<(long double other) const { return (*this <=> other) < 0; }
bool BigInt::operator<=(const BigInt& other) const { return (*this <=> other) <= 0; }
bool BigInt::operator<=(long other) const { return (*this <=> other) <= 0; }
bool BigInt::operator<=(size_t other) const { return (*this <=> other) <= 0; }
bool BigInt::operator<=(int other) const { return (*this <=> other) <= 0; }
bool BigInt::operator<=(long double other) const { return (*this <=> other) <= 0; }
bool BigInt::operator>(const BigInt& other) const { return (*this <=> other) > 0; }
bool BigInt::operator>(long other) const { return (*this <=> other) > 0; }
bool BigInt::operator>(size_t other) const { return (*this <=> other) > 0; }
bool BigInt::operator>(int other) const { return (*this <=> other) > 0; }
bool BigInt::operator>(long double other) const { return (*this <=> other) > 0; }
bool BigInt::operator>=(const BigInt& other) const { return (*this <=> other) >= 0; }
bool BigInt::operator>=(long other) const { return (*this <=> other) >= 0; }
bool BigInt::operator>=(size_t other) const { return (*this <=> other) >= 0; }
bool BigInt::operator>=(int other) const { return (*this <=> other) >= 0; }
bool BigInt::operator>=(long double other) const { return (*this <=> other) >= 0; }
bool BigInt::operator==(const BigInt& other) const { return !(*this <=> other); }
bool BigInt::operator==(long other) const { return !(*this <=> other); }
bool BigInt::operator==(size_t other) const { return !(*this <=> other); }
bool BigInt::operator==(int other) const { return !(*this <=> other); }
bool BigInt::operator==(long double other) const { return (*this <=> other) == partial_ordering::equivalent; }
bool BigInt::operator!=(const BigInt& other) const { return (*this <=> other); }
bool BigInt::operator!=(long other) const { return !(*this <=> other); }
bool BigInt::operator!=(size_t other) const { return !(*this <=> other); }
bool BigInt::operator!=(int other) const { return !(*this <=> other); }
bool BigInt::operator!=(long double other) const { return (*this <=> other) != partial_ordering::equivalent; }

BigInt::operator bool() const { return v.size() > 1 || v[0]; }

size_t BigInt::SignificantBits() const { return v.size() * 32 - countl_zero(v.back()); }

int operator<=>(size_t a, const BigInt& b) { return -(b <=> a); }

int operator<=>(int a, const BigInt& b) { return -(b <=> a); }

int operator<=>(long a, const BigInt& b) { return -(b <=> a); }

std::partial_ordering operator<=>(long double a, const BigInt& b) {
    auto res = b <=> a;
    if (res == partial_ordering::unordered || res == partial_ordering::equivalent) return res;
    if (res == partial_ordering::less) return partial_ordering::greater;
    return partial_ordering::less;
}

bool operator<(long a, const BigInt& b) { return (a <=> b) < 0; }
bool operator<=(long a, const BigInt& b) { return (a <=> b) <= 0; }
bool operator>(long a, const BigInt& b) { return (a <=> b) > 0; }
bool operator>=(long a, const BigInt& b) { return (a <=> b) >= 0; }
bool operator==(long a, const BigInt& b) { return (a <=> b) == 0; }
bool operator!=(long a, const BigInt& b) { return (a <=> b) != 0; }

bool operator<(size_t a, const BigInt& b) { return (a <=> b) < 0; }
bool operator<=(size_t a, const BigInt& b) { return (a <=> b) <= 0; }
bool operator>(size_t a, const BigInt& b) { return (a <=> b) > 0; }
bool operator>=(size_t a, const BigInt& b) { return (a <=> b) >= 0; }
bool operator==(size_t a, const BigInt& b) { return (a <=> b) == 0; }
bool operator!=(size_t a, const BigInt& b) { return (a <=> b) != 0; }

bool operator<(int a, const BigInt& b) { return (a <=> b) < 0; }
bool operator<=(int a, const BigInt& b) { return (a <=> b) <= 0; }
bool operator>(int a, const BigInt& b) { return (a <=> b) > 0; }
bool operator>=(int a, const BigInt& b) { return (a <=> b) >= 0; }
bool operator==(int a, const BigInt& b) { return (a <=> b) == 0; }
bool operator!=(int a, const BigInt& b) { return (a <=> b) != 0; }

bool operator<(long double a, const BigInt& b) { return (a <=> b) < 0; }
bool operator<=(long double a, const BigInt& b) { return (a <=> b) <= 0; }
bool operator>(long double a, const BigInt& b) { return (a <=> b) > 0; }
bool operator>=(long double a, const BigInt& b) { return (a <=> b) >= 0; }
bool operator==(long double a, const BigInt& b) { return (a <=> b) == 0; }
bool operator!=(long double a, const BigInt& b) { return (a <=> b) != 0; }

void BigInt::WriteRawReprToStream(ostream& out) const {
    out << "BigInt( { ";
    bool first = true;
    for (uint32_t i : v) {
        if (!first) out << ", ";
        first = false;
        out << i;
    }
    out << " }, sign = " << sign << " }";
}

ostream& operator<<(ostream& out, const BigInt& a) {
    a.WriteRawReprToStream(out);
    return out;
}

std::string BigInt::RawRepr() const {
    stringstream ss;
    WriteRawReprToStream(ss);
    return ss.str();
}

std::string to_string(const BigInt& a) { return a.RawRepr(); }
