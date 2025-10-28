#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>

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
    virtual std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> UnaryMinus(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> UnaryPlus(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> UnaryNot(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> Field(const std::string& name) const;
    virtual std::optional<std::shared_ptr<Type>> Field(const Type& other) const;
    virtual std::optional<std::shared_ptr<Type>> Subscript(const Type& other) const;
    virtual ~Type() = default;
};

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
    std::optional<std::shared_ptr<Type>> Field(const Type& other) const override;
    virtual ~IntegerType() override = default;
};

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
    std::optional<std::shared_ptr<Type>> Field(const Type& other) const override;
    virtual ~RealType() override = default;
};

class StringType : public Type {
public:
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    std::optional<std::shared_ptr<Type>> BinaryPlus(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryEq(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> BinaryOrdering(const Type& other) const override;
    std::optional<std::shared_ptr<Type>> Field(const std::string& name) const override;
    std::optional<std::shared_ptr<Type>> Field(const Type& other) const override;
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
    std::optional<std::shared_ptr<Type>> UnaryNot(const Type& other) const override;
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
public:
    const size_t ArgCount;
    const std::shared_ptr<Type> ReturnType;
    FuncType(size_t argCount, const std::shared_ptr<Type>& returnType);
    FuncType();
    bool TypeEq(const Type& other) const override;
    std::string Name() const override;
    virtual ~FuncType() override = default;
};

}  // namespace runtime
