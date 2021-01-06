#pragma once
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include "ast.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

using namespace llvm;

enum Types { TYPE_int, TYPE_bool, TYPE_char, TYPE_real, TYPE_string, TYPE_array, TYPE_pointer, TYPE_proc, TYPE_result, TYPE_nil, TYPE_label };

struct SymbolEntry {
  Types type;
  int offset;
  AllocaInst* val;
  Value* v;
  Function* f;
  
  SymbolEntry() {}
  SymbolEntry(Types t, int ofs) {type = t, offset = ofs;}
  SymbolEntry(Types t, int ofs, llvm::AllocaInst *v) {type = t, offset = ofs, val = v;}
  SymbolEntry(Types t, int ofs, Value *val) {type = t, offset = ofs, v = val; }
  SymbolEntry(int ofs, llvm::Function *v) {offset = ofs, f = v;}
};

class Formal_list;
class Stmt;

class Scope {
public:
  Scope() : locals(), offset(-1), size(0) {}
  Scope(int ofs) : locals(), offset(ofs), size(0) {}
  int getOffset() const { return offset; }
  int getSize() const { return size; }
  SymbolEntry *lookup(std::string c) {
    if (locals.find(c) == locals.end()) return nullptr;
    return &(locals[c]);
  }
  void insert(std::string c, Types t) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++);
    ++size;
    procs[c] = false;
    procformals[c] = nullptr;
    funcs[c] = false;
    funcformals[c] = nullptr;
    labels[c] = false;
  }
  void insert(std::string c, Types t, AllocaInst *v) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, v);
    ++size;
    procs[c] = false;
    procformals[c] = nullptr;
    funcs[c] = false;
    funcformals[c] = nullptr;
    labels[c] = false;
  }
  void insert(std::string c, Function *v) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(offset++, v);
    ++size;
    procs[c] = false;
    procformals[c] = nullptr;
    funcs[c] = true;
    funcformals[c] = nullptr;
    labels[c] = false;
  }
  void insert(std::string c, Types t, Value* v) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, v);
    ++size;
    procs[c] = false;
    procformals[c] = nullptr;
    funcs[c] = false;
    funcformals[c] = nullptr;
    labels[c] = false;
  }
  void insertForwardDecl(std::string c, Types t){
  	insert(c, t);
  	Center_Forwards[c] = true;
  }
  void insertLabel(std::string c, Types t){
  	if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++);
    ++size;
    procs[c] = false;
    procformals[c] = nullptr;
    funcs[c] = false;
    funcformals[c] = nullptr;
    labels[c] = true;
  }
  void insertProcedure(std::string c, Types t, Formal_list *f, bool forward){
  	if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++);
    ++size;
    procs[c] = true;
    procformals[c] = f;
    funcs[c] = false;
    funcformals[c] = nullptr;
    labels[c] = false;
    Center_Forwards[c] = forward;
    queue.push_back(c);
  }
  void insertParent(std::string c){
  	queue.push_back(c);
  }
  void insertFunction(std::string c, Types t, Formal_list *f, bool forward){
  	if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++);
    ++size;
    procs[c] = false;
    procformals[c] = nullptr;
    funcs[c] = true;
    funcformals[c] = f;
    labels[c] = false;
    Center_Forwards[c] = forward;
    queue.push_back(c);
  }
  std::string getParentFunction(){
    if(queue.size()>0){
      return queue.back();
    }
    else{
      std::cerr << "Cant find parent function!";
      exit(1);
    }
  }
  bool found(std::string c){
    if (locals.find(c) == locals.end()) return false;
    return true;
  }
  bool foundForward(std::string c){
  	if (Center_Forwards.find(c) == Center_Forwards.end()){
  		return false;
  	}
  	return true;
  }
  bool foundProc(std::string c){
  	return procs[c];
  }
  bool foundFunc(std::string c){
  	return funcs[c];
  }
  void makeNew(std::string c){
  	news[c] = true;
  }
  bool isLabel(std::string c){
  	return labels[c];
  }
  
  bool isNew(std::string c){
  	return news[c];
  }
  void clearForDecl(std::string c){
  	std::map<std::string, bool>::iterator it;
  	it = Center_Forwards.find(c);
  	Center_Forwards.erase(it);
  }
  Formal_list *getFormalsFunc(std::string c){
  	return funcformals[c];
  }
  Formal_list *getFormalsProcedure(std::string c){
  	return procformals[c];
  }
  void insertLabelStmt(std::string c, Stmt *s){
    labelStmt[c] = s;
  }
  bool LabelHasStmt(std::string c){
    if (labelStmt.find(c) == labelStmt.end()) return false;
    return true;
  }
