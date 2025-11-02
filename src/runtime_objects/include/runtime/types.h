#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/*
 * In subclasses of `Type`, returning {} where the return type is std::optional, means "operation not supported".
 * Returning UnknownType means that the operation could be supported.
 * If the virtual method is not overridden, the default is to return {} ("not supported").
 */

namespace runtime {

class Type {
public:
    virtual bool TypeEq(const Type& other) const = 0;
    virtual std::string Name() const = 0;
    virtual std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> BinaryMinus(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> BinaryMul(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> BinaryDiv(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> BinaryLogical(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const;        // = /=
    virtual std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const;  // < <= > >=
    virtual std::optional<std::shared_ptr<Type>> UnaryMinus(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> UnaryPlus(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> UnaryNot() const;
    virtual std::optional<std::shared_ptr<Type>> Field(const std::string& name) const;  // a.fieldname
    virtual std::optional<std::shared_ptr<Type>> Field(const Type& other) const;        // a.(2 + 3)
    virtual std::optional<std::shared_ptr<Type>> Subscript(const Type& other) const;    // a[3 + 2]
    virtual ~Type() = default;
};

/*
 * Has fields:
 * + Round: int = this
 * + Floor: int = this
 * + Ceil: int = this
 * + Frac: real = 0.0
 */
class IntegerType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryMinus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryMul(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryDiv(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryMinus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Field(const std::string& name) const override;
    virtual ~IntegerType() override = default;
};

/*
 * Has fields:
 * + Round: int
 * + Floor: int
 * + Ceil: int
 * + Frac: real
 */
class RealType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryMinus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryMul(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryDiv(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryMinus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Field(const std::string& name) const override;
    virtual ~RealType() override = default;
};

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
class StringType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Field(const std::string& name) const override;
    std::optional<std::shared_ptr<Type>> Subscript(const Type& other) const override;
    virtual ~StringType() override = default;
};

class NoneType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    virtual ~NoneType() override = default;
};

class BoolType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryLogical(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryNot() const override;
    virtual ~BoolType() override = default;
};

class ArrayType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Subscript(const Type& other) const override;
    virtual ~ArrayType() override = default;
};

class TupleType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Field(const std::string& name) const override;
    std::optional<std::shared_ptr<Type>> Field(const Type& other) const override;
    virtual ~TupleType() override = default;
};

class FuncType : public Type {
    bool pure;
    std::optional<std::vector<std::shared_ptr<Type>>> argTypes;
    std::shared_ptr<Type> returnType;

public:
    FuncType(size_t argCount, const std::shared_ptr<Type>& returnType);  // pure = false
    FuncType(bool pure, const std::vector<std::shared_ptr<Type>>& argTypes, const std::shared_ptr<Type>& returnType);
    FuncType();  // pure = false, argTypes = ?, returnType = UnknownType
    bool Pure() const;
    std::optional<std::vector<std::shared_ptr<Type>>> ArgTypes() const;
    std::shared_ptr<Type> ReturnType() const;
    bool TypeEq(const Type& other) const override;
    virtual std::string Name() const override;
    virtual ~FuncType() override = default;
};

class UnknownType : public Type {
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryMinus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryMul(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryDiv(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryLogical(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryMinus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> UnaryNot() const override;
    std::optional<std::shared_ptr<Type>> Field(const std::string& name) const override;
    std::optional<std::shared_ptr<Type>> Field(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Subscript(const Type& other) const override;
    virtual ~UnknownType() override = default;
};

}  // namespace runtime
