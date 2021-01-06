#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include <stack>
#include "symbol.hpp"
#include "lexer.hpp"
#include "llvm.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>


using namespace std;


inline std::ostream& operator<<(std::ostream &out, Types t) {
  switch (t) {
  case TYPE_int: out << "int"; break;
  case TYPE_bool: out << "bool"; break;
  case TYPE_real: out << "real"; break;
  case TYPE_char: out << "char"; break;
  case TYPE_array: out << "array"; break;
  case TYPE_string: out << "string"; break;
  case TYPE_pointer: out << "pointer"; break;
  case TYPE_result: out << "result"; break;
  case TYPE_proc: out << "procedure"; break;
  case TYPE_nil: out << "nil"; break;
  case TYPE_label: out << "label"; break;
  }
  return out;
}

class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
  virtual std::string getstring() { return "AST()";}
  virtual void sem() {}
  virtual void semfor(){}
  virtual Value* compile() const = 0;
  void llvm_compile_and_dump(){
    // Initialize the module and the optimization passes.
    TheModule = make_unique<Module>("pcl program", TheContext);
    TheFPM = make_unique<legacy::FunctionPassManager>(TheModule.get());
    TheFPM->add(createPromoteMemoryToRegisterPass());
    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createReassociatePass());
    TheFPM->add(createGVNPass());
    TheFPM->add(createCFGSimplificationPass());
    TheFPM->doInitialization();

    /***********declare lib functions***********/

    // declare void @writeInteger(i64)
    FunctionType *writeInteger_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                        std::vector<Type *> { i64 }, false);
    TheWriteInteger =
      Function::Create(writeInteger_type, Function::ExternalLinkage,
                       "writeInteger", TheModule.get());
    // declare void @writeReal(DoubleTyID)
    FunctionType *writeReal_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                std::vector<Type *> { DoubleTyID }, false);
    TheWriteReal =
      Function::Create(writeReal_type, Function::ExternalLinkage,
                "writeReal", TheModule.get());
    // declare void @writeBoolean(i1)
    FunctionType *writeBoolean_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                        std::vector<Type *> { i32 }, false);
    TheWriteBoolean =
      Function::Create(writeBoolean_type, Function::ExternalLinkage,
                       "writeBoolean", TheModule.get());
    // declare void @writeChar(i8)
    FunctionType *writeChar_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                        std::vector<Type *> { i8 }, false);
    TheWriteChar =
      Function::Create(writeChar_type, Function::ExternalLinkage,
                       "writeChar", TheModule.get());
    // declare void @writeString(i8*)
    FunctionType *writeString_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                        std::vector<Type *> { PointerType::get(i8, 0) }, false);
    TheWriteString =
      Function::Create(writeString_type, Function::ExternalLinkage,
                       "writeString", TheModule.get());
    //declare int @readInteger(i32)
    FunctionType *readInteger_type =
        FunctionType::get(i32, std::vector<llvm::Type *>{}, false);
    TheReadInteger =
      Function::Create(readInteger_type, Function::ExternalLinkage,
                       "readInteger", TheModule.get());
    //declare int @readBoolean(i32)
    FunctionType *readBoolean_type =
        FunctionType::get(i32, std::vector<llvm::Type *>{}, false);
    TheReadBoolean =
      Function::Create(readBoolean_type, Function::ExternalLinkage,
                       "readBoolean", TheModule.get());
    //declare int @readChar(i8)
    FunctionType *readChar_type =
        FunctionType::get(i32, std::vector<llvm::Type *>{}, false);
    TheReadChar =
      Function::Create(readChar_type, Function::ExternalLinkage,
                       "readChar", TheModule.get());
    //declare int @readString(*i8)
    FunctionType *readString_type =
        FunctionType::get(PointerType::get(i8, 0), std::vector<llvm::Type *>{}, false);
    TheReadString =
      Function::Create(readString_type, Function::ExternalLinkage,
                       "readString", TheModule.get());
    
    // declare i32 @abs(i32)
    FunctionType *abs_type =
      FunctionType::get(i32,
                        std::vector<Type *> { i32 }, false);
    TheAbs =
      Function::Create(abs_type, Function::ExternalLinkage,
                       "abs", TheModule.get());

    FunctionType *math_real_type =
      FunctionType::get(DoubleTyID,
                        std::vector<Type *> { DoubleTyID }, false);
    //declare DoubleTyID @fabs(DoubleTyID)
    TheFabs =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "fabs", TheModule.get());
    //declare DoubleTyID @sqrt(DoubleTyID)
    TheSqrt =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "sqrt", TheModule.get());
    //declare DoubleTyID @sin(DoubleTyID)
    TheSin =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "sin", TheModule.get());
    //declare DoubleTyID @cos(DoubleTyID)
    TheCos =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "cos", TheModule.get());
    //declare DoubleTyID @tan(DoubleTyID)
    TheTan =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "tan", TheModule.get());
    //declare DoubleTyID @atan(DoubleTyID)
    TheArctan =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "atan", TheModule.get());
    //declare DoubleTyID @exp(DoubleTyID)
    TheExp =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "exp", TheModule.get());
    //declare DoubleTyID @ln(DoubleTyID)
    TheLn =
      Function::Create(math_real_type, Function::ExternalLinkage,
                       "ln", TheModule.get());
    //declare DoubleTyID @pi()
    FunctionType *pi_type =
        FunctionType::get(DoubleTyID, std::vector<llvm::Type *>{}, false);
    ThePi =
      Function::Create(pi_type, Function::ExternalLinkage,
                       "pi", TheModule.get());
    FunctionType *conv_type =
    	FunctionType::get(i32, std::vector<llvm::Type *> { DoubleTyID }, false);
    //declare i32 @trunc(DoubleTyID)
    TheTrunc =
      Function::Create(conv_type, Function::ExternalLinkage,
      	               "trunc", TheModule.get());
    //declare i32 @round(DoubleTyID)
    TheRound =
      Function::Create(conv_type, Function::ExternalLinkage,
      	               "round", TheModule.get());
    //declare i32 @ord(i8)
    FunctionType *ord_type =
    	FunctionType::get(i32, std::vector<llvm::Type *> { i8 }, false);
    TheOrd =
      Function::Create(ord_type, Function::ExternalLinkage,
      	               "ord", TheModule.get());
    //declare i8 @chr(i32)
    FunctionType *chr_type =
    	FunctionType::get(i8, std::vector<llvm::Type *> { i32 }, false);
    TheChr =
      Function::Create(chr_type, Function::ExternalLinkage,
      	               "chr", TheModule.get());



    // Define and start the main function.
    loggedinfo.openScope();
    Function *main =
      cast<Function>(TheModule->getOrInsertFunction("main", i32));
    loggedinfo.addFunctionInScope("main", main);
    // // Emit the program code.
    compile();
    // Verify the IR.
    bool bad = verifyModule(*TheModule, &errs());
    if (bad) {
      std::cerr << "The IR is bad!" << std::endl;
      std::exit(1);
    }
    TheFPM->run(*main);
    // Print out the IR.
    TheModule->print(outs(), nullptr);
  }
protected:
  // Global LLVM variables related to the LLVM suite.
  static LLVMContext TheContext;
  static IRBuilder<> Builder;
  static std::unique_ptr<Module> TheModule;
  static std::unique_ptr<legacy::FunctionPassManager> TheFPM;

  static Function *TheWriteInteger;
  static Function *TheWriteBoolean;
  static Function *TheWriteChar;
  static Function *TheWriteReal;
  static Function *TheWriteString;
  static Function *TheReadInteger;
  static Function *TheReadBoolean;
  static Function *TheReadChar;
  static Function *TheReadString;
  static Function *TheAbs;
  static Function *TheFabs;
  static Function *TheSqrt;
  static Function *TheSin;
  static Function *TheCos;
  static Function *TheTan;
  static Function *TheArctan;
  static Function *TheExp;
  static Function *ThePi;
  static Function *TheLn;
  static Function *TheTrunc;
  static Function *TheRound;
  static Function *TheOrd;
  static Function *TheChr;

  // Useful LLVM types.
  static Type *i1;
  static Type *i8;
  static Type *i32;
  static Type *i64;
  static Type *DoubleTyID;

  // Useful LLVM helper functions.
  ConstantInt* c1(char c) const {
    return ConstantInt::get(TheContext, APInt(1, c, true));
  }
  ConstantInt* c8(char c) const {
    return ConstantInt::get(TheContext, APInt(8, c, true));
  }
  ConstantInt* c32(int n) const {
    return ConstantInt::get(TheContext, APInt(32, n, true));
  }
  ConstantFP* fp32(double d) const {
    return ConstantFP::get(TheContext, APFloat(d));
  }
  // calculate variable address
  virtual Value *calcAddr (std::string id, std::string function, Value * index = nullptr) const {
    
    llvm::Value *addr;
    llvm::Type *t;
    // dereference if necessary
    if (loggedinfo.isPointer(id)) {
      
      addr = Builder.CreateLoad(loggedinfo.getVarAlloca(id));
      t = loggedinfo.getVarType(id)->getPointerElementType();
    }
    else {
      
      addr = loggedinfo.getVarAlloca(id);
      t = loggedinfo.getVarType(id);
    }
    
    // id is an array
    if (t->isArrayTy()){
      
      addr = Builder.CreateGEP(addr, vector<llvm::Value *>{c32(0), c32(0)});
    }

    
    else{
      
      addr = Builder.CreateGEP(addr, c32(0));
    }

    // id is a variable
    if (addr) {
          
    }

    // variable is element of array (e.g. a[2])
    if (index){
      
      if (t->isArrayTy()) {
        
        addr = Builder.CreateGEP(addr, index);
      }
    }
    return addr;
  }
  // checks if id is the name of a library defined function/proc
  virtual bool isLibFunc(std::string id) const{
    return id == "writeInteger" or id == "writeBoolean" or id == "writeReal" or id == "writeChar" or
             id == "writeString" or id == "readInteger" or id == "readBoolean" or
             id == "readChar" or id == "readString" or id == "readReal" or
             id == "abs" or id == "fabs" or id == "sqrt" or id == "sin" or
             id == "cos" or id == "tan" or id == "arctan" or id == "exp" or
             id == "ln" or id == "pi" or id == "trunc" or id == "round" or
             id == "ord" or id == "chr";
  }
  // function that translates symbol table types to llvm types
  virtual Type *type_to_llvm(Types type, std::string pm = "PASS_BY_VALUE") const {
    llvm::Type *llvmtype;
    switch (type) {
      case TYPE_bool:
        llvmtype = i1;
        break;
      case TYPE_int:
        llvmtype = i32;
        break;
      case TYPE_char:
        llvmtype = i8;
        break;
      default: std::cerr << "cannot cast semantic type to LLVM type";
    }
    if (pm.compare("PASS_BY_REFERENCE") == 0) return llvmtype->getPointerTo();
    return llvmtype;
  }
  // dereferencing function
  virtual llvm::Value *deref (llvm::Value *var) const{
    while (var->getType()->getPointerElementType()->isPointerTy())
      var = Builder.CreateLoad(var);
    return var;
  }
};

