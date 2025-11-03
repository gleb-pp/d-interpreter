#include "runtime/types.h"

#include <memory>
#include <sstream>
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
bool Type::BinaryEq(const Type& other) const { return {}; }
bool Type::BinaryOrdering(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryMinus() const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryPlus() const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryNot() const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Field(const std::string& name) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Field(const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Subscript(const Type& other) const { return {}; }

bool IntegerType::TypeEq(const Type& other) const { return !!dynamic_cast<const IntegerType*>(&other); }

std::string IntegerType::Name() const { return "int"; }

static bool IsRealOrInt(const Type& t) { return t.TypeEq(IntegerType()) || t.TypeEq(RealType()); }

static optional<shared_ptr<Type>> NumericArith(const Type& a, const Type& b) {
    auto unknown = make_shared<UnknownType>();
    if (a.TypeEq(*unknown)) return NumericArith(b, a);
    auto inttype = make_shared<IntegerType>();
    auto realtype = make_shared<RealType>();
    bool inta, intb;
    if (a.TypeEq(*inttype))
        inta = true;
    else if (a.TypeEq(*realtype))
        inta = false;
    else
        return {};
    if (b.TypeEq(*unknown)) {
        if (inta) return unknown;
        return realtype;
    }
    if (b.TypeEq(*inttype))
        intb = true;
    else if (b.TypeEq(*realtype))
        intb = false;
    else
        return {};
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

bool IntegerType::BinaryEq(const Type& other) const {
    return IsRealOrInt(other) || other.TypeEq(UnknownType());
}

bool IntegerType::BinaryOrdering(const Type& other) const {
    return IsRealOrInt(other) || other.TypeEq(UnknownType());
}

std::optional<std::shared_ptr<Type>> IntegerType::UnaryMinus() const { return make_shared<IntegerType>(); }

std::optional<std::shared_ptr<Type>> IntegerType::UnaryPlus() const { return make_shared<IntegerType>(); }

std::optional<std::shared_ptr<Type>> IntegerType::Field(const std::string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerType>();
    if (name == "Frac") return make_shared<RealType>();
    return {};
}

bool RealType::TypeEq(const Type& other) const { return !!dynamic_cast<const RealType*>(&other); }

std::string RealType::Name() const { return "real"; }

std::optional<std::shared_ptr<Type>> RealType::BinaryPlus(const Type& other) const {
    return NumericArith(*this, other);
}

std::optional<std::shared_ptr<Type>> RealType::BinaryMinus(const Type& other) const {
    return NumericArith(*this, other);
}

std::optional<std::shared_ptr<Type>> RealType::BinaryMul(const Type& other) const { return NumericArith(*this, other); }

std::optional<std::shared_ptr<Type>> RealType::BinaryDiv(const Type& other) const { return NumericArith(*this, other); }

bool RealType::BinaryEq(const Type& other) const {
    return IsRealOrInt(other) || other.TypeEq(UnknownType());
}

bool RealType::BinaryOrdering(const Type& other) const {
    return IsRealOrInt(other) || other.TypeEq(UnknownType());
}

std::optional<std::shared_ptr<Type>> RealType::UnaryMinus() const { return make_shared<RealType>(); }

std::optional<std::shared_ptr<Type>> RealType::UnaryPlus() const { return make_shared<RealType>(); }

std::optional<std::shared_ptr<Type>> RealType::Field(const std::string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerType>();
    if (name == "Frac") return make_shared<RealType>();
    return {};
}

bool StringType::TypeEq(const Type& other) const { return !!dynamic_cast<const StringType*>(&other); }

std::string StringType::Name() const { return "string"; }

std::optional<std::shared_ptr<Type>> StringType::BinaryPlus(const Type& other) const {
    if (other.TypeEq(UnknownType())) return make_shared<StringType>();
    if (TypeEq(other)) return make_shared<StringType>();
    return {};
}

bool StringType::BinaryEq(const Type& other) const {
    return TypeEq(other) || other.TypeEq(UnknownType());
}

bool StringType::BinaryOrdering(const Type& other) const {
    return TypeEq(other) || other.TypeEq(UnknownType());
}

std::optional<std::shared_ptr<Type>> StringType::Field(const std::string& name) const {
    if (name == "Split")
        return make_shared<FuncType>(true, vector<shared_ptr<Type>>{make_shared<StringType>()},
                                     make_shared<ArrayType>());
    if (name == "SplitWS") return make_shared<FuncType>(true, vector<shared_ptr<Type>>(), make_shared<ArrayType>());
    if (name == "Join")
        return make_shared<FuncType>(true, vector<shared_ptr<Type>>{make_shared<ArrayType>()},
                                     make_shared<StringType>());
    if (name == "Lower" || name == "Upper") return make_shared<StringType>();
    if (name == "Slice")
        return make_shared<FuncType>(true, vector<shared_ptr<Type>>(3, make_shared<IntegerType>()),
                                     make_shared<ArrayType>());
    if (name == "Length") return make_shared<IntegerType>();
    return {};
}

std::optional<std::shared_ptr<Type>> StringType::Subscript(const Type& other) const {
    if (other.TypeEq(IntegerType()) || other.TypeEq(UnknownType())) return make_shared<StringType>();
    return {};
}

bool NoneType::TypeEq(const Type& other) const { return !!dynamic_cast<const NoneType*>(&other); }

std::string NoneType::Name() const { return "none"; }

bool BoolType::TypeEq(const Type& other) const { return !!dynamic_cast<const BoolType*>(&other); }

std::string BoolType::Name() const { return "bool"; }

std::optional<std::shared_ptr<Type>> BoolType::BinaryLogical(const Type& other) const {
    if (other.TypeEq(*this)) return make_shared<BoolType>();
    return {};
}

std::optional<std::shared_ptr<Type>> BoolType::UnaryNot() const { return make_shared<BoolType>(); }

bool ArrayType::TypeEq(const Type& other) const { return !!dynamic_cast<const ArrayType*>(&other); }

std::string ArrayType::Name() const { return "[Array]"; }

std::optional<std::shared_ptr<Type>> ArrayType::BinaryPlus(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(UnknownType())) return make_shared<ArrayType>();
    return {};
}

bool ArrayType::BinaryEq(const Type& other) const {
    return TypeEq(other) || other.TypeEq(UnknownType());
}

std::optional<std::shared_ptr<Type>> ArrayType::Subscript(const Type& other) const {
    if (other.TypeEq(IntegerType()) || other.TypeEq(UnknownType())) return make_shared<UnknownType>();
    return {};
}

bool TupleType::TypeEq(const Type& other) const { return !!dynamic_cast<const TupleType*>(&other); }

std::string TupleType::Name() const { return "{Tuple}"; }

std::optional<std::shared_ptr<Type>> TupleType::BinaryPlus(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(UnknownType())) return make_shared<TupleType>();
    return {};
}

std::optional<std::shared_ptr<Type>> TupleType::Field(const std::string& name) const {
    return make_shared<UnknownType>();
}

std::optional<std::shared_ptr<Type>> TupleType::Field(const Type& other) const {
    if (other.TypeEq(IntegerType())) return make_shared<UnknownType>();
    return {};
}

FuncType::FuncType(size_t argCount, const std::shared_ptr<Type>& returnType)
    : pure(false), argTypes({vector<shared_ptr<Type>>(argCount, make_shared<UnknownType>())}), returnType(returnType) {}
FuncType::FuncType(bool pure, const std::vector<std::shared_ptr<Type>>& argTypes,
                   const std::shared_ptr<Type>& returnType)
    : pure(pure), argTypes(argTypes), returnType(returnType) {}
FuncType::FuncType() : pure(false), argTypes({}), returnType(make_shared<UnknownType>()) {}
bool FuncType::Pure() const { return pure; }
std::optional<std::vector<std::shared_ptr<Type>>> FuncType::ArgTypes() const { return argTypes; }
std::shared_ptr<Type> FuncType::ReturnType() const { return returnType; }
bool FuncType::TypeEq(const Type& other) const { return !!dynamic_cast<const FuncType*>(&other); }
std::string FuncType::Name() const {
    stringstream res;
    if (pure) res << "(pure)";
    res << "function (";
    if (argTypes) {
        bool first = true;
        for (auto& ptr : *argTypes) {
            if (!first) res << ", ";
            first = false;
            res << ptr->Name();
        }
    } else
        res << "...";
    res << ") -> " << returnType->Name();
    return res.str();
}

bool UnknownType::TypeEq(const Type& other) const { return !!dynamic_cast<const UnknownType*>(&other); }

std::string UnknownType::Name() const { return "object?"; }

std::optional<std::shared_ptr<Type>> UnknownType::BinaryPlus(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(IntegerType())) return make_shared<UnknownType>();
    shared_ptr<Type> sametypes[] = {make_shared<RealType>(), make_shared<StringType>(), make_shared<ArrayType>(),
        make_shared<TupleType>()};
    for (auto& p : sametypes) if (other.TypeEq(*p)) return p;
    return {};
}
std::optional<std::shared_ptr<Type>> UnknownType::BinaryMinus(const Type& other) const {
    if (TypeEq(other)) return make_shared<UnknownType>();
    return NumericArith(*this, other);
}
std::optional<std::shared_ptr<Type>> UnknownType::BinaryMul(const Type& other) const {
    if (TypeEq(other)) return make_shared<UnknownType>();
    return NumericArith(*this, other);
}
std::optional<std::shared_ptr<Type>> UnknownType::BinaryDiv(const Type& other) const {
    if (TypeEq(other)) return make_shared<UnknownType>();
    return NumericArith(*this, other);
}
std::optional<std::shared_ptr<Type>> UnknownType::BinaryLogical(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(BoolType())) return make_shared<BoolType>();
    return {};
}
bool UnknownType::BinaryEq(const Type& other) const {
    unique_ptr<Type> allowed[] = {
        make_unique<UnknownType>(), make_unique<IntegerType>(), make_unique<RealType>(), make_unique<StringType>(),
        make_unique<ArrayType>()
    };
    for (auto& p : allowed) if (other.TypeEq(*p)) return true;
    return false;
}
bool UnknownType::BinaryOrdering(const Type& other) const {
    unique_ptr<Type> allowed[] = {
        make_unique<UnknownType>(), make_unique<IntegerType>(), make_unique<RealType>(), make_unique<StringType>()
    };
    for (auto& p : allowed) if (other.TypeEq(*p)) return true;
    return false;
}
std::optional<std::shared_ptr<Type>> UnknownType::UnaryMinus() const { return make_shared<UnknownType>(); }
std::optional<std::shared_ptr<Type>> UnknownType::UnaryPlus() const { return make_shared<UnknownType>(); }
std::optional<std::shared_ptr<Type>> UnknownType::UnaryNot() const { return make_shared<BoolType>(); }
std::optional<std::shared_ptr<Type>> UnknownType::Field(const std::string& name) const {
    return make_shared<UnknownType>();
}
std::optional<std::shared_ptr<Type>> UnknownType::Field(const Type& other) const {
    if (!TypeEq(other) && !other.TypeEq(IntegerType())) return {};
    return make_shared<UnknownType>();
}
std::optional<std::shared_ptr<Type>> UnknownType::Subscript(const Type& other) const {
    if (!TypeEq(other) && !other.TypeEq(IntegerType())) return {};
    return make_shared<UnknownType>();
}

}  // namespace runtime
