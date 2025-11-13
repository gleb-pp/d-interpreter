#include "runtime/types.h"

#include <algorithm>
#include <memory>
#include <sstream>
using namespace std;

/*
 * In subclasses of `Type`, returning {} where the return type is std::optional, means "operation not supported".
 * Returning UnknownType means that the operation could be supported.
 * If the virtual method is not overridden, the default is to return {} ("not supported").
 */

namespace runtime {

// Type

bool Type::StrictTypeEq(const Type& other) const { return TypeEq(other); }
shared_ptr<Type> Type::Generalize(const Type& other) const {
    if (!StrictTypeEq(other)) return make_shared<UnknownType>();
    return Clone();
}
std::optional<std::shared_ptr<Type>> Type::BinaryPlus([[maybe_unused]] const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryMinus([[maybe_unused]] const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryMul([[maybe_unused]] const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryDiv([[maybe_unused]] const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::BinaryLogical([[maybe_unused]] const Type& other) const { return {}; }
bool Type::BinaryEq([[maybe_unused]] const Type& other) const { return {}; }
bool Type::BinaryOrdering([[maybe_unused]] const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryMinus() const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryPlus() const { return {}; }
std::optional<std::shared_ptr<Type>> Type::UnaryNot() const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Field([[maybe_unused]] const std::string& name) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Field([[maybe_unused]] const Type& other) const { return {}; }
std::optional<std::shared_ptr<Type>> Type::Subscript([[maybe_unused]] const Type& other) const { return {}; }

// IntegerType

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

shared_ptr<Type> IntegerType::Clone() const { return make_shared<IntegerType>(); }

bool IntegerType::BinaryEq(const Type& other) const { return IsRealOrInt(other) || other.TypeEq(UnknownType()); }

bool IntegerType::BinaryOrdering(const Type& other) const { return IsRealOrInt(other) || other.TypeEq(UnknownType()); }

std::optional<std::shared_ptr<Type>> IntegerType::UnaryMinus() const { return make_shared<IntegerType>(); }

std::optional<std::shared_ptr<Type>> IntegerType::UnaryPlus() const { return make_shared<IntegerType>(); }

std::optional<std::shared_ptr<Type>> IntegerType::Field(const std::string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerType>();
    if (name == "Frac") return make_shared<RealType>();
    return {};
}

// RealType

shared_ptr<Type> RealType::Clone() const { return make_shared<RealType>(); }

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

bool RealType::BinaryEq(const Type& other) const { return IsRealOrInt(other) || other.TypeEq(UnknownType()); }

bool RealType::BinaryOrdering(const Type& other) const { return IsRealOrInt(other) || other.TypeEq(UnknownType()); }

std::optional<std::shared_ptr<Type>> RealType::UnaryMinus() const { return make_shared<RealType>(); }

std::optional<std::shared_ptr<Type>> RealType::UnaryPlus() const { return make_shared<RealType>(); }

std::optional<std::shared_ptr<Type>> RealType::Field(const std::string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerType>();
    if (name == "Frac") return make_shared<RealType>();
    return {};
}

// StringType

shared_ptr<Type> StringType::Clone() const { return make_shared<StringType>(); }

bool StringType::TypeEq(const Type& other) const { return !!dynamic_cast<const StringType*>(&other); }

std::string StringType::Name() const { return "string"; }

std::optional<std::shared_ptr<Type>> StringType::BinaryPlus(const Type& other) const {
    if (other.TypeEq(UnknownType())) return make_shared<StringType>();
    if (TypeEq(other)) return make_shared<StringType>();
    return {};
}

bool StringType::BinaryEq(const Type& other) const { return TypeEq(other) || other.TypeEq(UnknownType()); }

bool StringType::BinaryOrdering(const Type& other) const { return TypeEq(other) || other.TypeEq(UnknownType()); }

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

// NoneType

shared_ptr<Type> NoneType::Clone() const { return make_shared<NoneType>(); }

bool NoneType::TypeEq(const Type& other) const { return !!dynamic_cast<const NoneType*>(&other); }

std::string NoneType::Name() const { return "none"; }

// BoolType

shared_ptr<Type> BoolType::Clone() const { return make_shared<BoolType>(); }

bool BoolType::TypeEq(const Type& other) const { return !!dynamic_cast<const BoolType*>(&other); }

std::string BoolType::Name() const { return "bool"; }

std::optional<std::shared_ptr<Type>> BoolType::BinaryLogical(const Type& other) const {
    if (other.TypeEq(*this)) return make_shared<BoolType>();
    return {};
}

std::optional<std::shared_ptr<Type>> BoolType::UnaryNot() const { return make_shared<BoolType>(); }

// ArrayType

shared_ptr<Type> ArrayType::Clone() const { return make_shared<ArrayType>(); }

bool ArrayType::TypeEq(const Type& other) const { return !!dynamic_cast<const ArrayType*>(&other); }

std::string ArrayType::Name() const { return "[Array]"; }

std::optional<std::shared_ptr<Type>> ArrayType::BinaryPlus(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(UnknownType())) return make_shared<ArrayType>();
    return {};
}

bool ArrayType::BinaryEq(const Type& other) const { return TypeEq(other) || other.TypeEq(UnknownType()); }

std::optional<std::shared_ptr<Type>> ArrayType::Subscript(const Type& other) const {
    if (other.TypeEq(IntegerType()) || other.TypeEq(UnknownType())) return make_shared<UnknownType>();
    return {};
}

// TupleType

shared_ptr<Type> TupleType::Clone() const { return make_shared<TupleType>(); }

bool TupleType::TypeEq(const Type& other) const { return !!dynamic_cast<const TupleType*>(&other); }

std::string TupleType::Name() const { return "{Tuple}"; }

std::optional<std::shared_ptr<Type>> TupleType::BinaryPlus(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(UnknownType())) return make_shared<TupleType>();
    return {};
}

std::optional<std::shared_ptr<Type>> TupleType::Field([[maybe_unused]] const std::string& name) const {
    return make_shared<UnknownType>();
}

std::optional<std::shared_ptr<Type>> TupleType::Field(const Type& other) const {
    if (other.TypeEq(IntegerType())) return make_shared<UnknownType>();
    return {};
}

// FuncType

FuncType::FuncType(bool pure, size_t argCount, const std::shared_ptr<Type>& returnType)
    : pure(pure), argTypes({vector<shared_ptr<Type>>(argCount, make_shared<UnknownType>())}), returnType(returnType) {}
FuncType::FuncType(bool pure, const std::vector<std::shared_ptr<Type>>& argTypes,
                   const std::shared_ptr<Type>& returnType)
    : pure(pure), argTypes(argTypes), returnType(returnType) {}
FuncType::FuncType(bool pure, const std::shared_ptr<Type>& returnType) : pure(pure), returnType(returnType) {}
FuncType::FuncType() : pure(false), argTypes({}), returnType(make_shared<UnknownType>()) {}
bool FuncType::Pure() const { return pure; }
std::optional<std::vector<std::shared_ptr<Type>>> FuncType::ArgTypes() const { return argTypes; }
std::shared_ptr<Type> FuncType::ReturnType() const { return returnType; }
bool FuncType::TypeEq(const Type& other) const { return !!dynamic_cast<const FuncType*>(&other); }
bool FuncType::StrictTypeEq(const Type& other) const {
    const FuncType* p = dynamic_cast<const FuncType*>(&other);
    if (!p) return false;
    if (pure != p->pure) return false;
    if (argTypes) {
        if (!p->argTypes) return false;
        size_t n = argTypes->size();
        if (p->argTypes->size() != n) return false;
        for (size_t i = 0; i < n; i++)
            if (argTypes->at(i)->StrictTypeEq(*p->argTypes->at(i))) return false;
    } else if (p->argTypes)
        return false;
    return returnType->StrictTypeEq(*p->returnType);
}
shared_ptr<Type> FuncType::Clone() const { return make_shared<FuncType>(*this); }
shared_ptr<Type> FuncType::Generalize(const Type& other) const {
    const FuncType* p = dynamic_cast<const FuncType*>(&other);
    if (!p) return make_shared<UnknownType>();
    bool respure = pure && p->pure;
    optional<vector<shared_ptr<Type>>> resArgs;
    if (argTypes && p->argTypes && argTypes->size() == p->argTypes->size()) {
        resArgs.emplace();
        resArgs->reserve(argTypes->size());
        std::ranges::transform(
            *argTypes, *p->argTypes, back_inserter(*resArgs),
            [](const std::shared_ptr<Type>& a, const std::shared_ptr<Type>& b) { return a->Generalize(*b); });
    }
    auto resRet = returnType->Generalize(*p->returnType);
    if (resArgs) return make_shared<FuncType>(respure, *resArgs, resRet);
    return make_shared<FuncType>(respure, resRet);
}
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

// UnknownType

shared_ptr<Type> UnknownType::Clone() const { return make_shared<UnknownType>(); }

bool UnknownType::TypeEq(const Type& other) const { return !!dynamic_cast<const UnknownType*>(&other); }

std::string UnknownType::Name() const { return "object?"; }

std::optional<std::shared_ptr<Type>> UnknownType::BinaryPlus(const Type& other) const {
    if (TypeEq(other) || other.TypeEq(IntegerType())) return make_shared<UnknownType>();
    shared_ptr<Type> sametypes[] = {make_shared<RealType>(), make_shared<StringType>(), make_shared<ArrayType>(),
                                    make_shared<TupleType>()};
    for (auto& p : sametypes)
        if (other.TypeEq(*p)) return p;
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
    unique_ptr<Type> allowed[] = {make_unique<UnknownType>(), make_unique<IntegerType>(), make_unique<RealType>(),
                                  make_unique<StringType>(), make_unique<ArrayType>()};
    for (auto& p : allowed)
        if (other.TypeEq(*p)) return true;
    return false;
}
bool UnknownType::BinaryOrdering(const Type& other) const {
    unique_ptr<Type> allowed[] = {make_unique<UnknownType>(), make_unique<IntegerType>(), make_unique<RealType>(),
                                  make_unique<StringType>()};
    for (auto& p : allowed)
        if (other.TypeEq(*p)) return true;
    return false;
}
std::optional<std::shared_ptr<Type>> UnknownType::UnaryMinus() const { return make_shared<UnknownType>(); }
std::optional<std::shared_ptr<Type>> UnknownType::UnaryPlus() const { return make_shared<UnknownType>(); }
std::optional<std::shared_ptr<Type>> UnknownType::UnaryNot() const { return make_shared<BoolType>(); }
std::optional<std::shared_ptr<Type>> UnknownType::Field([[maybe_unused]] const std::string& name) const {
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
