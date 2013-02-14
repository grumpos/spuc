
#include <cstdint>
//#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;



bool foo(string& v );

/*

struct type_sym
{
	string name;
	struct 
	{
		uint8_t is_const : 1;
		uint8_t is_ref : 1;
		uint8_t is_ptr : 1;
	} flags;
};

struct lval_sym
{
	string name;
	type_sym type;
};

struct fn_sym
{
	string name;
	vector<lval_sym> args;
	type_sym return_type;
};


string print(const type_sym& ts)
{
	string str;
	str += ts.flags.is_const ? "const " : "";
	str += ts.name;
	str += ts.flags.is_ref ? "&" : "";
	str += ts.flags.is_const ? "*" : "";
	str += " ";

	return str;
}

string print2(const lval_sym& lvs)
{
	return print(lvs.type) + lvs.name;
}

string print(const fn_sym& fs)
{
	string str;
	str += print(fs.return_type);
	str += fs.name;
	str += "(";
	for_each(fs.args.cbegin(), --fs.args.cend(),
		[&](const lval_sym& lvs)
	{
		str += print2(lvs);
		str += ",";
	});
	str += print2(fs.args.back());
	str += ")";

	return str;
}

string(*fptr)(const lval_sym& lvs) = print2;


void symbol_test()
{
	cout << "Symbol Testing" << endl;

	type_sym xmm_type;
	xmm_type.name = "__m128";
	xmm_type.flags.is_const = 1;

	cout << print(xmm_type) << endl;

	lval_sym arg;
	arg.name = "arg0";
	arg.type = xmm_type; 

	cout << print2(arg) << endl;

	fn_sym fun;
	fun.name = "myfun";
	fun.return_type = xmm_type;
	fun.args.push_back(arg);
	fun.args.push_back(arg);
	fun.args.push_back(arg);

	cout << print(fun) << endl;
}*/