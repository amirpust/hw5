#ifndef EX5_CODE_GEN
#define EX5_CODE_GEN

#include <vector>
#include <string>
#include "Exp_t.hpp"
#include "Symbol.hpp"

using namespace std;

//this enum is used to distinguish between the two possible missing labels of a conditional branch in LLVM during backpatching.
//for an unconditional branch (which contains only a single label) use FIRST.
enum BranchLabelIndex {FIRST, SECOND};

class CodeBuffer{
	CodeBuffer();
	CodeBuffer(CodeBuffer const&);
    void operator=(CodeBuffer const&);
	std::vector<std::string> buffer;
	std::vector<std::string> globalDefs;
public:
	static CodeBuffer &instance();
	void firstEmit(){
        declareExterns();
	    definePrints();
	    defineDivideByZero();
	};
    void declareExterns(){
        emit("@.intFormat = internal constant [4 x i8] c\"%d\\0A\\00\"");
        emit("@.DIVIDE_BY_ZERO.str = internal constant [23 x i8] c\"Error division by zero\\00\"");

        emit("declare i32 @printf(i8*, ...)");
        emit("declare void @exit(i32)");

        emit("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
        emit("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    }
    void definePrints(){
        emit("define void @print(i8*){");
        emit("call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
        emit("ret void");
        emit("}");

        emit("define void @printi(i32){");
        emit("%format_ptr = getelementptr [4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0");
        emit("call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0), i32 %0)");
        emit("ret void");
        emit("}");
    }
	void defineDivideByZero(){
        emit("define void @checkDivideByZero(i32){");

        emit("%checkIfZero = icmp eq i32 %0 , 0");
        emit("br i1 %checkIfZero, label %DIVIDE_BY_ZERO, label %DONE");
        
        emit("DONE:");
        emit("ret void");

        emit("DIVIDE_BY_ZERO:");
        emit("call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIVIDE_BY_ZERO.str, i32 0, i32 0))");
        emit("call void @exit(i32 0)");
        emit("ret void");

        emit("}");
    }

	// ******** Methods to handle the code section ******** //

	//generates a jump location label for the next command, writes it to the buffer and returns it
	std::string genLabel();

	//writes command to the buffer, returns its location in the buffer
	int emit(const std::string &command);

	//gets a pair<int,BranchLabelIndex> item of the form {buffer_location, branch_label_index} and creates a list for it
	static vector<pair<int,BranchLabelIndex>> makelist(pair<int,BranchLabelIndex> item);

	//merges two lists of {buffer_location, branch_label_index} items
	static vector<pair<int,BranchLabelIndex>> merge(const vector<pair<int,BranchLabelIndex>> &l1,const vector<pair<int,BranchLabelIndex>> &l2);

	/* accepts a list of {buffer_location, branch_label_index} items and a label.
	For each {buffer_location, branch_label_index} item in address_list, backpatches the branch command 
	at buffer_location, at index branch_label_index (FIRST or SECOND), with the label.
	note - the function expects to find a '@' char in place of the missing label.
	note - for unconditional branches (which contain only a single label) use FIRST as the branch_label_index.
	example #1:
	int loc1 = emit("br label @");  - unconditional branch missing a label. ~ Note the '@' ~
	bpatch(makelist({loc1,FIRST}),"my_label"); - location loc1 in the buffer will now contain the command "br label %my_label"
	note that index FIRST referes to the one and only label in the line.
	example #2:
	int loc2 = emit("br i1 %cond, label @, label @"); - conditional branch missing two labels.
	bpatch(makelist({loc2,SECOND}),"my_false_label"); - location loc2 in the buffer will now contain the command "br i1 %cond, label @, label %my_false_label"
	bpatch(makelist({loc2,FIRST}),"my_true_label"); - location loc2 in the buffer will now contain the command "br i1 %cond, label @my_true_label, label %my_false_label"
	*/
	void bpatch(const vector<pair<int,BranchLabelIndex>>& address_list, const std::string &label);
	
	//prints the content of the code buffer to stdout
	void printCodeBuffer();

	// ******** Methods to handle the data section ******** //
	//write a line to the global section
	void emitGlobal(const string& dataLine);
	//print the content of the global buffer to stdout
	void printGlobalBuffer();

	string emitAlloca(){
        string newRbp = getNewRegister();
        emit(newRbp + " = alloca i32, i32 50");
        return newRbp;
    }

    void emitStore(Exp_t* E, string rbpReg){
        string ptr = getNewRegister("ptr");
        emit(ptr + " = getelementptr i32, i32* " + rbpReg + ", i32 " + to_string(E->offset));
        emit("store i32 " + E->regName + ", i32* " + ptr); //TODO: test

    }

    string emitLoad(Exp_t* E, string rbpReg){
        string newReg = getNewRegister();
        string ptr = getNewRegister("ptr");

        emit(ptr + " = getelementptr i32, i32* " + rbpReg + ", i32 " + to_string(E->offset));
        emit(newReg + " = load i32, i32* " + ptr); //TODO: test
        E->regName = newReg;
        return newReg;
    }

    void emitOp(Exp_t* E, Exp_t* E1, const string op, Exp_t* E2){
	    output::printLog("emitOP: reg " + E->regName + " type: " + E->t.getStr());
        emit(E->regName + " = " + op + " i32 " + E1->regName + ", " + E2->regName);
        if (E->t == E_byte ||E->t == E_bool ){
            string newReg = getNewRegister();
            // and i32 4, %var
            emit(newReg + " = and i32 255, " + E->regName);
            E->regName = newReg;
        }

    }

    void emitAssign(Exp_t* dst, Exp_t* src, string rbp){
	    dst->regName = src->regName;
	    emitStore(dst, rbp);
    }

    void emitFuncDefenition(IDtype id, SymList args, Type retType){
	    //define void @print(i8*){
        string llvmArgs = "";
	    for(auto sym: args.symList){
            llvmArgs += getLlvmType(sym.getType());
            if (sym.getId() != (args.symList.back().getId())){
                llvmArgs += ", ";
            }
	    }
        emit("define " + getLlvmType(retType) + " @" + id.id + "(" + llvmArgs+"){");
    }

    void emitCloseFunc(){
	    emit("}");
	}

	void emitCallFunc(Exp_t* retVal, IDtype funcName, ExpList arguments){
        string llvmArgs = "";

        for(auto exp: arguments.expList){

            llvmArgs += getLlvmType(exp.t);
            llvmArgs += " ";
            llvmArgs += exp.regName;

                    //exp.regName();
            if (exp.regName != (arguments.expList.back().regName)){
                llvmArgs += ", ";
            }
        }

        //    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)
        if (retVal->t == E_void){
            emit("call " + getLlvmType(retVal->t) + " @" + funcName.id + "(" + llvmArgs+")");
        }else{
            emit(retVal->regName + " = call " + getLlvmType(retVal->t) + " @" + funcName.id + "(" + llvmArgs+")");
        }
	}

	void emitReturn(Exp_t* retType){
	    if(retType->t.t == E_void){
	        emit("ret void");

	    }else{

	        emit("ret " + getLlvmType(retType->t.t) + " " + retType->regName);
	    }
	}

	void emitSaveString(Exp_t* exp, String str){
	    //"@.DIVIDE_BY_ZERO.str = internal constant [23 x i8] c\"Error division by zero\\00\""
	    exp->regName = getNewGlobalRegister("string");
	    str.val.pop_back();
        emitGlobal(exp->regName + " = constant [" + to_string(str.val.length()) + " x i8] c" + str.val + "\\00\"");
	}

private:
    string getLlvmType(Type t){
        if (t == E_void)
            return "void";
        if (t == E_string)
            return  "i8*";
        return "i32";
	}
};

#endif

