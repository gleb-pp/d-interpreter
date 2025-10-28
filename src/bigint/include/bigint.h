#pragma once
#include <cstdint>
#include <vector>
#include <string>

class BigInt {
private:
    std::vector<std::uint32_t> v;
    bool sign;
public:
    BigInt(long val);
    BigInt();
    BigInt(const std::string& repr, size_t base);
    BigInt(const std::vector<size_t>& bigEndianRepr, size_t base);
    BigInt(const BigInt& other) = default;
    BigInt(BigInt&& other) = default;
    explicit BigInt(long double val);
    BigInt& operator=(long val);
    BigInt& operator=(const BigInt& other) = default;
    BigInt& operator=(BigInt&& other) = default;
    std::vector<size_t> Repr(size_t base);
    std::string ToString(size_t base = 10);
    BigInt& operator+=(const BigInt& other);
    BigInt operator+(const BigInt& other) const;
    BigInt& operator-=(const BigInt& other);
    BigInt operator-(const BigInt& other) const;
    BigInt& operator*=(const BigInt& other);
    BigInt operator*(const BigInt& other) const;
    BigInt& operator/=(const BigInt& other);
    BigInt operator/(const BigInt& other) const;
    BigInt& operator%=(const BigInt& other);
    BigInt operator%(const BigInt& other) const;
    std::pair<BigInt, BigInt> DivMod(const BigInt& other) const;
    int operator<=>(const BigInt& other) const;
    int operator<=>(long other) const;
    int operator<=>(long double other) const;
    long double ToFloat() const;
};

int operator<=>(long a, const BigInt& b);
int operator<=>(long double a, const BigInt& b);