class Type_not_from_llvm: public AST{
public:
  virtual void printOn(std::ostream &out) const override {
    out << "Type()";
  }
  virtual bool operator==(const Type_not_from_llvm &that) const { return false; }
  virtual Types get_type() const {
    return value;
  }
  virtual Types get_oftype() const {
  	return value;
  }
  virtual int get_size() const {
  	return 0;
  }

  virtual Value* compile() const override { return 0;}
private:
  Types value;
};

class Int: public Type_not_from_llvm{
public:
  Int(){
    value = TYPE_int;
  }
  virtual Types get_type() const{
    return value;
  }
private:
  Types value;
};

class Real: public Type_not_from_llvm{
public:
  Real(){
    value = TYPE_real;
  }
  virtual Types get_type() const{
    return value;
  }
private:
  Types value;
};

class Char: public Type_not_from_llvm{
public:
  Char(){
    value = TYPE_char;
  }
  virtual Types get_type() const{
    return value;
  }
private:
  Types value;
};

class Bool: public Type_not_from_llvm{
public:
  Bool(){
    value = TYPE_bool;
  }
  virtual Types get_type() const{
    return value;
  }
private:
  Types value;
};


class Array: public Type_not_from_llvm {
public:
  Array(Types t, int s = -1){
    value = TYPE_array;
    oftype = t;
    size = s;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Array(size:";
    out << size;
    out <<",type:"; 
   out << oftype;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Array(size:";
    s += size;
    s += ",type: ";
    s += oftype;
    s += ")";
    return s;
  }
  virtual bool operator==(const Array &that) const {
    if(that.value == TYPE_array){
      if(oftype == that.oftype && size == that.size) {
        return true;
      }
    }
    return false;
  }
  virtual Types get_type() const{
  	return TYPE_array;
  }
  virtual int get_size() const {
  	return size;
  }
  virtual Types get_oftype() const{
    return oftype;
  }
private:
  Types value;
  Types oftype;
  int size;
};

class Pointer: public Type_not_from_llvm {
public:
  Pointer(Types t){
    value = TYPE_pointer;
    oftype = t;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Pointer(type:" << oftype << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Pointer(type:";
    s += oftype;
    s += ")";
    return s;
  }
  virtual bool operator==(const Pointer &that) const {
    if (that.value == TYPE_nil){
      return true;
    }
    if (that.value == TYPE_pointer){
      if(oftype == that.oftype) {
        return true;
      }
    }
    return false;
  }
  virtual Types get_type() const{
  	return TYPE_pointer;
  }
  virtual Types get_oftype() const{
    return oftype;
  }
private:
  Types value;
  Types oftype;
  int size;
};

inline std::ostream& operator<<(std::ostream &out, const AST &t) {
  t.printOn(out);
  return out;
}

extern std::vector<int> rt_stack;

class Stmt: public AST {
public:
  	virtual void printOn(std::ostream &out) const = 0;
};

class Stmtlist: public AST {
public:
  Stmtlist(): stmt_list(){}
  ~Stmtlist() {
    for (Stmt *s : stmt_list) delete s;
  }
  void append(Stmt* s) { if (s) stmt_list.push_back(s); }
  void append_first(Stmt* s) {if (s) stmt_list.insert(stmt_list.begin(), s); }
  virtual void printOn(std::ostream &out) const override {
    out << "\nStmtlist(";
    for (Stmt *s : stmt_list) {
      	s->printOn(out);
    	out << "\n\t";
    }
    out << "\n)";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Stmtlist(";
    for (Stmt *st : stmt_list) {
      s += st->getstring();
      s += ", ";
    }
    s += ")";
    return s;
  }
  virtual void sem() override {
    for (Stmt *s : stmt_list) s->sem();
  }
  virtual Value* compile() const override {
    for (Stmt *s : stmt_list) { s->compile();}
    return nullptr;
  }
  std::vector<Stmt *> getlist() {
    return stmt_list;
  }
private:
  std::vector<Stmt *> stmt_list;
};

class Expr: public AST {
public:
  virtual int eval() const = 0;
  bool type_check(Types t) {
    //sem();
    if (type != t) {
      return 0;
    }
    else {
      return 1;
    }
  }
  virtual Types get_type(){
    return type;
  }
  void set_type(Types t){
    type = t;
  }
  Pointer *get_pointer(){
    return p;
  }
  Array *get_array(){
    return a;
  }
  void set_pointer(Pointer *poin){
    p = poin;
  }
  void set_array(Array *arr){
    a = arr;
  }                         //                                       _ 0 _
  bool isresult(){ //it doesnt work for assign if we have false here  \|/
  	return false;
  }
  virtual bool isArElement() {
    return false;
  }
  virtual std::string get_char_var(){ return 0; }
  virtual Value* get_offset() { return nullptr; }
  virtual Value* compile() const override { return nullptr;}
private:
  Types type;
  Pointer *p;
  Array *a;
};

class Exprlist: public AST {
public:
  Exprlist(): exprlist(){}
  ~Exprlist() {
    for (Expr *e : exprlist) delete e;
  }
  void append(Expr *e) { exprlist.push_back(e);}
  void append_first(Expr *e) { exprlist.insert(exprlist.begin(), e); }
  virtual void printOn(std::ostream &out) const override {
    out << "Exprlist(";
    for (Expr *e : exprlist) {
      e->printOn(out);
      out << ", ";
    }
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Exprlist(";
    for (Expr *e : exprlist) {
      s += e->getstring();
      s += ", ";
    }
    s += ")";
    return s;
  }
  std::vector<Expr *> getlist() {
    return exprlist;
  }
  std::vector<Value *> get_compiled_list(){
  	std::vector<Value *> l;
  	for (Expr *e : exprlist){
  		l.push_back(e->compile());
  	}
  	return l;
  }
  virtual void sem() override{
    for (Expr *e : exprlist) e->sem();
  }
  virtual Value* compile() const override { 
  	for (Expr *e : exprlist){
  		e->compile();
  	}
  	return nullptr;
  }
private:
  std::vector<Expr *> exprlist;
};

class Rvalue: public Expr {
public:
  virtual int eval() const override {   //unreachable
    return -1;
  }
};

class Lvalue: public Expr {
public:
  virtual int eval() const override {   //unreachable
    return -1;
  }
  virtual bool isresult() {
    return false;
  }
};

class Result: public Lvalue {
public:
  Result(): var("result"), offset(-1){ type = TYPE_result;}
  virtual void printOn(std::ostream &out) const override {
    out << "Result(";
    out << var;
    out << "offset: ";
    out << offset;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Result(";
    s += var;
    s += "offset: ";
    s += offset;
    s += ")";
    return s;
  }
  virtual int eval() const override {
    return rt_stack[offset];
  }
  virtual std::string get_char_var() override{
  	return "@";
  }
  virtual Types get_type() const{
    return TYPE_result;
  }
  virtual bool isresult() {
    return true;
  }
  virtual void sem() override{ }
  virtual Value* compile() const override {
  	return nullptr;
  }
private:
  Types type;
  std::string var;
  int offset;
};

class BinOp: public Rvalue {
public:
  BinOp(Expr *l, char *o, Expr *r): left(l), op(o), right(r) {}
    ~BinOp() { delete left; delete right; }
    virtual void printOn(std::ostream &out) const override {
      out << "BinOP(";
      if (op) out << op;
      left->printOn(out);
      out << ", ";
      right->printOn(out);
      out << ")";
    }
    virtual std::string getstring() override{
      std::string s = "";
      s += "BinOp(";
      s += op;
      s += left->getstring();
      s += ",";
      s += right->getstring();
      s += ")";
      return s;
    }
  virtual int eval() const override {
    if (! strcmp(op,"+")) return left->eval() + right->eval();
      if (! strcmp(op,"-")) return left->eval() - right->eval();
      if (! strcmp(op,"*")) return left->eval() * right->eval();
      if (! strcmp(op,"/")) return left->eval() / right->eval();
      if (! strcmp(op,"%")) return left->eval() % right->eval();
      if (! strcmp(op,"=")) return left->eval() == right->eval();
      if (! strcmp(op,"<")) return left->eval() < right->eval();
      if (! strcmp(op,">")) return left->eval() > right->eval();
      if (! strcmp(op,"<=")) return left->eval() <= right->eval();
      if (! strcmp(op,">=")) return left->eval() >= right->eval();
      if (! strcmp(op,"<>")) return left->eval() != right->eval();
      if (! strcmp(op,"div")) return left->eval() / right->eval();
      if (! strcmp(op,"mod")) return left->eval() % right->eval();
      if (! strcmp(op,"or")) return left->eval() || right->eval();
      if (! strcmp(op,"and")) return left->eval() && right->eval();
      return 0;  // this will never be reached.
    }
  virtual void sem() override {
      left->sem();
      right->sem();

      if (left->get_type() == TYPE_result){
        left->set_type(st.lookup("result")->type);
      }

      if (right->get_type() == TYPE_result){
        right->set_type(st.lookup("result")->type);
      }

      if ((! strcmp(op,"+")) || (!strcmp(op,"-")) || (!strcmp(op,"*"))){ //make sure both operands have the same "number type" 
        if (left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          type = new Int();
        }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_real)){
            type = new Real();
          }
          else if (left->type_check(TYPE_int) && right->type_check(TYPE_real)){
            type = new Real();
          }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_int)){
            type = new Real();
          }
          else {
            std::cerr << "Type mismatch for BinOp " << op << std::endl;
          }
        }
        if (! strcmp(op,"/")) //make sure that left and right are numbers (ints or reals)
        {
          if (left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          type = new Real();
        }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_real)){
            type = new Real();
          }
          else if (left->type_check(TYPE_int) && right->type_check(TYPE_real)){
            type = new Real();
          }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_int)){
            type = new Real();
          }
          else {
            std::cerr << "Type mismatch for BinOp " << op << std::endl;
          }
        }
        if ((! strcmp(op,"mod")) || (!strcmp(op,"div"))){
          if (left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          type = new Int();
        }
        else {
            std::cerr << "Type mismatch for BinOp " << op << std::endl;
          }
        }
      if ((! strcmp(op,"<")) || (!strcmp(op,">")) || (!strcmp(op,"<=")) || (!strcmp(op,">="))){ //both operands must be numbers
          if (left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          type = new Bool();
        }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_real)){
            type = new Bool();
          }
          else if (left->type_check(TYPE_int) && right->type_check(TYPE_real)){
            type = new Bool();
          }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_int)){
            type = new Bool();
          }
          else {
            std::cerr << "Type mismatch for BinOp " << op << std::endl;
          } 
      }
      if ((! strcmp(op,"=")) || (!strcmp(op,"<>"))){ //opernads must be both numbers or of the same type but not arrays
        if (left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          type = new Bool();
        }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_real)){
            type = new Bool();
          }
          else if (left->type_check(TYPE_int) && right->type_check(TYPE_real)){
            type = new Bool();
          }
          else if (left->type_check(TYPE_real) && right->type_check(TYPE_int)){
            type = new Bool();
          }
          else if ((left->get_type() == right->get_type()) && (left->get_type() != TYPE_array)){
            type = new Bool();
          }
          else {
            std::cerr << "Type mismatch for BinOp " << op << std::endl;
          }
      }
      if ((! strcmp(op,"or")) || (!strcmp(op,"and"))){ //operands must be both booleans
        if (left->type_check(TYPE_bool) && right->type_check(TYPE_bool)){
          type = new Bool();
        }
      }
    }
  virtual Value* compile() const override {
      Value *l = left->compile();
      Value *r = right->compile();

      if (! strcmp(op,"+")){
        if((left->type_check(TYPE_real)) && (right->type_check(TYPE_real))) return Builder.CreateFAdd(l, r, "addftmp");
        return Builder.CreateAdd(l, r, "addtmp");
      }
      if (! strcmp(op,"-")){
        if((left->type_check(TYPE_real)) && (right->type_check(TYPE_real))) return Builder.CreateFSub(l, r, "subftmp");
        return Builder.CreateSub(l, r, "subtmp");
      }
      if (! strcmp(op,"*")){
        if((left->type_check(TYPE_real)) && (right->type_check(TYPE_real))) return Builder.CreateFMul(l, r, "mulftmp");
        return Builder.CreateMul(l, r, "multmp");
      }
      if (! strcmp(op, "/")) return Builder.CreateFDiv(l, r, "divftmp");
      if (! strcmp(op, "=")){
        if(left->type_check(TYPE_real) && right->type_check(TYPE_real)){
          return Builder.CreateFCmpOEQ(l, r, "feqtmp"); // OEQ = ordered
        }
        else if(left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          Value *v = Builder.CreateICmpEQ(l, r, "eqtmp");
          v->print(errs());
          return Builder.CreateICmpEQ(l, r, "eqtmp");
        }
      }
      if (! strcmp(op, "<")){
        if(left->type_check(TYPE_real) && right->type_check(TYPE_real)){
          return Builder.CreateFCmpOLT(l, r, "flttmp"); // less than
        }
        else if(left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          return Builder.CreateICmpSLT(l, r, "lttmp");
        }
      }
      if (! strcmp(op, ">")){
        if(left->type_check(TYPE_real) && right->type_check(TYPE_real)){
          return Builder.CreateFCmpOGT(l, r, "fgttmp"); // greater than
        }
        else if(left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          return Builder.CreateICmpSGT(l, r, "gttmp");
        }
      }
      if (! strcmp(op, "<=")){
        if(left->type_check(TYPE_real) && right->type_check(TYPE_real)){
          return Builder.CreateFCmpOLE(l, r, "fletmp"); // O = ordered
        }
        else if(left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          return Builder.CreateICmpSLE(l, r, "letmp");
        }
      }
      if (! strcmp(op, ">=")){
        if(left->type_check(TYPE_real) && right->type_check(TYPE_real)){
          return Builder.CreateFCmpOGE(l, r, "fgetmp"); // OEQ = ordered
        }
        else if(left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          return Builder.CreateICmpSGE(l, r, "getmp");
        }
      }
      if (! strcmp(op, "<>")){
        if(left->type_check(TYPE_real) && right->type_check(TYPE_real)){
          return Builder.CreateFCmpONE(l, r, "fnetmp"); // OEQ = ordered
        }
        else if(left->type_check(TYPE_int) && right->type_check(TYPE_int)){
          return Builder.CreateICmpNE(l, r, "netmp");
        }
      }
      if (! strcmp(op, "div")) return Builder.CreateSDiv(l, r, "divtmp");
      if (! strcmp(op, "mod")) return Builder.CreateSRem(l, r, "modtmp");
      if (! strcmp(op, "or")) return Builder.CreateOr(l, r, "ortmp");
      if (! strcmp(op, "and")) return Builder.CreateAnd(l, r, "andtmp");
      return nullptr;
    }
