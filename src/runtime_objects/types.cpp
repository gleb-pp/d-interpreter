#include "runtime/types.h"
using namespace std;

/*
 * In subclasses of `Type`, returning {} where the return type is std::optional, means "operation not supported".
 * Returning UnknownType means that the operation could be supported.
 * If the virtual method is not overridden, the default is to return {} ("not supported").
 */

namespace runtime {

std::optional<std::shared_ptr<Type>> Type::BinaryPlus(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryMinus(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryMul(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryDiv(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryLogical(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryEq(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryOrdering(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryMinus(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryPlus(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryNot(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Field(const std::string& name) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Field(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Subscript(const Type& other) const { return {}; }

bool IntegerType::TypeEq(const Type& other) const {
    return !!dynamic_cast<const IntegerType*>(&other);
}

std::string IntegerType::Name() const {
    return "int";
}

static bool IsRealOrInt(const Type& t) {
    return t.TypeEq(IntegerType()) || t.TypeEq(RealType());
}

static optional<shared_ptr<Type>> NumericArith(const Type& a, const Type& b) {
    auto inttype = make_shared<IntegerType>();
    auto realtype = make_shared<RealType>();
    bool inta, intb;
    if (a.TypeEq(*inttype)) inta = true;
    else if (a.TypeEq(*realtype)) inta = false;
    else return {};
    if (b.TypeEq(*inttype)) intb = true;
    else if (b.TypeEq(*realtype)) intb = false;
    else return {};
    bool intres = inta && intb;
    return intres ? static_cast<shared_ptr<Type>>(inttype) : realtype;
}

std::optional<std::shared_ptr<Type>> IntegerType::BinaryPlus(const Type& other) const {
    return NumericArith(*this, other);
}

std::optional<std::shared_ptr<Type>> IntegerType::BinaryMinus(const Type& other) const {
    return NumericArith(*this, other);
}

std::optional<std::shared_ptr<Type>> IntegerType::BinaryMul(const Type& other) const {
    return NumericArith(*this, other);
}

std::optional<std::shared_ptr<Type>> IntegerType::BinaryDiv(const Type& other) const {
    return NumericArith(*this, other);
}

std::optional<std::shared_ptr<Type>> IntegerType::BinaryEq(const Type& other) const {
    if (IsRealOrInt(other)) return make_shared<BoolType>();
    return {};
}

std::optional<std::shared_ptr<Type>> IntegerType::BinaryOrdering(const Type& other) const {
    if (IsRealOrInt(other)) return make_shared<BoolType>();
    return {};
}

std::optional<std::shared_ptr<Type>> IntegerType::UnaryMinus(const Type& other) const {
    return make_shared<IntegerType>();
}

std::optional<std::shared_ptr<Type>> IntegerType::UnaryPlus(const Type& other) const {
    return make_shared<IntegerType>();
}

std::optional<std::shared_ptr<Type>> IntegerType::Field(const std::string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerType>();
    if (name == "Frac") return make_shared<RealType>();
    return {};
}

std::optional<std::shared_ptr<Type>> IntegerType::Field(const Type& other) const {
    return {};
}

// Implement class RealType
/*
 * Has fields:
 * + Round: int
 * + Floor: int
 * + Ceil: int
 * + Frac: real
 */

// Implement class StringType
/*
 * Has fields:
 * + Split: function (string) -> []
 * + SplitWS: function () -> []
 * + Join: function ([]) -> string
 * + Lower: string
 * + Upper: string
 * + Slice: function (int, int, int) -> string  // start, stop, step
 * + Length: int
 */

// Implement class NoneType

// Implement class BoolType

// Implement class ArrayType

// Implement class TupleType

// Implement class FuncType

// Implement class UnknownType

}  // namespace runtime
