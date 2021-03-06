//===--- APValue.h - Union class for APFloat/APSInt/Complex -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the APValue class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_APVALUE_H
#define LLVM_CLANG_AST_APVALUE_H

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"

namespace clang {
  class Expr;

/// APValue - This class implements a discriminated union of [uninitialized]
/// [APSInt] [APFloat], [Complex APSInt] [Complex APFloat], [Expr + Offset].
class APValue {
  typedef llvm::APSInt APSInt;
  typedef llvm::APFloat APFloat;
public:
  enum ValueKind {
    Uninitialized,
    Int,
    Float,
    ComplexInt,
    ComplexFloat,
    LValue,
    Vector
  };
private:
  ValueKind Kind;
  
  struct ComplexAPSInt { 
    APSInt Real, Imag; 
    ComplexAPSInt() : Real(1), Imag(1) {}
  };
  struct ComplexAPFloat {
    APFloat Real, Imag;
    ComplexAPFloat() : Real(0.0), Imag(0.0) {}
  };
  
  struct LV {
    Expr* Base;
    uint64_t Offset;
  };
  struct Vec {
    APValue *Elts;
    unsigned NumElts;
    Vec() : Elts(0), NumElts(0) {}
    ~Vec() { delete[] Elts; }
  };
  
  enum {
    MaxSize = (sizeof(ComplexAPSInt) > sizeof(ComplexAPFloat) ? 
               sizeof(ComplexAPSInt) : sizeof(ComplexAPFloat))
  };
  
  /// Data - space for the largest member in units of void*.  This is an effort
  /// to ensure that the APSInt/APFloat values have proper alignment.
  void *Data[(MaxSize+sizeof(void*)-1)/sizeof(void*)];
  
public:
  APValue() : Kind(Uninitialized) {}
  explicit APValue(const APSInt &I) : Kind(Uninitialized) {
    MakeInt(); setInt(I);
  }
  explicit APValue(const APFloat &F) : Kind(Uninitialized) {
    MakeFloat(); setFloat(F);
  }
  explicit APValue(const APValue *E, unsigned N) : Kind(Uninitialized) {
    MakeVector(); setVector(E, N);
  }
  APValue(const APSInt &R, const APSInt &I) : Kind(Uninitialized) {
    MakeComplexInt(); setComplexInt(R, I);
  }
  APValue(const APFloat &R, const APFloat &I) : Kind(Uninitialized) {
    MakeComplexFloat(); setComplexFloat(R, I);
  }
  APValue(const APValue &RHS) : Kind(Uninitialized) {
    *this = RHS;
  }
  APValue(Expr* B, uint64_t O) : Kind(Uninitialized) {
    MakeLValue(); setLValue(B, O);
  }
  ~APValue() {
    MakeUninit();
  }
  
  ValueKind getKind() const { return Kind; }
  bool isUninit() const { return Kind == Uninitialized; }
  bool isInt() const { return Kind == Int; }
  bool isFloat() const { return Kind == Float; }
  bool isComplexInt() const { return Kind == ComplexInt; }
  bool isComplexFloat() const { return Kind == ComplexFloat; }
  bool isLValue() const { return Kind == LValue; }
  bool isVector() const { return Kind == Vector; }
  
  void print(llvm::raw_ostream &OS) const;
  void dump() const;
  
  APSInt &getInt() {
    assert(isInt() && "Invalid accessor");
    return *(APSInt*)(void*)Data;
  }
  const APSInt &getInt() const {
    return const_cast<APValue*>(this)->getInt();
  }
  
  APFloat &getFloat() {
    assert(isFloat() && "Invalid accessor");
    return *(APFloat*)(void*)Data;
  }
  const APFloat &getFloat() const {
    return const_cast<APValue*>(this)->getFloat();
  }
  
  APValue &getVectorElt(unsigned i) const {
    assert(isVector() && "Invalid accessor");
    return ((Vec*)(void*)Data)->Elts[i];
  }
  unsigned getVectorLength() const {
    assert(isVector() && "Invalid accessor");
    return ((Vec*)(void *)Data)->NumElts;
  }
  
  APSInt &getComplexIntReal() {
    assert(isComplexInt() && "Invalid accessor");
    return ((ComplexAPSInt*)(void*)Data)->Real;
  }
  const APSInt &getComplexIntReal() const {
    return const_cast<APValue*>(this)->getComplexIntReal();
  }
  