private:
  Type_not_from_llvm *type;
  Expr *left;
  char *op;
  Expr *right;
};

class UnOp: public Rvalue
{
public:
  UnOp(char *o, Expr *r): op(o), right(r) {}
  ~UnOp() {delete right; } 
  virtual void printOn(std::ostream &out) const override {
    out << "UnOp(";
    if (op) out << op << ",";
    right->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override{
      std::string s = "";
      s += "UnOp(";
      s += op;
      s += right->getstring();
      s += ")";
      return s;
    }
    virtual int eval() const override {
      if (! strcmp(op,"+")) return right->eval();
      if (! strcmp(op,"-")) return -right->eval();
      if (! strcmp(op,"not")) return !right->eval();
    return 0; //this will never be reached
    }
    virtual void sem() override {
      right->sem();

      if (right->get_type() == TYPE_result){
        right->set_type(st.lookup("result")->type);
      }

      if ((! strcmp(op,"+")) || (!strcmp(op,"-"))){ //operand must be number. result same type as number
        if (right->type_check(TYPE_real)){
          type = new Real();
        }
        if (right->type_check(TYPE_int)){
          type = new Int();
        }
        else {
          std::cerr << "Type mismatch for UnOp " << op << std::endl;
        }
      }
      if (! strcmp(op,"not")) { //operand must be boolean
        if (right->type_check(TYPE_bool)){
          type = new Bool();
        }
      }
    }
    virtual Value* compile() const override { 
    	Value *r = right->compile();    	
    	if (! strcmp(op,"+")) return r;
    	if (! strcmp(op,"-")) {
    		
    		if (right->type_check(TYPE_int)){
    			Value *l = c32(-1);
    			return Builder.CreateMul(l, r, "multmp");
    		}
    		else if (right->type_check(TYPE_real)){
    			Value *l = fp32(-1.0);
    			return Builder.CreateFMul(l, r, "addftmp");
    		}
    	}
    	if (! strcmp(op,"not")){
    		return Builder.CreateNot(r, "not");
    	} 
    	return nullptr;
    }
private:
  Type_not_from_llvm *type;
  char *op;
  Expr *right;  
};

class Id: public Lvalue {
public:
  Id(std::string v) {var = v; offset = -1; }
  virtual void printOn(std::ostream &out) const override {
    out << "Id(" << var << " with offset " << offset << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Id(";
    s += var;
    s += ")";
    return s;
  }
  virtual int eval() const override {
    return rt_stack[offset];
  }
  virtual std::string get_char_var() override{
  	return var;
  }
  virtual void sem() override {
    type = st.lookup(var)->type;
    SymbolEntry *en = st.lookup(var);
    offset = en->offset;
  }
  virtual Value* compile() const override {
    return Builder.CreateLoad(calcAddr(var, "Id"));
  }
private:
  Types type;
  std::string var;
  int offset;
};

class ArElement: public Lvalue {
public:
  ArElement(Expr *l, Expr *e){
    lvalue = l;
    expr = e;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "ArElement(";
    if (lvalue) lvalue->printOn(out);
    if (expr) expr->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "ArElement(";
    if (lvalue) s += lvalue->getstring();
    if (expr) s += expr->getstring();
    s += ")";
    return s;
  }
  virtual int eval() const override {
    return -1;
  }
  virtual std::string get_char_var() override{
  	std::string id = lvalue->get_char_var();
  	return id;
  }
  virtual Value * get_offset() override{
  	Value *offset = expr->compile();
  	return offset;
  }
  virtual bool isArElement() override{
    return true;
  }
  virtual void sem() override {
    lvalue->sem();
    expr->sem();

    if (lvalue->get_type() == TYPE_result) {
      lvalue->set_type(st.lookup("result")->type);
    }

    if (expr->get_type() == TYPE_result){
      expr->set_type(st.lookup("result")->type);
    }
    if (lvalue->get_type() != TYPE_array){
      printOn(std::cout);
      std::cout << " is not of type array!\n\n";
      exit(1);
    }
    else {
      if (expr->get_type() != TYPE_int) {
        printOn(std::cout);
        std::cout << " [] expr is not an integer\n";
        exit(1);
      }
    }
    consttype = lvalue->get_array()->get_oftype();
    if (consttype == TYPE_int){
      type = new Int();
    }
    if (consttype == TYPE_bool){
      type = new Bool();
    }
    if (consttype == TYPE_real){
      type = new Real();
    }
    if (consttype == TYPE_char){
      type = new Char();
    }
  }
  virtual Value* compile() const override { 
    std::string s = lvalue->get_char_var();
    return Builder.CreateLoad(calcAddr(s, "ArElement", expr->compile()));
  }
private:
  Types consttype;
  Type_not_from_llvm *type;
  Expr *lvalue;
  Expr *expr;
};

class Reference: public Rvalue {
public:
  Reference (Expr *l){
    lvalue = l;
    type = nullptr;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Reference(";
    if (lvalue) lvalue->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Reference(";
    if (lvalue) s += lvalue->getstring();
    s += ")";
    return s;
  }
  virtual int eval() const override {
    return -1;
  }
  virtual void sem() override {
    lvalue->sem();

    if (lvalue->get_type() == TYPE_result) {
      lvalue->set_type(st.lookup("result")->type);
    }

    type = new Pointer(lvalue->get_type());
  }
  virtual Value* compile() const override { 
    
  	std::string s = lvalue->get_char_var();
    return Builder.CreateLoad(calcAddr(s,"Reference", nullptr));
  }
private:
  Type_not_from_llvm *type;
  Expr *lvalue;
};

class Dereference: public Lvalue {
public:
  Dereference(Expr *e){
    expr = e;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Dereference(";
    if (expr) expr->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Dereference(";
    if (expr) s += expr->getstring();
    s += ")";
    return s;
  }
  virtual int eval() const override {
    return -1;
  }
  virtual void sem() override {
    expr->sem();
    
    if (expr->get_type() == TYPE_result) {
      expr->set_type(st.lookup("result")->type);
    }

    if (!(expr->get_type() == TYPE_pointer)){
      printOn(std::cout);
      std::cerr << "Only pointer type can be dereferenced!\n";
      std::cerr << "expression is of type " << expr->get_type();
      exit(1);
    }
    consttype = expr->get_pointer()->get_oftype();
    if (consttype == TYPE_int){
      type = new Int();
    }
    if (consttype == TYPE_bool){
      type = new Bool();
    }
    if (consttype == TYPE_real){
      type = new Real();
    }
    if (consttype == TYPE_char){
      type = new Char();
    }
  }
  virtual Value* compile() const override {
    std::string s = expr->get_char_var();
    return Builder.CreateLoad(calcAddr(s,"Dereference", nullptr));
  }
private:
  Types consttype;
  Type_not_from_llvm *type;
  Expr *expr;
};

class Assign: public Stmt {
public:
  Assign(Expr *left, Expr *e){
    lvalue = left;
    expr = e;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Assign(";
    if(lvalue && expr){
    	lvalue->printOn(out); 
    	out << " := "; 
    	expr->printOn(out);
    }
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Assign(";
    if (lvalue && expr) {s += lvalue->getstring(); s += " := "; s += expr->getstring();}
    s += ")";
    return s;
  }
  virtual void sem() override { //sth for result types
    lvalue->sem();
    expr->sem();
    if (lvalue->get_type() == TYPE_result){
      if(!st.foundResult()){
        st.insert("result", expr->get_type());
      }

      std::string fname = st.getParent();
      Types ftype = st.lookup(fname)->type;
      if(ftype == TYPE_proc){
        std::cerr << "A procedure cannot return a result (" << fname << ")\n"; 
      }

      Types resType = expr->get_type();
      if (resType == TYPE_array){
       std::cerr << "Result cannot be of type Array (" << fname <<")";
      }

      if(!(resType == ftype)){
       std::cerr << "ERROR: Type mismatch! " << fname << "is of type " << ftype << "but its result is of type " << resType;
       exit(1);
      }
      lvalue->set_type(resType);
    }
    else{
      //not result
        if (!expr->type_check(lvalue->get_type())) //{offset = lhs->offset;} //check again
        { 
          std::cerr << "Assign Type Missmatch in Expression:\n";
          printOn(std::cerr);
          std::cerr << lvalue->get_type() << " := " << expr->get_type() << "\n";
          exit(1);
        }
      }
  }
  virtual Value* compile() const override {
    
    std::string s = lvalue->get_char_var();
    auto *right = expr->compile();
    if (s.compare("@") == 0){ // we have a result to return
      std::string fname = loggedinfo.getLastFunc();
      loggedinfo.storeResult(fname, right);
      return nullptr;
    }
    if (lvalue->isArElement()){
      
      auto *index = lvalue->get_offset();
      
      auto *addr = calcAddr(s, "Assign", index);
      return Builder.CreateStore(right,addr);  
    }
    
    auto *addr = calcAddr(s, "Assign", nullptr);
    return Builder.CreateStore(right,addr);
  }
private:
  Expr *expr;
  Expr *lvalue;
};

class Return: public Stmt {
public:
  Return() {};
  virtual void printOn(std::ostream &out) const override {
    out << "Return()";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Return()";
    return s;
  }
  virtual Value* compile() const override { 
  	// create a void return
    llvm::ReturnInst *ret;
    ret = Builder.CreateRetVoid();
    return ret;
  }
private:
};

class Id_list:public AST {
public:
  Id_list(): id_list(){ }
  virtual void printOn(std::ostream &out) const override {
    out << "Idlist(";
    for (std::string id : id_list) {
      out << id;
      out << ", ";
    }
    out << ")";
  }
  void append_char(std::string str) {
    id_list.push_back(str);
  }
  void append_first(std::string i) { id_list.insert(id_list.begin(), i); }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Idlist(";
    for (std::string id : id_list) {
      s += id;
      s += ", ";
    }
    s += ")";
    return s;
  }
  std::vector<std::string> getlist() {
    return id_list;
  }
  virtual Value* compile() const override { return nullptr;}
private:
	std::vector<std::string> id_list;
};

class Formal: public AST {
public:
  Formal(char *i, Id_list *idlist, Type_not_from_llvm *t, std::string var = "ref"){
    if (var.compare("var") == 0)
      by = "PASS_BY_REFERENCE";
    else
      by = "PASS_BY_VALUE";
    id_list = idlist;
    if (i) {id_list->append_first(i);}
    type = t;
  }
  Formal(Id_list *idlist, Type_not_from_llvm *t){
    id_list = idlist;
    type = t;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Formal(";
    id_list->printOn(out);
    out << type->get_type() << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Formal(";
    s += id_list->getstring();
    s += type->get_type();
    s += ")";
    return s;
  }
  Types get_type() {
    return type->get_type();
  }
  std::vector<std::string> getIdList(){
    return id_list->getlist();
  }
  std::string getby(){
    return by;
  }
  virtual void semfor() override {
    for (std::string i : id_list->getlist()) {
      st.insertForwardDecl(i,type->get_type());
    }
  }
  virtual void sem() override {
    for (std::string i : id_list->getlist()){
      if(!st.foundForward(i)){
        st.insert(i,type->get_type());
      }
    }
  }
  virtual Value* compile() const override {
  	return nullptr;
  }
private:
  Id *id;
  Id_list *id_list;
  Type_not_from_llvm *type;
  std::string by;
};

class Formal_list: public AST {
public:
  Formal_list(): formal_list(){ }
  ~Formal_list() {
    for (Formal *f : formal_list) delete f;
  }
  void append(Formal* f) { formal_list.push_back(f); }
  void append_first(Formal *f) { formal_list.insert(formal_list.begin(), f); }
  virtual void printOn(std::ostream &out) const override {
    out << "Formallist(";
    if (!formal_list.empty()){
    	for (Formal *f : formal_list) {
      		f->printOn(out);
      		out << ", ";
    	}
    	out << ")";
	}
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Formallist(";
    if (!formal_list.empty()){
	    for (Formal *f : formal_list) {
	      s += f->getstring();
	      s += ", ";
	    }
	    s += ")";
	}
    return s;
  }
  std::vector<Formal *> getlist() {
    return formal_list;
  }
  virtual void semfor() override{
    for (Formal *f : formal_list) { f->semfor(); }
  }
  virtual void sem() override {
    for (Formal *f : formal_list) { f->sem(); }
  }
  virtual Value* compile() const override { return nullptr;}
private:
  std::vector<Formal *> formal_list;
};

class Call: public Stmt{
public:
  Call(char* i) {
    id = i;
    expr_list = nullptr;
  }
  Call(char* i, Expr *e, Exprlist *el = nullptr){
    id = i;
    expr_list = el;
    if (e) expr_list->append_first(e);
  }
  ~Call(){
    delete id;
    delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Call(";
    if (id) out << id;
    if (expr_list) expr_list->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Call(";
    if (id) s += std::string(id);
    if (expr_list) s += expr_list->getstring();
    s += ")";
    return s;
  }
  virtual void sem() override {
    std::string s = id;
    if(expr_list) expr_list->sem();
    st.lookup(s);
    if (st.foundProc(s)){
      std::vector<Formal *> formal_list;
      int expected = 0;
      int given = 0;
      formal_list = st.getFormalsProcedureAll(s)->getlist();
      if(!formal_list.empty()){ //count how many args we expect the proc to have
        for (Formal *f : formal_list){
          expected += int(f->getIdList().size());
          int formalsize;
          formalsize = f->getIdList().size();
          int j = 0;
          for (int i = 0; i < formalsize; i++){
            if (!(f->get_type() == expr_list->getlist().at(j)->get_type())){
              std::cout << "ERROR: In procedure" << s << " type mismatch regarding arguments ";
              std::cout << f->getIdList().at(j) << " and " << expr_list->getlist().at(i);
              std::cout << ". One is of type ";
              std::cout << f->get_type();
              std::cout << " and the other one is of type ";
              std::cout << expr_list->getlist().at(j)->get_type();
              std::cout << "\n";
              exit(1);
              j++;
            }
          }
        }
      }
      if(expr_list) given = expr_list->getlist().size();
      if (given != expected) {
        std::cout << "ERROR: Procedure " << s << " needs " << expected <<" arguments. However " << given << " are given.\n";
        exit(1);
      }
    }
    else if (st.foundFunc(s)){
      std::vector<Formal *> formal_list;
      int expected = 0;
      int given = 0;
      formal_list = st.getFormalsFuncAll(s)->getlist();
      if(!formal_list.empty()){ //count how many args we expect the proc to have
        for (Formal *f : formal_list){
          expected += int(f->getIdList().size());
          int formalsize;
          formalsize = f->getIdList().size();
          int j = 0;
          for (int i = 0; i < formalsize; i++){
            if (!(f->get_type() == expr_list->getlist().at(j)->get_type())){
              std::cout << "ERROR: In Function" << s << " type mismatch regarding arguments ";
              std::cout << f->getIdList().at(j) << " and " << expr_list->getlist().at(i);
              std::cout << ". One is of type ";
              std::cout << f->get_type();
              std::cout << " and the other one is of type ";
              std::cout << expr_list->getlist().at(j)->get_type();
              std::cout << "\n";
              exit(1);
              j++;
            }
          }
        }
      }
      if(expr_list) given = expr_list->getlist().size();
      if (given != expected) {
        std::cout << "ERROR: Function " << s << " needs " << expected <<" arguments. However " << given << " are given.\n";
        exit(1);
      }
    }
  }
  virtual Value* compile() const override { 
    std::string s = id;
    if (isLibFunc(s)){  //if it's a library proc
    	std::vector<Expr *> list;
    	list = expr_list->getlist();
    	Value *n = list.front()->compile();
    	if ((strcmp(id, "writeInteger")) == 0){
    		if (!n) {std::cerr << "there is no n\n";}
    		Value *n64 = Builder.CreateZExt(n, i64, "ext");
    		Builder.CreateCall(TheWriteInteger, std::vector<Value *> { n64 });
    	}
    	if ((strcmp(id, "writeBoolean")) == 0){
    		//Value *n8 = Builder.CreateZExt(n, i8, "ext");
    		Builder.CreateCall(TheWriteBoolean, std::vector<Value *> { n });
    	}
    	if ((strcmp(id, "writeChar")) == 0){
        Value *n8 = Builder.CreateZExtOrTrunc(n, i8, "extrunc");
    	  //Value *n8 = c8(n);
        Builder.CreateCall(TheWriteChar, std::vector<Value *> { n8 });
      }
      if ((strcmp(id, "writeReal")) == 0){
        //Value *n8 = Builder.CreateTrunc(n, DoubleTyID, "extrunc");
        Builder.CreateCall(TheWriteReal, std::vector<Value *> { n });
      }
      if ((strcmp(id, "writeString")) == 0){
        //Value *n8 = Builder.CreateZExt(n, i8, "ext");
        Builder.CreateCall(TheWriteString, std::vector<Value *> { n });
      }
    }
    else {  //if it's user defined procedure
      
      std::string fname = id;
      
      llvm::Function *F = loggedinfo.getProcInScope(fname);
      std::vector<llvm::Value*> argv;
      std::vector<Expr *> ASTargs = expr_list->getlist();
      auto itr = ASTargs.begin();
      auto *ASTarg = *itr;

      // loop through parameters
      for (auto &Arg: F->args()){

        llvm::Value *arg;
        // function with no parameters, only outer scope ones
        if (ASTargs.empty()) {
          argv.push_back(deref(loggedinfo.getVarAlloca(Arg.getName().str())));
          continue;
        }

        itr++;

        //check if done with real parameters
        if (ASTarg == nullptr) {
          argv.push_back(deref(loggedinfo.getVarAlloca(Arg.getName().str())));
          continue;
        }
        
        // if expected argument is by value
        if (!Arg.getType()->isPointerTy()){
          arg = ASTarg->compile();
        }
        else {
          //variable
          std::string var = ASTarg->get_char_var();
          arg = calcAddr(var, "ID", nullptr);
        }

        argv.push_back(arg);
        if (itr != ASTargs.end()) ASTarg = *itr;

      }
      
      
      return Builder.CreateCall(F,argv);
    }
  	return nullptr;
  }
private:
  char* id;
  Exprlist *expr_list;
};

class ExprCall: public Rvalue{
public:
  ExprCall(char* i) {
    id = i;
    expr_list = nullptr;
  }
  ExprCall(char* i, Expr *e, Exprlist *el = nullptr){
    id = i;
    expr_list = el;
    if (e) expr_list->append_first(e);
  }
  ~ExprCall(){
    delete id;
    delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "ExprCall(";
    if (id) out <<"id: " << id << ",";
    if (expr_list) expr_list->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "ExprCall(";
    if (id) s += std::string(id) + ",";
    if (expr_list) s += expr_list->getstring();
    s += ")";
    return s;
  }
  virtual void sem() override {
    std::string s = id;
    if(expr_list) expr_list->sem();
    st.lookup(s);
    if (st.foundProc(s)){
      std::vector<Formal *> formal_list;
      int expected = 0;
      int given = 0;
      formal_list = st.getFormalsProcedureAll(s)->getlist();
      if(!formal_list.empty()){ //count how many args we expect the proc to have
        for (Formal *f : formal_list){
          expected += int(f->getIdList().size());
          int formalsize;
          formalsize = f->getIdList().size();
          int j = 0;
          for (int i = 0; i < formalsize; i++){
            if (!(f->get_type() == expr_list->getlist().at(j)->get_type())){
              std::cout << "ERROR: In procedure" << s << " type mismatch regarding arguments ";
              std::cout << f->getIdList().at(j) << " and " << expr_list->getlist().at(i);
              std::cout << ". One is of type ";
              std::cout << f->get_type();
              std::cout << " and the other one is of type ";
              std::cout << expr_list->getlist().at(j)->get_type();
              std::cout << "\n";
              exit(1);
              j++;
            }
          }
        }
      }
      if(expr_list) given = expr_list->getlist().size();
      if (given != expected) {
        std::cout << "ERROR: Procedure " << s << " needs " << expected <<" arguments. However " << given << " are given.\n";
        exit(1);
      }
    }
    else if (st.foundFunc(s)){
      std::vector<Formal *> formal_list;
      int expected = 0;
      int given = 0;
      formal_list = st.getFormalsFuncAll(s)->getlist();
      if(!formal_list.empty()){ //count how many args we expect the proc to have
        for (Formal *f : formal_list){
          expected += int(f->getIdList().size());
          int formalsize;
          formalsize = f->getIdList().size();
          int j = 0;
          for (int i = 0; i < formalsize; i++){
            if (!(f->get_type() == expr_list->getlist().at(j)->get_type())){
              std::cout << "ERROR: In Function" << s << " type mismatch regarding arguments ";
              std::cout << f->getIdList().at(j) << " and " << expr_list->getlist().at(i);
              std::cout << ". One is of type ";
              std::cout << f->get_type();
              std::cout << " and the other one is of type ";
              std::cout << expr_list->getlist().at(j)->get_type();
              std::cout << "\n";
              exit(1);
              j++;
            }
          }
        }
      }
      if(expr_list) given = expr_list->getlist().size();
      if (given != expected) {
        std::cout << "ERROR: Function " << s << " needs " << expected <<" arguments. However " << given << " are given.\n";
        exit(1);
      }
    }
  }
  virtual Value* compile() const override { 
    std::string s = id;
  	if (isLibFunc(s)){
      std::vector<Expr *> list;
        if ((strcmp(id, "readInteger")) == 0){
          return Builder.CreateCall(TheReadInteger, std::vector<Value *> { });
        }
        if ((strcmp(id, "readBoolean")) == 0){
          
          return Builder.CreateCall(TheReadBoolean, std::vector<Value *> {  });
          
        }
        if ((strcmp(id, "readChar")) == 0){
          return Builder.CreateCall(TheReadChar, std::vector<Value *> {  });
        }
        if ((strcmp(id, "readString")) == 0){
         
          if (expr_list){
          	list = expr_list->getlist();
          	Value *n = list.front()->compile();
          	Value *arr = (list.at(1))->compile();
          	return Builder.CreateCall(TheReadString, std::vector<Value *> { n, arr });
          }
          else{
          	/*some error message */
          }
          
        }
        if ((strcmp(id, "abs")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            Value *n64 = Builder.CreateZExt(n, i32, "ext");
            return Builder.CreateCall(TheAbs, std::vector<Value *> { n64 });
          }
          else {
            /*some error message*/
          }
        }
        if ((strcmp(id, "fabs")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            //Value *n64 = Builder.CreateZExt(n, i32, "ext");
            return Builder.CreateCall(TheFabs, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
          }
        }
        if ((strcmp(id, "sqrt")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            
            return Builder.CreateCall(TheSqrt, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for sqrt!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "sin")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            
            return Builder.CreateCall(TheSin, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for sin!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "cos")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            
            return Builder.CreateCall(TheCos, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for cos!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "tan")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            
            return Builder.CreateCall(TheTan, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for tan!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "arctan")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            
            return Builder.CreateCall(TheArctan, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for atan!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "exp")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            
            return Builder.CreateCall(TheExp, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for exp!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "ln")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            return Builder.CreateCall(TheLn, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for ln!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "pi")) == 0){
          return Builder.CreateCall(ThePi, std::vector<Value *> { });
        }
        if ((strcmp(id, "trunc")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            return Builder.CreateCall(TheTrunc, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for trunc!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "round")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            return Builder.CreateCall(TheRound, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for round!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "ord")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            return Builder.CreateCall(TheOrd, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for ord!\n";
            exit(1);
          }
        }
        if ((strcmp(id, "chr")) == 0){
          if (expr_list){
            list = expr_list->getlist();
            Value *n = list.front()->compile();
            return Builder.CreateCall(TheChr, std::vector<Value *> { n });
          }
          else {
            /*some error message*/
            std::cerr << "No arguments provided for chr!\n";
            exit(1);
          }
        }
    }
    else{ //user defined function
      
      std::string fname = id;
      
      llvm::Function *F = loggedinfo.getFunctionInScope(fname);
      std::vector<llvm::Value*> argv;
      std::vector<Expr *> ASTargs = expr_list->getlist();
      auto itr = ASTargs.begin();
      auto *ASTarg = *itr;

      // loop through parameters
      for (auto &Arg: F->args()){

        llvm::Value *arg;
        // function with no parameters, only outer scope ones
        if (ASTargs.empty()) {
          argv.push_back(deref(loggedinfo.getVarAlloca(Arg.getName().str())));
          continue;
        }

        itr++;

        // check if done with real parameters
        if (ASTarg == nullptr) {
          argv.push_back(deref(loggedinfo.getVarAlloca(Arg.getName().str())));
          continue;
        }
        
        // if expected argument is by value
        if (!Arg.getType()->isPointerTy()){
          arg = ASTarg->compile();
        }
        else {
          //variable
          std::string var = ASTarg->get_char_var();
          arg = calcAddr(var, "ID", nullptr);
        }

        argv.push_back(arg);
        if (itr != ASTargs.end()) ASTarg = *itr;

      }
      
      
      return Builder.CreateCall(F,argv);
    }
    return nullptr;
  }
private:
  char* id;
  Exprlist *expr_list;
};

class New: public Stmt {
public:
  New(Expr *e, Expr *l){
    brackets = e;
    lvalue = l;
  }
  New(Expr *l) {
    lvalue = l;
    brackets = nullptr;
  }
  New(Expr *e, Expr *l, char* p){
    brackets = e;
    lvalue = l;
    op = p;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "New(";
    if (lvalue) lvalue->printOn(out);
    if (brackets) brackets->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "New(";
    if (lvalue) s += lvalue->getstring();
    if (brackets) s += brackets->getstring();
    s += ")";
    return s;
  }
  virtual void sem() override {
    if (lvalue && brackets){
      // new [ expr ] lvalue
      lvalue->sem();
      brackets->sem();
      if (lvalue->get_type() == TYPE_result){
        lvalue->set_type(st.lookup("result")->type);
      }
      if (brackets->get_type() == TYPE_result){
        brackets->set_type(st.lookup("result")->type);
      }
      if (lvalue->get_type() != TYPE_pointer){
        std::cout << "\nError: In expression: ";
        printOn(std::cout);
        std::cout << "\nleft value of the expression must be of type pointer but it's of type: " << lvalue->get_type() << "\n";
        exit(1);
      }
      else{
        if (lvalue->get_type() == TYPE_pointer){
          if (lvalue->get_pointer()->get_oftype() != TYPE_array){
            std::cout << "\nERORR: In expression: ";
            printOn(std::cout);
            std::cout << "\nleft value of the expression must be of type pointer to array but it's a pointer to ";
            std::cout << lvalue->get_pointer()->get_oftype();
            std::cout << "\n";
            exit(1);
          }
        }
        
      }
      if (brackets->get_type() != TYPE_int){
        std::cout << "\nERROR: In expression: ";
        printOn(std::cout);
        std::cout << "\nthe expression inside the brackets must be of type integer but it's of type: ";
        std::cout << brackets->get_type();
        std::cout << "\n";
        exit(1);
      }
      st.makeNew(lvalue->getstring());
    }
    else {
      //"new" lvalue
      lvalue->sem();
      if (lvalue->get_type() == TYPE_result){
        lvalue->set_type(st.lookup("result")->type);
      }
      if (lvalue->get_type() != TYPE_pointer){
        std::cout << "\nERROR: In expression: ";
        printOn(std::cout);
        std::cout << "\nleft value of the expression must be of type pointer but it's of type: " << lvalue->get_type() << "\n";
        exit(1);
      }
      st.makeNew(lvalue->getstring());
    }
  }
  virtual Value* compile() const override { 
  	if (!(brackets)){  // if allocate pointer
      std::string var = lvalue->get_char_var();
      auto *t = loggedinfo.getPoinType(var);
      auto *ptype = PointerType::get(t,0);
      auto *valloca = Builder.CreateAlloca(ptype, 0, var);
      loggedinfo.changeAlloca(var, ptype, valloca);
    }
    else{ // if allocate array
      std::string var = lvalue->get_char_var();
      auto *t = loggedinfo.getArrType(var);
      auto *atype = ArrayType::get(t,brackets->eval());
      auto *valloca = Builder.CreateAlloca(atype, 0, var);
      loggedinfo.changeAlloca(var, atype,valloca);
    }
    return nullptr;
  }
private:
  Expr *lvalue;
  Expr *brackets;
  char *op;
};

class Goto: public Stmt {
public:
  Goto(char* i){
    id = i;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Goto(";
    out << id;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Goto(";
    s += id;
    s += ")";
    return s;
  }
  virtual void sem() override {
    std::string s_id;
    s_id = id;
    if (!st.isLabel(s_id)){
      std::cout << "\nERORR: In expression: ";
      printOn(std::cout);
      std::cout << "\n" << s_id << "is not a label!\n";
      exit(1);
    }
    else {
     if (!st.LabelHasStmt(s_id)){
       std::cout << "\nERORR: In expression: ";
       printOn(std::cout);
       std::cout << "\nLabel " << s_id << " does not correspond to a statement!\n";
       exit(1);
     }
    }
  }
  virtual Value* compile() const override {
  	std::string label = id;
    auto *label_block = loggedinfo.getLabelBlock(label);
    auto *after_block = loggedinfo.getLabelCont(label);
    Function *function = Builder.GetInsertBlock()->getParent();
    auto *b = BasicBlock::Create(TheContext, "goto", function);
    auto *e = BasicBlock::Create(TheContext, "gotoafter", function);
    Builder.CreateBr(label_block);
    Builder.SetInsertPoint(b);
    Builder.CreateBr(after_block);
    Builder.SetInsertPoint(e);
    return nullptr;
  }
private:
  char* id;
};

class Intconst: public Rvalue {
public:
  Intconst(int i): cons(i){
    type = new Int();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Intconst(";
    out << cons;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s="";
    s += "Intconst(";
    s += cons;
    s += ")";
    return s;
  }
  virtual Types get_type() {
    return type->get_type();
  }
  virtual int eval() const override {return cons; }
  virtual int get() {
    return cons;
  }
  virtual Value* compile() const override { return c32(cons);}
private:
  Type_not_from_llvm *type;
  int cons;
};

class Boolconst: public Rvalue {
public:
  Boolconst(std::string val){
    if(val.compare("true")){
      cons = 1;
    }
    else {
      cons = 0;
    }
    type = new Bool();
  }
  Boolconst(bool b){
    cons = b;
    type = new Bool();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Boolconst(";
    out << cons;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Boolconst(";
    s += cons;
    s += ")";
    return s;
  }
  virtual Types get_type() {
    return type->get_type();
  }
  virtual int eval() const override { return cons; }
  virtual Value* compile() const override { return c1(cons);}
private:
  Type_not_from_llvm *type;
  bool cons;
};

class Realconst: public Rvalue {
public:
  Realconst(double r): cons(r) {
    type = new Real();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Realconst(";
    out << cons;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Realconst(";
    s += cons;
    s += ")";
    return s;
  }
  virtual Types get_type() {
    return type->get_type();
  }
  virtual int eval() const override { return cons; }
  virtual Value* compile() const override { return fp32(cons);}
private:
  Type_not_from_llvm *type;
  double cons;
};

class Charconst: public Rvalue {
public:
  Charconst(char *c): cons(c) {
    type = new Char();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Charconst(";
    out << cons;
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Charconst(";
    s += cons;
    s += ")";
    return s;
  }
  virtual Types get_type() {
    return type->get_type();
  }
  virtual int eval() const override { return 0; }
  virtual Value* compile() const override {
    char ch = cons[1];
    return c8(ch);
  }
private:
  Type_not_from_llvm *type;
  char *cons;
};

class Stringconst: public Lvalue {
public:
  Stringconst(char *c): cons(c) {
    arr = new Array(TYPE_char, strlen(c) - 1);
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Stringconst(" << cons << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Stringconst(";
    s += cons;
    s += ")";
    return s;
  }
  virtual Types get_type() {
    return TYPE_array;
  }
  virtual int eval() const override { return 0; }
  virtual Value* compile() const override { 
  	return Builder.CreateGlobalStringPtr(cons);
  }
private:
  Array *arr;
  char *cons;
};

class Nil: public Rvalue {
public:
  Nil(){
    cons = nullptr;
    type = TYPE_nil;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Nil()";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Nil()";
    return s;
  }
  virtual Types get_type() {
    return type;
  }
  virtual int eval() const override { return 0; } //huh?
  virtual Value* compile() const override { 
  	return ConstantPointerNull::get(PointerType::get(i32,0));
  }
private:
  Types type;
  char *cons;
};

class Dispose: public Stmt {
public:
  Dispose(char* b, Expr *lval){
    lvalue = lval;
    if (*b == '['){ brackets = true; }
  }
  Dispose(Expr *lval){
    lvalue = lval;
    brackets = false;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Dispose(";
    if (lvalue) lvalue->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Dispose(";
    if (lvalue) s += lvalue->getstring();
    s += ")";
    return s;
  }
  virtual void sem() override {
    if (lvalue && !brackets) { //dispose l-value
      lvalue->sem();
      if (lvalue->get_type() == TYPE_result){
        lvalue->set_type(st.lookup("result")->type);
      }
      if (lvalue->get_type() != TYPE_pointer){
        std::cout << "\nERROR: In expression: ";
        printOn(std::cout);
        std::cout << "\nleft value of the expression must be of type pointer but it's of type: " << lvalue->get_type() << "\n";
        exit(1);
      }
      if (!st.isNew(lvalue->getstring())){
        std::cout << "\nERROR: In expression: ";
        printOn(std::cout);
        std::cout << "\nleft value of the expression must be created from a new l-value\n";
        exit(1);
      }
      lvalue = nullptr;
    }
    else { //dispose [] l-value
      lvalue->sem();
      if(lvalue->get_type() == TYPE_result){
        lvalue->set_type(st.lookup("result")->type);
      }
      if (lvalue->get_type() != TYPE_pointer) {
        std::cout << "\nERROR: In expression: ";
        printOn(std::cout);
        std::cout << "\nleft value of the expression must be of type pointer but it's of type: " << lvalue->get_type() << "\n";
        exit(1);
      }
      if (!st.isNew(lvalue->getstring())){
        std::cout << "\nERROR: In expression: ";
        printOn(std::cout);
        std::cout << "\nleft value of the expression must be created from a new l-value\n";
        exit(1);
      }
      if (lvalue->get_type() == TYPE_pointer){
        if (lvalue->get_pointer()->get_oftype() != TYPE_array){
          std::cout << "\nERROR: In expression: ";
          printOn(std::cout);
          std::cout << "\nleft value of the expression must be of type pointer to array but it's pointer to ";
          std::cout << lvalue->get_pointer()->get_oftype();
          std::cout << "\n";
          exit(1);
        }
      }
      lvalue = nullptr;
    }
  }
  virtual Value* compile() const override { 
    std::string var = lvalue->get_char_var();
    loggedinfo.disposePointer(var);
    return nullptr;
  }
private:
  Expr *lvalue;
  bool brackets;
};

class If: public Stmt {
public:
  If(Expr *c, Stmt *s1, Stmt *s2 = nullptr):
    cond(c), stmt1(s1), stmt2(s2) {}
  ~If() { delete cond; delete stmt1; delete stmt2; }
  virtual void printOn(std::ostream &out) const override {
    out << "If(" << *cond << ", " << *stmt1;
    if (stmt2 != nullptr) out << ", " << *stmt2;
    out << ")";
  }
  virtual std::string getstring() override{
    std::string s = "";
    s += "If(";
    s += cond->getstring();
    s += ",";
    s += stmt1->getstring();
    s += ",";
    s += stmt2->getstring();
    s += ")";
    return s;
  }
  virtual void sem() override {
    cond->sem();
    if(cond->get_type() == TYPE_result){
      cond->set_type(st.lookup("result")->type);
    }
    if (cond->type_check(TYPE_bool)){
      stmt1->sem();
      if (stmt2 != nullptr) stmt2->sem();
    }
    else {
      std::cout << "\nERORR: In expression: ";
      printOn(std::cout);
      std::cout << "\nCondition is not of type bool!\n";
      exit(1);
    }
  }
  virtual Value* compile() const override {
    Value *condv = cond->compile ();

    if (!condv)
    	return nullptr;

    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *ThenBB =
      BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB =
      BasicBlock::Create(TheContext, "else", TheFunction);
    BasicBlock *AfterBB =
      BasicBlock::Create(TheContext, "endif", TheFunction);
    Builder.CreateCondBr(condv, ThenBB, ElseBB);
    Builder.SetInsertPoint(ThenBB);
    stmt1->compile();
    Builder.CreateBr(AfterBB);
    Builder.SetInsertPoint(ElseBB);
    if (stmt2 != nullptr)
      stmt2->compile();
    Builder.CreateBr(AfterBB);
    Builder.SetInsertPoint(AfterBB);
    return nullptr;
  }
private:
  Expr *cond;
  Stmt *stmt1;
  Stmt *stmt2;
};

class While: public Stmt {
public:
  While(Expr *e, Stmt *st){
    stmt = st;
    cond = e;
  }
  ~While() {
    delete cond;
    delete stmt;
  }
  virtual void printOn(std::ostream &out) const override {
      out << "While(" << *cond << ", " << *stmt;
      out << ")";
    }
  virtual std::string getstring() override{
      std::string s = "";
      s += "While(";
      s += cond->getstring();
      s += ",";
      s += stmt->getstring();
      return s;
  }
  virtual void sem() override {
      cond->sem();
      if (cond->get_type() == TYPE_result){
        cond->set_type(st.lookup("result")->type);
      }
      if (cond->type_check(TYPE_bool)){
        stmt->sem();
      }
      else {
        std::cout << "\nERORR: In expression: ";
        printOn(std::cout);
        std::cout << "\nCondition is not of type bool!\n";
        exit(1);
      }
  }
  virtual Value* compile() const override {
      BasicBlock *PrevBB = Builder.GetInsertBlock();
      Function *TheFunction = PrevBB->getParent();
      BasicBlock *LoopBB =
        BasicBlock::Create(TheContext, "loop", TheFunction);
      BasicBlock *BodyBB =
        BasicBlock::Create(TheContext, "body", TheFunction);
      BasicBlock *AfterBB =
        BasicBlock::Create(TheContext, "endwhile", TheFunction);
      Builder.CreateBr(LoopBB);
      Builder.SetInsertPoint(LoopBB);
      Builder.CreateCondBr(cond->compile(), BodyBB, AfterBB);
      Builder.SetInsertPoint(BodyBB);

      stmt->compile();
      Builder.CreateBr(LoopBB);
      Builder.SetInsertPoint(AfterBB);

      return nullptr;
  }
private:
  Expr *cond;
  Stmt *stmt;
};

class Block: public Stmt {
public:
  Block(Stmt *st, Stmtlist *s_list){
    stmtlist = s_list;
    if (st) {
    	stmt = st;
    	stmtlist->append_first(stmt);
    }
  }
  ~Block() {
    delete stmt;
    delete stmtlist;
  }
  virtual void printOn(std::ostream &out) const override {
      out << "Block(";
      if(stmtlist) stmtlist->printOn(out);
      out << ")";
    }
  virtual std::string getstring() override{
      std::string s = "";
      s += "Block(";
      if(stmtlist) s += stmtlist->getstring();
      return s;
  }
  virtual void sem() override {
    std::string parent = st.getParent();
    if(st.getFormalsFuncAll(parent)){
      st.getFormalsFuncAll(parent)->sem();
    }
    else if(st.getFormalsProcedureAll(parent)){
      st.getFormalsProcedureAll(parent)->sem();
    }
    stmtlist->sem();
  }
  virtual Value* compile() const override {
      stmtlist->compile();
      return nullptr;
   }
private:
  Stmt *stmt;
  Stmtlist *stmtlist;
  int size;
};

class Label: public AST {
public:
  Label(std::string i, Id_list *idlist){
    id_list = idlist;
    if (!i.empty()) idlist->append_first(i);
  }
  ~Label(){
    delete id_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Label(";
    if (id_list) id_list->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Label(";
    if (id_list) s += id_list->getstring();
    s += ")";
    return s;
  }
  virtual void sem() override {
    std::vector<std::string> v;
    v = id_list->getlist();
    for (std::string s : v){
      st.insertLabel(s, TYPE_label);
    }
  }
  virtual Value* compile() const override { 
  	return nullptr;
  }
private:
  Id *id;
  Id_list *id_list;
};

class LabelStmt: public Stmt{
public:
  LabelStmt(char* i, Stmt *s){
    id = i;
    stmt = s;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "LabelStmt(";
    if(id) out << id << " ";
    if(stmt) stmt->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s ="";
    s += "LabelStmt(";
    std::string var;
    var = id;
    if(id) s+= var + " ";
    if(stmt) s+= stmt->getstring();
    s+= ")";
    return s;
  }
  virtual void sem() override{
    stmt->sem();
  }
  virtual Value* compile() const override { 
    std::string name = id;
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *BB =
      BasicBlock::Create(TheContext, name, TheFunction);
    BasicBlock *AfterBB =
      BasicBlock::Create(TheContext, "after", TheFunction);
    Builder.CreateBr(BB);
    Builder.SetInsertPoint(BB);
    stmt->compile();
    Builder.CreateBr(AfterBB);
    Builder.SetInsertPoint(AfterBB);
    loggedinfo.addLabel(name,BB,AfterBB);
    return nullptr;
  	
  }
private:
  char* id;
  Stmt *stmt;
};

class Decl: public AST {
public:
  Decl(std::string i, Id_list *idlist, Type_not_from_llvm *t){
    id_list = idlist;
    if (!i.empty()) id_list->append_first(i); 
    type = t;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Decl(";
    if (id_list) id_list->printOn(out);
    out << type->get_type() << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Decl(";
    if (id_list) s += id_list->getstring();
    s += type->get_type();
    s += ")";
    return s;
  }
  virtual void sem() {
    for (std::string s : id_list->getlist()) {
      st.insert(s, type->get_type());
    }
  }
  virtual Value* compile() const override {   

    if (type->get_type() == TYPE_array){
      
      if (type->get_size() == -1){  //array is not allocated yet. will be with a "new" statement
        for (std::string var : id_list->getlist()){
          auto *t = type_to_llvm(type->get_oftype());
          loggedinfo.addVariable(var, nullptr, nullptr);
          loggedinfo.arrayType(var, t);
        }
        return nullptr;
      }
      else{ // array size was declared, so allocate the array
        
        for (std::string var : id_list->getlist()){
          auto *t = type_to_llvm(type->get_oftype());
          
          int num = type->get_size();
          
          auto *vtype = llvm::ArrayType::get(t, num);
          
          auto *valloca = Builder.CreateAlloca(vtype, nullptr, var); 
          
          loggedinfo.addVariable(var, vtype, valloca);
          
          loggedinfo.arrayType(var, t);
          
          }
          return nullptr;
      }
      return nullptr;
    }
    if (type->get_type() == TYPE_pointer) { // pointer will be allocated with a "new" statement

      for (std::string var : id_list->getlist()){
        auto *ptype = type_to_llvm(type->get_oftype());
        auto *vtype = ptype->getPointerTo();
        loggedinfo.addVariable(var, vtype, nullptr);
        loggedinfo.pointerType(var, ptype);
      }
      
      return nullptr;
    }
    for (std::string var : id_list->getlist()){  // this is for everything not array, pointer
      
      auto *vtype = type_to_llvm(type->get_type());
      auto *valloca = Builder.CreateAlloca(vtype, nullptr, var); 
      //log variable
      loggedinfo.addVariable(var, vtype, valloca);
    }
    return nullptr;
  }
private:
  Id *id;
  Id_list *id_list;
  Type_not_from_llvm *type;
};


class Decl_list: public AST {
public:
  Decl_list(){
    
  }
  ~Decl_list(){
    for (Decl *d : decl_list) { delete d; }
  }
  void append (Decl *d) {
    decl_list.push_back(d);
  }
  void append_first(Decl *d) { decl_list.insert(decl_list.begin(), d); }
  virtual void printOn(std::ostream &out) const override {
    out << "Decl_list(";
    for (Decl *d : decl_list) {
      d->printOn(out);
      out << ", ";
    }
    out << "\n)";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Decl_list(";
    for (Decl *d : decl_list){
      s += d->getstring();
      s += ",";
    }
    s += ")";
    return s;
  }
  virtual void sem () override {
    for (Decl *d : decl_list){
      d->sem();
    }
  }
  virtual Value* compile() const override {
    
    for (Decl *d : decl_list){
      
      d->compile();
      
    }
    return nullptr;
  }
private:
  std::vector<Decl *> decl_list;
};

class Header: public AST{
public:
  virtual void printOn(std::ostream &out) const = 0;
  virtual std::string getstring() { return "Header()";}
};

class Procedure: public Header {
public:
  Procedure(char const *i, Formal *f, Formal_list *fl){
    id = i;
    formal_list = fl;
    if (f) formal_list->append_first(f);
    forward = false;
  }
  Procedure(char* i){
    id = i;
    formal_list = nullptr;
    forward = false;
  }
  ~Procedure(){
    delete id;
    delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Procedure(" << id << ",";
    if (formal_list) formal_list->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Procedure(";
    s += std::string(id);
    s += ",";
    if (formal_list) s += formal_list->getstring();
    s += ")";
    return s;
  }
  virtual void semfor() override {
    forward = true;
    std::string s = id;
    st.insertProcedure(s, TYPE_proc, formal_list, forward);
  }
  virtual void sem() override {
    std::string s = id;
    if (st.foundForward(s)) {
      std::string forward_decl;
      std::string current_decl;
      forward_decl = st.getFormalsProcedureAll(s)->getstring();
      current_decl = formal_list->getstring();
      if (forward_decl.compare(current_decl)){
        std::cout << "ERROR in procedure " << s << ". It was forward declared with arguments " << forward_decl << " but now it has arguments " << current_decl << "\n";
        exit(1);
      }
      st.clearForDecl(s);
      st.insertParent(s);
    }
    else {
      st.insertProcedure(s, TYPE_proc, formal_list, forward);
    }
  }
  virtual Value* compile() const override { 
    std::string pname = id;
    loggedinfo.pushProc(pname);
    loggedinfo.addProcFormals(pname,formal_list);
    return nullptr;
  }
private:
  char const *id;
  Formal_list *formal_list;
  bool forward;
};

class Function_not_from_llvm: public Header {
public:
  Function_not_from_llvm(char const *i, Formal *f, Formal_list *fl, Type_not_from_llvm *t){
    id = i;
    formal_list = fl;
    if (f) fl->append_first(f);
    type = t;
    forward = false;
  }
  Function_not_from_llvm(char const *i, Type_not_from_llvm *t){
    id = i;
    formal_list = nullptr;
    type = t;
    forward = false;
  }
  ~Function_not_from_llvm(){
    delete id;
    delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Function(" << id << "," << type->get_type() << ",";
    if (formal_list) formal_list->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Function(";
    s += id;
    s += ",";
    s += type->get_type();
    s += ",";
    if (formal_list) s += formal_list->getstring();
    s += ")";
    return s;
  }
  virtual void semfor() override {
    forward = true;
    std::string s = id;
    st.insertFunction(s, type->get_type(), formal_list, forward);
  }
  virtual void sem() override {
    std::string s = id;
    if (type->get_type() == TYPE_array){
      std::cout << "ERROR: Function " << s << " can not be of type ARRAY!";
      exit(1);
    }
    if (st.foundForward(s)){
      std::string forward_decl;
      std::string current_decl;
      forward_decl = st.getFormalsFuncAll(s)->getstring();
      current_decl = formal_list->getstring();
      if (forward_decl.compare(current_decl)){
        std::cout << "ERROR in function " << s << ". It was forward declared with arguments " << forward_decl << " but now it has arguments " << current_decl << "\n";
        exit(1);
      }
      st.clearForDecl(s);
      st.insertParent(s);
      formal_list->sem();
    }
    else {
      st.insertFunction(s, type->get_type(), formal_list, forward);
      formal_list->sem();
    }
  }
  virtual Value* compile() const override {

  	std::string fname = id;
    loggedinfo.pushFunc(fname);
    loggedinfo.addFuncFormals(fname,formal_list);
    llvm::Type *retType;
    if (type->get_type() == TYPE_array){
      retType = type_to_llvm(TYPE_int);
    }
    else if (type->get_type() == TYPE_pointer){
      retType = type_to_llvm(TYPE_int);
    }
    else{
      retType = type_to_llvm(type->get_type());  
    }
    loggedinfo.addFuncType(fname,retType);
    return nullptr;
  }
private:
  char const *id;
  Type_not_from_llvm *type;
  Formal_list *formal_list;
  bool forward;
};

class Body;

class Local: public AST {
public:
  Local(Decl_list *d_list){ //"var"
    decl_list = d_list;
    type = "var";
  }
  Local(Label *l){ //"label"
    //id = new Id(i);
    label = l;
    type = "label";
  }
  Local(Header *h, AST *b){ //header ; body (func/proc)
    header = h;
    body = b;
    type = "func_proc";
  }
  Local(Header *h){ //"forward" header ;
    header = h;
    type = "forward";
  }
  ~Local() {};
  virtual void printOn(std::ostream &out) const override {
    out << "Local(";
    if (type.compare("var") == 0){
      if (decl_list) decl_list->printOn(out);
    }
    else if (type.compare("label") == 0){
      if (label) label->printOn(out);
    }
    else if (type.compare("func_proc") == 0){
      if (header) header->printOn(out);
      if (body) body->printOn(out);
    }
    else if (type.compare("forward") == 0){
      if (header) header->printOn(out);
    }
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Local(";
    if (type.compare("var") == 0){
      s += decl_list->getstring();
    }
    else if (type.compare("label") == 0){
      s += label->getstring();
    }
    else if (type.compare("func_proc") == 0){
      s += header->getstring();
      s += body->getstring();
    }
    else if (type.compare("forward") == 0){
      s += header->getstring();
    }
    s += ")";
    return s;
  }
  bool isProcFunc() {
  	if (type.compare("func_proc") == 0)
  		return true;
  	else
  		return false;
  }
  virtual void sem() override {
    if (type.compare("var") == 0){
      decl_list->sem();
    }
    else if (type.compare("label") == 0){
      label->sem();
    }
    else if (type.compare("func_proc") == 0){
      header->sem();
      body->sem();
    }
    else if (type.compare("forward") == 0){
      header->semfor();
    }
  }
  virtual Value* compile() const override {
    if(type.compare("var") == 0){
      decl_list->compile();
    }
    else if (type.compare("label") == 0){
      label->compile();
    }
    else if (type.compare("func_proc") == 0){
      header->compile();
      body->compile();
    }
    else if(type.compare("forward") == 0){
      header->compile();
    }
    return nullptr;
  }
private:
  Id *id;
  Decl_list *decl_list;
  Label *label;
  Header *header;
  AST *body;
  std::string type;
};

class Local_list: public AST {
public:
  Local_list(){}
  ~Local_list() {
    for (Local *l : local_list) {
      delete l;
    }
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Local_list(";
    for (Local *l : local_list) {
      	l->printOn(out);
      	//out << "local";
      	out << "\n\t";
    }
    out << "\n)";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Local_list(";
    for (Local *l : local_list){
      l->getstring();
      s += ",";
    }
    s += ")";
    return s;
  }

  void append(Local *l) {
    local_list.push_back(l);
  }
  void append_first(Local *l) { local_list.insert(local_list.begin(), l); }
  virtual void sem() override {
    for (Local *l : local_list) {
      l->sem();
    }
  }
  std::vector<Local *> getlist() {
    return local_list;
  }

  std::vector<Local *> getProcFuncDecl() {
  	std::vector<Local *> list;
  	for (Local *l : local_list){
  		if (l->isProcFunc()){
  			list.push_back(l);
  		}
  	}
  	return list;
  }
  virtual Value* compile() const override {
  	
    for (Local *l : local_list) {
      if (l->isProcFunc())
      	continue;
      l->compile();
    }
    return nullptr;
  }
private:
  std::vector<Local *> local_list;
};

class Body: public AST {
public:
  Body(Local_list *llist, Block *b){
    local_list = llist;
    block = b;
    size = 0;
  }
  ~Body(){ delete local_list; };
  virtual void printOn(std::ostream &out) const override {
    out << "Body(";
    if (local_list) local_list->printOn(out);
    out << ", ";
    if (block) block->printOn(out);
    out << ")";
  }
  virtual std::string getstring() override {
    std::string s = "";
    s += "Body";
    if (local_list) s += local_list->getstring();
    s += ", ";
    if (block) s += block->getstring();
    return s;
  }
  virtual void sem() override {
    st.openScope();
    local_list->sem();
    block->sem();
    size = st.getSizeOfCurrentScope();
    st.closeScope();
  }
  virtual Value* compile() const override {
    

    if ((loggedinfo.isFuncStackEmpty()) and (loggedinfo.isProcStackEmpty())){ // this is our main's body
      llvm::Function * main = loggedinfo.getFunctionInScope("main");
  	  std::vector<Local *> list;
  	  list = local_list->getProcFuncDecl();
  	  for (Local *l : list){
  	  	l->compile();
  	  }
  	  
      BasicBlock *BB = BasicBlock::Create(TheContext, "entry", main);
      Builder.SetInsertPoint(BB);
      
      local_list->compile();

      
      
      block->compile();

      Builder.CreateRet(c32(0));

      if (llvm::verifyFunction(*main, &llvm::errs())) {std::cout << "Somethings wrong!\n";}
      loggedinfo.closeScope();
      return nullptr;
    }
    else if (!(loggedinfo.isProcStackEmpty())){ //this is the body of a procedure

      std::string pname = loggedinfo.getLastProc();
      Formal_list * fl = loggedinfo.getProcFormals(pname);
      std::vector<Formal *> params = fl->getlist();
      std::vector<std::string> parameterNames;
      std::vector<llvm::Type *> parameterTypes;
      std::vector<std::string> outerScopeVarsNames;
      unordered_map<std::string, llvm::Type *> outerScopeVarsTypes;
      unordered_map<std::string, llvm::AllocaInst*> outerScopeVarsAllocas;
      llvm::Type *retType = Type::getVoidTy(TheContext);

      // first log param types and names
      for (Formal *f : params){

        std::vector<std::string> id_list;
        id_list = f->getIdList();
        for (std::string name : id_list){
          // std::string name = id;
          parameterNames.push_back(name);
          parameterTypes.push_back(type_to_llvm(f->get_type(),f->getby()));
        }
      }

      // then add references to outer scope variables as parameters
      outerScopeVarsTypes = loggedinfo.getCurrentScopeVarTypes();
      outerScopeVarsAllocas = loggedinfo.getCurrentScopeVarAllocas();
      for (auto var: outerScopeVarsTypes) outerScopeVarsNames.push_back(var.first);

      llvm::Type *varType;
      for (std::string var : outerScopeVarsNames) {
        // skip shadowed outer scope variables
        if (find(parameterNames.begin(), parameterNames.end(), var) != parameterNames.end()) continue;
        varType = outerScopeVarsTypes[var];
        parameterNames.push_back(var);
        // if var is pointer, leave it as it is
        if (varType->isPointerTy())
          parameterTypes.push_back(varType);
        //else, we need to pass a reference to it as a parameter
        else
          parameterTypes.push_back(varType->getPointerTo());
      }

      llvm::FunctionType *PT = llvm::FunctionType::get(retType, parameterTypes, false);
      llvm::Function *P = llvm::Function::Create(PT, llvm::Function::ExternalLinkage, pname, TheModule.get());

      loggedinfo.addProcInScope(pname, P);

      loggedinfo.openScope();
      // now, let's set all param names
      unsigned Idx = 0;
      for (auto &arg : P->args()) arg.setName(parameterNames[Idx++]);

      llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "", P);
      Builder.SetInsertPoint(BB);

      // create allocas for params
      for (auto &arg : P->args()) {
        auto *alloca = Builder.CreateAlloca(arg.getType(), nullptr, arg.getName().str());
        Builder.CreateStore(&arg, alloca);
        loggedinfo.addVariable(arg.getName().str(), arg.getType(), alloca);
      }

      // compile local_list
      std::vector<Local *> local_definitions;
      for (Local *l : local_definitions){
        l->compile();
        Builder.SetInsertPoint(BB);
      }

      // compile block of statements
      block->compile();

      // check if the instruction "return" terminates our block
      if (!BB->getTerminator()){
      	std::cout << "Procedure " << pname << " lacks a terminator. Maybe you forgot 'return' ? \n";
      	exit(1);
      }

      if (llvm::verifyFunction(*P, &llvm::errs())) {
        std::cout << "Somethings wrong! LLVM couldn't verify Procedure " << pname <<"\n";
        exit(1);
      }
      loggedinfo.closeScope();
      loggedinfo.removeCurrProc();
      return nullptr;


    }
    else { //this is the body of a function

	  std::string fname = loggedinfo.getLastFunc();
      Formal_list * fl = loggedinfo.getFuncFormals(fname);
      llvm::Type *retType = loggedinfo.getFuncType(fname);
      std::vector<Formal *> params = fl->getlist();
      std::vector<std::string> parameterNames;
      std::vector<llvm::Type *> parameterTypes;
      std::vector<std::string> outerScopeVarsNames;
      unordered_map<std::string, llvm::Type *> outerScopeVarsTypes;
      unordered_map<std::string, llvm::AllocaInst*> outerScopeVarsAllocas;
      

      // first log param types and names
      for (Formal *f : params){
        std::vector<std::string> id_list;
        id_list = f->getIdList();
        for (std::string name : id_list){
          // std::string name = id;
          parameterNames.push_back(name);
          parameterTypes.push_back(type_to_llvm(f->get_type(),f->getby()));
        }
      }

      // then add references to outer scope variables as parameters
      outerScopeVarsTypes = loggedinfo.getCurrentScopeVarTypes();
      outerScopeVarsAllocas = loggedinfo.getCurrentScopeVarAllocas();
      for (auto var: outerScopeVarsTypes) outerScopeVarsNames.push_back(var.first);

      llvm::Type *varType;
      for (std::string var : outerScopeVarsNames) {
        // skip shadowed outer scope variables
        if (find(parameterNames.begin(), parameterNames.end(), var) != parameterNames.end()) continue;
        varType = outerScopeVarsTypes[var];
        parameterNames.push_back(var);
        // if var is pointer, leave it as it is
        if (varType->isPointerTy())
          parameterTypes.push_back(varType);
        //else, we need to pass a reference to it as a parameter
        else
          parameterTypes.push_back(varType->getPointerTo());
      }

      llvm::FunctionType *FT = llvm::FunctionType::get(retType, parameterTypes, false);
      llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, fname, TheModule.get());

      loggedinfo.addFunctionInScope(fname, F);

      loggedinfo.openScope();

      //this is necessary for result assignment
      loggedinfo.pushFunc(fname);
      // now, let's set all param names
      unsigned Idx = 0;
      for (auto &arg : F->args()) arg.setName(parameterNames[Idx++]);

      llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "", F);
      Builder.SetInsertPoint(BB);

      // create allocas for params
      for (auto &arg : F->args()) {
        auto *alloca = Builder.CreateAlloca(arg.getType(), nullptr, arg.getName().str());
        Builder.CreateStore(&arg, alloca);
        loggedinfo.addVariable(arg.getName().str(), arg.getType(), alloca);
      }

      // compile local_list
      std::vector<Local *> local_definitions;
      for (Local *l : local_definitions){
        l->compile();
        Builder.SetInsertPoint(BB);
      }

      // compile block of statements
      block->compile();

      // create result return
      llvm::Value * result = loggedinfo.getFuncResult(fname);
      Builder.CreateRet(result);

      if (llvm::verifyFunction(*F, &llvm::errs())) {
        std::cout << "Somethings wrong!LLVM couldn't verify Function " << fname << "\n";
        exit(1);
      }
      loggedinfo.removeCurrFunc();
      loggedinfo.closeScope();
      loggedinfo.removeCurrFunc();
      return nullptr;
    }
  }
private:
  Block *block;
  Local_list *local_list;
  int size;
};
