#pragma once

#include "tokenizer.h"

namespace gnilk
{

	#ifdef WIN32
	#define CALLCONV __stdcall
	#else
	#define CALLCONV
	#endif

	#define EXP_SOLVER_MAX_ARGS 32

	using namespace Goat;


	extern "C"
	{
		typedef double (CALLCONV *PFNEVALUATE)(void *pUser, const char *data, int *bOk_out);
		typedef double (CALLCONV *PFNEVALUATEFUNC)(void *pUser, const char *data, int args, double *arg, int *bOk_out);
	}

	class BaseNode
	{
	public:
		virtual ~BaseNode();
		virtual double Evaluate() = 0;
	};

	class ConstNode : 
		public BaseNode
	{
	protected:
		double numeric;
	public:
		ConstNode(const char *input, bool negative);
		virtual ~ConstNode();
		double Evaluate();
	};
	class ConstUserNode :
		public BaseNode
	{
	protected:
		void *pUser;
		const char *sData;
		PFNEVALUATE pCallback;
	public:
		ConstUserNode(PFNEVALUATE func, void *pUser, const char *input);
		virtual ~ConstUserNode();
		double Evaluate();
	};
	class FuncNode :
		public BaseNode
	{
	protected:
		void *pUser;
		const char *sFuncName;
		PFNEVALUATEFUNC pCallback;
		int args;
		BaseNode *pArgument[EXP_SOLVER_MAX_ARGS];
	public:
		FuncNode(PFNEVALUATEFUNC func, void *pUser, const char *name, BaseNode *pArg);
		FuncNode(PFNEVALUATEFUNC func, void *pUser, const char *name, int args, BaseNode **pArg);
		virtual ~FuncNode();
		double Evaluate();
	};
	class BinOpNode :
		public BaseNode
	{
	protected:
		const char *op;
		BaseNode *pLeft;
		BaseNode *pRight;
	public:
		BinOpNode(const char *op, BaseNode *pLeft, BaseNode *pRight);
		virtual ~BinOpNode();
		double Evaluate();
	};


	class ExpSolver
	{
	protected:
		PFNEVALUATE pVariableCallback;
		PFNEVALUATEFUNC pFuncCallback;

		void *pVariableContext;
		void *pFunctionContext;

		// int argcounter;
		// BaseNode *funcargs[EXP_SOLVER_MAX_ARGS];

		Tokenizer *tokenizer;
		BaseNode *tree;

		BaseNode *BuildUserCall();
		BaseNode *BuildTerm();
		BaseNode *BuildFact();
		BaseNode *BuildTree();

		typedef enum
		{
			kTokenClass_Unknown,
			kTokenClass_Numeric,
			kTokenClass_Variable,
		} kTokenClass;
		kTokenClass ClassifyFactor(const char *token);


	public:
		ExpSolver(const char *expression);
		virtual ~ExpSolver();
		void RegisterUserVariableCallback(PFNEVALUATE pFunc, void *pUser);
		void RegisterUserFunctionCallback(PFNEVALUATEFUNC pFunc, void *pUser);
		bool Prepare();
		double Evaluate();
        static bool Solve(double *out, const char *expression);
	};

}