  APSInt &getComplexIntImag() {
    assert(isComplexInt() && "Invalid accessor");
    return ((ComplexAPSInt*)(void*)Data)->Imag;
  }
  const APSInt &getComplexIntImag() const {
    return const_cast<APValue*>(this)->getComplexIntImag();
  }
  
  APFloat &getComplexFloatReal() {
    assert(isComplexFloat() && "Invalid accessor");
    return ((ComplexAPFloat*)(void*)Data)->Real;
  }
  const APFloat &getComplexFloatReal() const {
    return const_cast<APValue*>(this)->getComplexFloatReal();
  }

  APFloat &getComplexFloatImag() {
    assert(isComplexFloat() && "Invalid accessor");
    return ((ComplexAPFloat*)(void*)Data)->Imag;
  }
  const APFloat &getComplexFloatImag() const {
    return const_cast<APValue*>(this)->getComplexFloatImag();
  }

  Expr* getLValueBase() const {
    assert(isLValue() && "Invalid accessor");
    return ((const LV*)(const void*)Data)->Base;
  }
  uint64_t getLValueOffset() const {
    assert(isLValue() && "Invalid accessor");
    return ((const LV*)(const void*)Data)->Offset;
  }
  
  void setInt(const APSInt &I) {
    assert(isInt() && "Invalid accessor");
    *(APSInt*)(void*)Data = I;
  }
  void setFloat(const APFloat &F) {
    assert(isFloat() && "Invalid accessor");
    *(APFloat*)(void*)Data = F;
  }
  void setVector(const APValue *E, unsigned N) {
    assert(isVector() && "Invalid accessor");
    ((Vec*)(void*)Data)->Elts = new APValue[N];
    ((Vec*)(void*)Data)->NumElts = N;
    for (unsigned i = 0; i != N; ++i)
      ((Vec*)(void*)Data)->Elts[i] = E[i];
  }
  void setComplexInt(const APSInt &R, const APSInt &I) {
    assert(R.getBitWidth() == I.getBitWidth() && 
           "Invalid complex int (type mismatch).");
    assert(isComplexInt() && "Invalid accessor");
    ((ComplexAPSInt*)(void*)Data)->Real = R;
    ((ComplexAPSInt*)(void*)Data)->Imag = I;
  }
  void setComplexFloat(const APFloat &R, const APFloat &I) {
    assert(&R.getSemantics() == &I.getSemantics() && 
           "Invalid complex float (type mismatch).");
    assert(isComplexFloat() && "Invalid accessor");
    ((ComplexAPFloat*)(void*)Data)->Real = R;
    ((ComplexAPFloat*)(void*)Data)->Imag = I;
  }
  void setLValue(Expr *B, uint64_t O) {
    assert(isLValue() && "Invalid accessor");
    ((LV*)(void*)Data)->Base = B;
    ((LV*)(void*)Data)->Offset = O;
  }
  
  const APValue &operator=(const APValue &RHS);
  
private:
  void MakeUninit();
  void MakeInt() {
    assert(isUninit() && "Bad state change");
    new ((void*)Data) APSInt(1);
    Kind = Int;
  }
  void MakeFloat() {
    assert(isUninit() && "Bad state change");
    new ((APFloat*)(void*)Data) APFloat(0.0);
    Kind = Float;
  }
  void MakeVector() {
    assert(isUninit() && "Bad state change");
    new ((Vec*)(void*)Data) Vec();
    Kind = Vector;
  }
  void MakeComplexInt() {
    assert(isUninit() && "Bad state change");
    new ((ComplexAPSInt*)(void*)Data) ComplexAPSInt();
    Kind = ComplexInt;
  }
  void MakeComplexFloat() {
    assert(isUninit() && "Bad state change");
    new ((ComplexAPFloat*)(void*)Data) ComplexAPFloat();
    Kind = ComplexFloat;
  }
  void MakeLValue() {
    assert(isUninit() && "Bad state change");
    new ((LV*)(void*)Data) LV();
    Kind = LValue;
  }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const APValue &V) {
  V.print(OS);
  return OS;
}
  
} // end namespace clang.

#endif
