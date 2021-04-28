#include <string>
#include <unordered_map>
#include <stack>
#include <vector>
#include <typeinfo>

#include "ast.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>

using namespace std;


void llvm_compile_and_dump();

class Formal_list;

/* ---------------------------------------------------------------------
   ------------------------------- Scopelog ----------------------------
   ---------------------------------------------------------------------
   > variableTypes:     types of all variables
   > variableAllocas:   addresses of the stack slots of all variables
   > pointerTypes:      types of all pointers
   > arrayTypes:        types of all arrays
   > functions:         all functions
   > FunctionsResults:  contains the results of each function
   > labelsbody:        all bodies of labels
   > labelscont:        all blocks after stmts of a label
   > FunctionsNoBody:         stack of the names of all functions with no body declared yet
 ----------------------------------------------------------------------- */

typedef struct {
    unordered_map<string, llvm::Type*> variableTypes;
    unordered_map<string, llvm::Type*> pointerTypes;
    unordered_map<string, llvm::Type*> arrayTypes;
    unordered_map<string, llvm::Type*> FuncTypes;
    unordered_map<string, llvm::AllocaInst*> variableAllocas;
    unordered_map<string, llvm::Function*> functions;
    unordered_map<string, llvm::BasicBlock*> functions_ends;
    unordered_map<string, llvm::Function*> procedures;
    unordered_map<string, llvm::Value*> FunctionsResults;
    unordered_map<string, llvm::BasicBlock*> labelsbody;
    unordered_map<string, llvm::BasicBlock*> labelscont;
    stack<string> FunctionsNoBody;
    stack<string> ProcsNoBody;
    unordered_map<string, Formal_list *> ProcsFormals;
    unordered_map<string, Formal_list *> FuncsFormals;
} scopeLog;

/* ---------------------------------------------------------------------
   ------------- Logger: scope, variable and function info -------------
   --------------------------------------------------------------------- */

class LoggedInfo {
private:
    vector<scopeLog> scopeLogs;
public:
    LoggedInfo() {
        this->openScope();
    };
    
    // create and push new scopelog
    void openScope() {
        scopeLog sl;
        this->scopeLogs.push_back(sl);
    };
    
    // pop scopelog
    void closeScope() {
        this->scopeLogs.pop_back();
    };

    // add a variable to current scopelog
    void addVariable(string id, llvm::Type *type, llvm::AllocaInst *alloca) {
        this->scopeLogs.back().variableTypes[id] = type;
        this->scopeLogs.back().variableAllocas[id] = alloca;
    };

    // change address of stack slot (used only in new)
    void changeAlloca(string id, llvm::Type *type, llvm::AllocaInst *alloca) {
    	this->scopeLogs.back().variableAllocas[id] = alloca;
        this->scopeLogs.back().variableTypes[id] = type;
    };

    // pointer point to nullptr (dispose)
    void disposePointer(string id) {
        this->scopeLogs.back().variableAllocas[id] = nullptr;
    };

    // store the type of array
    void arrayType(string id, llvm::Type *type) {
    	this->scopeLogs.back().arrayTypes[id] = type;
    };

    // store the type of pointer
    void pointerType(string id, llvm::Type *type){
    	this->scopeLogs.back().pointerTypes[id] = type;
    };

    // store a label with its basic block
    void addLabel(string id, llvm::BasicBlock *body, llvm::BasicBlock *cont){
    	this->scopeLogs.back().labelsbody[id] = body;
        this->scopeLogs.back().labelscont[id] = cont;
    };

    // push the name of a non body function
    void pushFunc(string func){
        this->scopeLogs.back().FunctionsNoBody.push(func);
    };

    // is the function stack empty?
    bool isFuncStackEmpty() {
        return this->scopeLogs.back().FunctionsNoBody.empty();
    };

    // get the name of the last non body function
    string getLastFunc(){
        string ret = this->scopeLogs.back().FunctionsNoBody.top();
        return ret;
    };

    // remove current func from stack when we are done
    void removeCurrFunc() {
        this->scopeLogs.back().FunctionsNoBody.pop();
    };

    // add function's formals
    void addFuncFormals(string fname, Formal_list *formals){
        this->scopeLogs.back().FuncsFormals[fname] = formals;
    };

    // add function's type
    void addFuncType(string fname, llvm::Type *t){
        this->scopeLogs.back().FuncTypes[fname] = t;
    };

    // add end of func
    void storeEndOfFunc(string fname, llvm::BasicBlock *b){
        this->scopeLogs.back().functions_ends[fname] = b;
    };

