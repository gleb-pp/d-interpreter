#pragma once
#include <compare>
#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace dinterp {

class ZeroDivisionException : public std::runtime_error {
public:
    ZeroDivisionException();
    ~ZeroDivisionException() override = default;
};

class BigInt {
private:
    std::vector<std::uint32_t> v;
    bool sign;
    void initBigEndianRepr(const std::vector<size_t>& bigEndianRepr, size_t base);
    void normalize();
    BigInt(const std::vector<std::uint32_t>& v, bool sign);

public:
    BigInt(long val);
    explicit BigInt(size_t val);
    explicit BigInt(int val);
    BigInt();
    BigInt(const std::string& repr, size_t base);
    BigInt(const std::vector<size_t>& bigEndianRepr, size_t base);
    BigInt(const BigInt& other) = default;
    BigInt(BigInt&& other) = default;
    explicit BigInt(long double val);
    BigInt& operator=(long val);
    BigInt& operator=(size_t val);
    BigInt& operator=(const BigInt& other) = default;
    BigInt& operator=(BigInt&& other) = default;
    std::vector<size_t> Repr(size_t base) const;
    std::string ToString(size_t base = 10) const;
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
    BigInt operator-() const;
    BigInt& Negate();
    BigInt& operator++();
    BigInt operator++(int);
    BigInt& operator--();
    BigInt operator--(int);
    std::optional<std::pair<BigInt, BigInt>> DivMod(const BigInt& other) const;
    std::optional<BigInt> DivLeaveMod(const BigInt& other);
    int operator<=>(const BigInt& other) const;
    int operator<=>(long other) const;
    int operator<=>(size_t other) const;
    int operator<=>(int other) const;
    std::partial_ordering operator<=>(long double other) const;
    long double ToFloat() const;
    long ClampToLong() const;
    bool operator<(const BigInt& other) const;
    bool operator<(long other) const;
    bool operator<(size_t other) const;
    bool operator<(int other) const;
    bool operator<(long double other) const;
    bool operator<=(const BigInt& other) const;
    bool operator<=(long other) const;
    bool operator<=(size_t other) const;
    bool operator<=(int other) const;
    bool operator<=(long double other) const;
    bool operator>(const BigInt& other) const;
    bool operator>(long other) const;
    bool operator>(size_t other) const;
    bool operator>(int other) const;
    bool operator>(long double other) const;
    bool operator>=(const BigInt& other) const;
    bool operator>=(long other) const;
    bool operator>=(size_t other) const;
    bool operator>=(int other) const;
    bool operator>=(long double other) const;
    bool operator==(const BigInt& other) const;
    bool operator==(long other) const;
    bool operator==(size_t other) const;
    bool operator==(int other) const;
    bool operator==(long double other) const;
    bool operator!=(const BigInt& other) const;
    bool operator!=(long other) const;
    bool operator!=(size_t other) const;
    bool operator!=(int other) const;
    bool operator!=(long double other) const;
    operator bool() const;
    bool IsNegative() const;
    size_t SignificantBits() const;
    std::string RawRepr() const;
    void WriteRawReprToStream(std::ostream& out) const;
};

int operator<=>(long a, const BigInt& b);
int operator<=>(size_t a, const BigInt& b);
int operator<=>(int a, const BigInt& b);
std::partial_ordering operator<=>(long double a, const BigInt& b);
bool operator<(long a, const BigInt& b);
bool operator<=(long a, const BigInt& b);
bool operator>(long a, const BigInt& b);
bool operator>=(long a, const BigInt& b);
bool operator==(long a, const BigInt& b);
bool operator!=(long a, const BigInt& b);

bool operator<(long double a, const BigInt& b);
bool operator<=(long double a, const BigInt& b);
bool operator>(long double a, const BigInt& b);
bool operator>=(long double a, const BigInt& b);
bool operator==(long double a, const BigInt& b);
bool operator!=(long double a, const BigInt& b);

bool operator<(int a, const BigInt& b);
bool operator<=(int a, const BigInt& b);
bool operator>(int a, const BigInt& b);
bool operator>=(int a, const BigInt& b);
bool operator==(int a, const BigInt& b);
bool operator!=(int a, const BigInt& b);

bool operator<(size_t a, const BigInt& b);
bool operator<=(size_t a, const BigInt& b);
bool operator>(size_t a, const BigInt& b);
bool operator>=(size_t a, const BigInt& b);
bool operator==(size_t a, const BigInt& b);
bool operator!=(size_t a, const BigInt& b);

std::string to_string(const BigInt& a);
std::ostream& operator<<(std::ostream& out, const BigInt& a);
}
