#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include "symbol.hpp"
#include "lexer.hpp"
#include "ast.hpp"

class Library{
public:
	Library(){};
	void init(){
		Formal *f, *p;
		Formal_list *formal_list;
		Id_list *idlist;
		char const *i, *l;
		//char *i, *l;
		// Procedure *proc;
		// Function_not_from_llvm *func;
		Type_not_from_llvm *type;
		
		//writeInteger (n: integer)
		formal_list = new Formal_list();
		idlist = new Id_list();
		i = "dummy4207210396";
		idlist->append_char(i);
		type = new Int();
		f = new Formal(idlist, type);
		formal_list->append(f);
		
		//formal_list->append(f);

		st.insertProcedure("writeInteger", TYPE_proc, formal_list, false);	//maybe make a new function in st (?)

		//proc = new Procedure("writeInteger", f, formal_list);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Bool();
		f = new Formal(idlist, type);
		formal_list->append(f);
		st.insertProcedure("writeBoolean", TYPE_proc, formal_list, false);
		//proc = new Procedure("writeBoolean", f, formal_list); 

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);
		type = new Char();
		f = new Formal(idlist, type);
		formal_list->append(f);
		st.insertProcedure("writeChar", TYPE_proc, formal_list, false);
		//proc = new Procedure("writeChar", f, formal_list);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// proc = new Procedure("writeReal", f, formal_list);
		st.insertProcedure("writeReal", TYPE_proc, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Array(TYPE_char);
		f = new Formal(idlist, type);
		formal_list->append(f);

		// proc = new Procedure("writeString", f, formal_list);
		st.insertProcedure("writeString", TYPE_proc, formal_list, false);

		//READ FUNCS

		formal_list = new Formal_list();
	    st.insertFunction("readInteger", TYPE_int, formal_list, false);

	    formal_list = new Formal_list();
	    st.insertFunction("readBoolean", TYPE_bool, formal_list, false);

	    formal_list = new Formal_list();
	    st.insertFunction("readChar", TYPE_char, formal_list, false);

    	formal_list = new Formal_list();
    	st.insertFunction("readReal", TYPE_real, formal_list, false);

		// func = new Function_not_from_llvm("readInteger", new Int());
		// func = new Function_not_from_llvm("readBoolean", new Bool());
		// func = new Function_not_from_llvm("readChar", new Char());
		// func = new Function_not_from_llvm("readReal", new Real());

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Int();
		f = new Formal(idlist, type);
		formal_list->append(f);

		idlist = new Id_list();
		
		
		l = "dummy4207210396";
		idlist->append_char(l);

		type = new Array(TYPE_char);
		p = new Formal(idlist, type);
		formal_list->append(p);

		// proc = new Procedure("readString", f, formal_list);
		st.insertProcedure("readString", TYPE_proc, formal_list, false);

		//MATH FUNCS

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Int();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("abs", f, formal_list, new Int());
		st.insertFunction("abs", TYPE_int, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("fabs", f, formal_list, new Real());
		st.insertFunction("fabs", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("sqrt", f, formal_list, new Real());
		st.insertFunction("sqrt", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);
		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("sin", f, formal_list, new Real());
		st.insertFunction("sin", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("cos", f, formal_list, new Real());
		st.insertFunction("cos", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("tan", f, formal_list, new Real());
		st.insertFunction("tan", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("arctan", f, formal_list, new Real());
		st.insertFunction("arctan", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("exp", f, formal_list, new Real());
		st.insertFunction("exp", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("ln", f, formal_list, new Real());
		st.insertFunction("ln", TYPE_real, formal_list, false);


		// func = new Function_not_from_llvm("pi", new Real());
		formal_list = new Formal_list();
    	st.insertFunction("pi", TYPE_real, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("trunc", f, formal_list, new Int());
		st.insertFunction("trunc", TYPE_int, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Real();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("round", f, formal_list, new Int());
		st.insertFunction("round", TYPE_int, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Char();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("ord", f, formal_list, new Int());
		st.insertFunction("ord", TYPE_int, formal_list, false);

		formal_list = new Formal_list();
		idlist = new Id_list();
		
		i = "dummy4207210396";
		idlist->append_char(i);

		type = new Int();
		f = new Formal(idlist, type);
		formal_list->append(f);

		// func = new Function_not_from_llvm("chr", f, formal_list, new Char());
		st.insertFunction("chr", TYPE_char, formal_list, false);
	}
};