private:
  std::map<std::string, SymbolEntry> locals;
  std::map<std::string, bool> procs;
  std::map<std::string, Formal_list *> procformals;
  std::map<std::string, bool> funcs;
  std::map<std::string, Formal_list *> funcformals;
  std::map<std::string, bool> labels;
  std::map<std::string, Stmt *> labelsStmts;
  std::map<std::string, bool> Center_Forwards;
  std::vector<std::string> queue;
  std::map<std::string, bool> news;
  std::map<std::string , Stmt *> labelStmt;
  int offset;
  int size;
};

class SymbolTable {
public:
  void openScope() {
    int ofs = scopes.empty() ? 0 : scopes.back().getOffset();
    scopes.push_back(Scope(ofs));
  }
  void closeScope() { scopes.pop_back(); };
  SymbolEntry *lookup(std::string c) {
    SymbolEntry *e;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      e = i->lookup(c);
      if (e != nullptr) {return e;}
    }
    std::cerr << "Unknown variable " << c << std::endl;
    exit(1);
  }
  int getSizeOfCurrentScope() const { return scopes.back().getSize(); }
  void insert(std::string c, Types t) { scopes.back().insert(c, t); }
  void insert(std::string c, Types t, AllocaInst *a) { scopes.back().insert(c, t, a); }
  void insert(std::string c, Types t, Value *v) { scopes.back().insert(c, t, v); }
  void insert(std::string c, Function *f) { scopes.back().insert(c, f); }
  bool foundResult(){
  	SymbolEntry *e;
  	e = scopes.back().lookup("result");
  	if (e != nullptr){
  		return true;
  	}
  	return false;
  }
  Formal_list *getFormalsProcedureAll(std::string c){
  	for (auto i = scopes.rbegin(); i != scopes.rend(); ++i){
  		if(i->getFormalsProcedure(c)){
  			return i->getFormalsProcedure(c);
  		}
  	}
  	return nullptr;
  }
  Formal_list *getFormalsFuncAll(std::string c){
  	for (auto i = scopes.rbegin(); i != scopes.rend(); ++i){
  		if(i->getFormalsFunc(c)){
  			return i->getFormalsFunc(c);
  		}
  	}
  	return nullptr;
  }
  void insertLabel(std::string c, Types t){
  	scopes.back().insertLabel(c,t);
  }
  void insertLabelStmt(std::string c, Stmt *s){
    scopes.back().insertLabelStmt(c, s);
  }
  void insertProcedure(std::string c, Types t, Formal_list *f, bool forward){
  	scopes.back().insertProcedure(c, t, f, forward);
  }
  void insertFunction(std::string c, Types t, Formal_list *f, bool forward){
  	scopes.back().insertFunction(c, t, f, forward);
  }
  void insertForwardDecl(std::string c, Types t){
  	scopes.back().insertForwardDecl(c, t);
  }
  void makeNew(std::string c){
  	scopes.back().makeNew(c);
  }
  void clearForDecl(std::string c){
  	scopes.back().clearForDecl(c);
  }
  void insertParent(std::string c){
  	for (auto i = scopes.rbegin(); i != scopes.rend(); ++i){
  		if(i->found(c)){
  			i->insertParent(c);
  			return;
  		}
  	}
  	std::cerr << c << "is not in a known scope!\n";
  	exit(1);
  	return;
  }
  std::string getParent(){
    std::string s;
    if(scopes.size() == 1){
      s = scopes.back().getParentFunction();
      return s;
    }
    if(scopes.size() >= 2){
      s = scopes[scopes.size() - 2].getParentFunction();
      return s;
    }
    else{
      std::cerr << "Cant find parent function\n";
      exit(1);
    }
  }
  bool foundProc(std::string c){
  	for (auto i = scopes.rbegin(); i != scopes.rend(); ++i){
  		if (i->found(c)){
  			return i->foundProc(c);
  		}
  	}
  	return false;
  }
  bool foundFunc(std::string c){
  	for (auto i = scopes.rbegin(); i != scopes.rend(); ++i){
  		if (i->found(c)){
  			return i->foundFunc(c);
  		}
  	}
  	return false;
  }
  bool foundForward(std::string c){
  	return scopes.back().foundForward(c);
  }
  bool isLabel(std::string c){
  	return scopes.back().isLabel(c);
  }
  bool isNew(std::string c){
    return scopes.back().isNew(c);
  }
  bool LabelHasStmt(std::string s){
    return scopes.back().LabelHasStmt(s);
  }
private:
  std::vector<Scope> scopes;
};

extern SymbolTable st;