    // get end of func
    llvm::BasicBlock * getEndOfFunc(string fname){
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->functions_ends.find(fname) == it->functions_ends.end()))
                return it->functions_ends[fname];
        }
        std::cerr << "ERROR: Func "<< fname << " encounterd a problem\n";
        return nullptr;
    }

    // get function's type
    llvm::Type * getFuncType(string fname){
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->FuncTypes.find(fname) == it->FuncTypes.end()))
                return it->FuncTypes[fname];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Func " << fname << " not in scope.\n";
        return nullptr;
    };

    // get function's formals
    Formal_list * getFuncFormals(string fname){
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->FuncsFormals.find(fname) == it->FuncsFormals.end()))
                return it->FuncsFormals[fname];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Func " << fname << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // push the name of a non body proc
    void pushProc(string proc){
        this->scopeLogs.back().ProcsNoBody.push(proc);
    };

    // is the procedure stack empty?
    bool isProcStackEmpty() {
        return this->scopeLogs.back().ProcsNoBody.empty();
    };

    // get the name of the last non body procedure
    string getLastProc(){
        string ret = this->scopeLogs.back().ProcsNoBody.top();
        return ret;
    };

    // remove current proc from stack when we are done
    void removeCurrProc() {
        this->scopeLogs.back().ProcsNoBody.pop();
    };

    // add procedures formals
    void addProcFormals(string pname, Formal_list *formals){
        this->scopeLogs.back().ProcsFormals[pname] = formals;
    };

    // get procedures formals
    Formal_list * getProcFormals(string pname){
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->ProcsFormals.find(pname) == it->ProcsFormals.end()))
                return it->ProcsFormals[pname];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Proc " << pname << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // store the Result of a function
    void storeResult(string fname, llvm::Value *res){
        this->scopeLogs.back().FunctionsResults[fname] = res;
    };

    llvm::Value * getFuncResult(string fname){
        return this->scopeLogs.back().FunctionsResults[fname];
    };

    // lookup variable by id and return type
    llvm::Type * getVarType(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->variableTypes.find(id) == it->variableTypes.end()))
                return it->variableTypes[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Variable " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // lookup array by id and return type
    llvm::Type * getArrType(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->arrayTypes.find(id) == it->arrayTypes.end()))
                return it->arrayTypes[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Array " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // lookup pointer by id and return type
    llvm::Type * getPoinType(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->pointerTypes.find(id) == it->pointerTypes.end()))
                return it->pointerTypes[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Pointer " << id << " not in scope(from getPoinType).\n";
        exit(1);
        return nullptr;
    };

    // lookup variable by id and return address of stack slot
    llvm::AllocaInst * getVarAlloca(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->variableAllocas.find(id) == it->variableAllocas.end()))
                return it->variableAllocas[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Variable's allocation " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // get the block of code after a label
    llvm::BasicBlock * getLabelBlock(string id){
    	for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->labelsbody.find(id) == it->labelsbody.end()))
                return it->labelsbody[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Label " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // get the block after our label's block
    llvm::BasicBlock * getLabelCont(string id){
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->labelscont.find(id) == it->labelscont.end()))
                return it->labelscont[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Label " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // lookup variable by id and return true if it is a pointer and false otherwise
    bool isPointer(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->variableTypes.find(id) == it->variableTypes.end()))
                return it->variableTypes[id]->isPointerTy();
        }
        return false;
        // if sem was ok, this point should be unreachable
        std::cerr << "Pointer to " << id << " not in scope.(from isPointer)\n";
        exit(1);
        
    };

    // add function to scopelog
    void addFunctionInScope(string fname, llvm::Function *F) {
    		this->scopeLogs.back().functions[fname] = F;
    };

    // add proc to scopelog
    void addProcInScope(string pname, llvm::Function *P) {
            this->scopeLogs.back().procedures[pname] = P;
    };

    // lookup function by id
    llvm::Function * getFunctionInScope(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->functions.find(id) == it->functions.end()))
                return it->functions[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Function " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // lookup procedure by id
    llvm::Function * getProcInScope(string id) {
        for (auto it = this->scopeLogs.rbegin(); it != this->scopeLogs.rend(); ++it) {
            if (!(it->procedures.find(id) == it->procedures.end()))
                return it->procedures[id];
        }
        // if sem was ok, this point should be unreachable
        std::cerr << "Procedure " << id << " not in scope.\n";
        exit(1);
        return nullptr;
    };

    // getter
    unordered_map<string, llvm::Type*> getCurrentScopeVarTypes() {
        return this->scopeLogs.back().variableTypes;
    };

    // getter
    unordered_map<string, llvm::AllocaInst*> getCurrentScopeVarAllocas() {
        return this->scopeLogs.back().variableAllocas;
    };
};

extern LoggedInfo loggedinfo;