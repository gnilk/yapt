/*-------------------------------------------------------------------------
File    : $Archive: ExpSolver.cpp $
Author  : $Author: FKling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Simple expression solver, 2 pass (build tree, solve tree) 
	  handles user callbacks for variables and function call's

Modified: $Date: $ by $Author: FKling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
 ! implement negative numbers 
 ! Multiple function arguments
 - '?' plus comparative operators
 + '>' and '<' in progress
</pre>


\History
- 04.08.14, FKling, Fixed bug related to priority of expressions and functions
                    Added multiple function arguments
                    Support for nested function calls
- 14.03.14, FKling, published on git hub
- 25.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
//#include <alloc.h>
#endif
#include <math.h>
#include "tokenizer.h"
#include "expsolver.h"

#include <vector>

using namespace gnilk;

BaseNode::~BaseNode()
{
	// nothing
}

ConstNode::ConstNode(const char *input, bool negative)
{
	numeric = atof(input);
	if (negative) {
		numeric *= -1;
	}
}

ConstNode::~ConstNode()
{
	// nothing
}

double ConstNode::Evaluate()
{
	return numeric;
}


ConstUserNode::ConstUserNode(PFNEVALUATE func, void *pUser, const char *input)
{
	this->pUser = pUser;
	pCallback = func;
	sData = strdup(input);
}

ConstUserNode::~ConstUserNode()
{
	free((void *)sData);
}

double ConstUserNode::Evaluate()
{
	int bOk = 0;
	return pCallback(pUser,sData,&bOk);
}

//
// Function node, implements user function callbacks.
// A function accepts only one argument, which is a tree
//
FuncNode::FuncNode(PFNEVALUATEFUNC func, void *pUser, const char *name, BaseNode *pArg)
{
	this->pUser = pUser;
	pCallback = func;
	args=0;
	sFuncName = strdup(name);
	pArgument[args++] = pArg;
}

FuncNode::FuncNode(PFNEVALUATEFUNC func, void *pUser, const char *name, int args, BaseNode **pArg)
{
	this->pUser = pUser;
	pCallback = func;
	sFuncName = strdup(name);
	this->args = 0;
	for(int i = 0; i<args; i++) {
		pArgument[this->args++] = pArg[i];
	}
}


FuncNode::~FuncNode()
{
	free((void *)sFuncName);
	for(int i=0;i<args;i++)
		delete (pArgument[i]);
	args = 0;
}

double FuncNode::Evaluate()
{
	int ok = 0;

	//printf("Calling '%s' with %d arguments\n", sFuncName, args);
	double values[EXP_SOLVER_MAX_ARGS];
	for(int i=0;i<args;i++) {
		values[i] = pArgument[i]->Evaluate();
	}
	return pCallback(pUser, sFuncName, args, values, &ok);
}

//
// Binary operation (left/right) node
//
BinOpNode::BinOpNode(const char *op, BaseNode *pLeft, BaseNode *pRight)
{
	this->op = strdup(op);
	this->pLeft = pLeft;
	this->pRight = pRight;
}

BinOpNode::~BinOpNode()
{
	free((void *)op);
	delete pLeft;
	delete pRight;
}

double BinOpNode::Evaluate()
{
	switch(op[0])
	{
	case '+' : return pLeft->Evaluate() + pRight->Evaluate();
	case '-' : return pLeft->Evaluate() - pRight->Evaluate();
	case '*' : return pLeft->Evaluate() * pRight->Evaluate();
	case '/' : return pLeft->Evaluate() / pRight->Evaluate();
	}
	printf("[!] Illegal operator: %s\n",op);
	return 0.0;
}

//
// Boolean operation
//
BoolOpNode::BoolOpNode(const char *op, BaseNode *pLeft, BaseNode *pRight) :
	BinOpNode(op, pLeft, pRight)
{
}

BoolOpNode::~BoolOpNode()
{
}

double BoolOpNode::Evaluate()
{
	//printf("BoolOpNode: %c\n",op[0]);
	switch(op[0])
	{
	case '>' : return pLeft->Evaluate() > (int)pRight->Evaluate();
	case '<' : return pLeft->Evaluate() < (int)pRight->Evaluate();
	// not sure about this...
	case '=' : return (int)pLeft->Evaluate() == (int)pRight->Evaluate();
	case '!' : return (int)pLeft->Evaluate() != (int)pRight->Evaluate();
	}
	printf("[!] Illegal operator: %s\n",op);
	return 0.0;
}
IfOperatorNode::IfOperatorNode(BaseNode *exp, BaseNode *pTrue, BaseNode *pFalse) 
{
	this->exp = exp;
	this->pTrue = pTrue;
	this->pFalse = pFalse;
}
IfOperatorNode::~IfOperatorNode() 
{
	delete exp;
	delete pTrue;
	delete pFalse;
}
double IfOperatorNode::Evaluate() 
{
//	printf("IfOperatorNode, evaluate\n");
	double res = exp->Evaluate();
	if (res > 0) {
		return pTrue->Evaluate();
	}
	return pFalse->Evaluate();
}

//
// constructor
//
ExpSolver::ExpSolver(const char *expression)
{
	//this->expression = strdup(expression);
	tokenizer = new Tokenizer(expression,"*/+-(),<>?:");
	pVariableCallback = NULL;
	pFuncCallback = NULL;
	tree = NULL;
}
bool ExpSolver::Solve(double *out, const char *expression)
{
    ExpSolver solver(expression);
    if (!solver.Prepare()) return false;
    *out = solver.Evaluate();
    return true;
}

ExpSolver::~ExpSolver()
{
	delete tokenizer;
	if (tree != NULL)
	{
		delete tree;
	}
}

//
// registration of variable handling
//
void ExpSolver::RegisterUserVariableCallback(PFNEVALUATE pFunc, void *pUser)
{
	pVariableCallback = pFunc;
	pVariableContext = pUser;
}

//
// registration of function callback's
//
void ExpSolver::RegisterUserFunctionCallback(PFNEVALUATEFUNC pFunc, void *pUser)
{
	pFuncCallback = pFunc;
	pFunctionContext = pUser;
}

//
// determines if a char is a numerical token or not
//
static bool IsNumeric(char c)
{
	static char *num="-0123456789";
	if (!strchr(num,c)) return false;
	return true;
}

//
// Classification of a token when building factors
//
ExpSolver::kTokenClass ExpSolver::ClassifyFactor(const char *token)
{
	kTokenClass result = kTokenClass_Unknown;
	if (IsNumeric(token[0]))
	{
		result = kTokenClass_Numeric;	
	} else
	{
		result = kTokenClass_Variable;
	}
	return result;
}

//
// user function calls and variables
//
BaseNode *ExpSolver::BuildUserCall()
{
	BaseNode *exp = NULL;
	BaseNode *arg = NULL;
	int argcounter = 0;
	BaseNode *funcargs[EXP_SOLVER_MAX_ARGS];


	const char *token = tokenizer->Next();

	const char *next = tokenizer->Peek();
	if ((next != NULL) && (next[0]=='('))
	{
		next = tokenizer->Next();

		// Assume arguments to function call..
		arg = BuildTree();

		// null == Empty expression - i.e. no parameters to function call
		if (arg != NULL) {
			funcargs[argcounter++] = arg;			
		}
		next = tokenizer->Peek();

		// Parse additional arguments
		while(next[0] == ',') {
			tokenizer->Next();
			arg = BuildTree();
			funcargs[argcounter++] = arg;
			next = tokenizer->Peek();
		}			

		if (next[0] == ')')
		{
			//printf("apa )\n");
			tokenizer->Next();
			if (pFuncCallback != NULL)
			{
				exp = new FuncNode(pFuncCallback, pFunctionContext, token, argcounter, funcargs);
			} else
			{
				printf("[!] Error: No functional callback assigned\n");
			}
		} else
		{
			printf("[!] Error: Unterminated function call: %s\n",token);
		}
	} else
	{
		// variable
		if (pVariableCallback != NULL)
		{
			exp = new ConstUserNode(pVariableCallback, pVariableContext, token);
		} else
		{
			printf("[!] Error: No variable callback defined, token=%s\n", token);
		}
	}
	return exp;
}

//
// build constant factors and sub-expressions
//
BaseNode *ExpSolver::BuildFact()
{
	BaseNode *exp = NULL;
	if (!tokenizer->HasMore()) return NULL;
	kTokenClass tc = kTokenClass_Unknown;
	const char *token = tokenizer->Peek();
	// classify next 
	if (token[0]=='(')	// Start of new expression, ok, build tree..
	{
		token = tokenizer->Next();

		exp = BuildTree();

		token = tokenizer->Peek();
		// Check if expression where terminated
		if (strcmp(token, ")"))
		{
			// error
			printf("[!] Error: Missing right parenthesis\n");
			return NULL;
		}
		tokenizer->Next();
	} else if (token[0]==')')	// empty expression
	{
		return NULL;
	} else if ((tc = ClassifyFactor(token)) != kTokenClass_Unknown)
	{
		switch (tc)
		{
		case kTokenClass_Numeric :
			{
				bool negative = false;
				token = tokenizer->Next();
				//printf("Numeric, next: %s\n", token);
				// Ugly - but I want to avoid string concat
				// will not handle multiple '--'
				if (token[0] == '-') {
					// negative numeric token
					token = tokenizer->Next();
					negative = true;

				}
				exp = new ConstNode(token, negative);

			}
			break;
		case kTokenClass_Variable :
			exp = BuildUserCall();
			break;
		}
	} else
	{
		printf("[!] Error: Unexpected token: %s\n",token);
	}
	return exp;
}

//
// builds high priority operators
//
BaseNode *ExpSolver::BuildTerm()
{
	BaseNode *exp;

	exp = BuildFact();	// build factory

	if (tokenizer->HasMore())
	{
		const char *token = tokenizer->Peek();
		//while(Tokenizer::Case(token,"* /") >= 0)
		while((token != NULL) && ((token[0]=='*') || (token[0]=='/')))
		{
			//printf("term\n");
			token = tokenizer->Next();
			BaseNode *next = BuildFact();
			exp = new BinOpNode(token, exp, next);
			token = tokenizer->Peek();
		}
	}
	return exp;
}

// 
// internal, builds the expression tree
// handles low priority operators - also called internally
//
BaseNode *ExpSolver::BuildBase()
{
	BaseNode *exp;
	exp = BuildTerm();
	if (tokenizer->HasMore())
	{
		const char *token = tokenizer->Peek();
		while ((token != NULL) && ((token[0]=='+') || (token[0]=='-')))
		{
			token = tokenizer->Next();
			BaseNode *nextTerm = BuildTerm();
			exp = new BinOpNode(token, exp, nextTerm);
			token = tokenizer->Peek();
		}
	}
	return exp;
}
BaseNode *ExpSolver::BuildBool() 
{
	BaseNode *exp;
	exp = BuildBase();
	if (tokenizer->HasMore())
	{
		const char *token = tokenizer->Peek();
		//printf("BuildBool, token=%s",token);
		while ((token != NULL) && ((token[0]=='>') || (token[0]=='<')))
		{
			token = tokenizer->Next();
			//printf("BuildBool, Next as BuildBase\n");
			BaseNode *nextBase = BuildBase();
			exp = new BoolOpNode(token, exp, nextBase);
			token = tokenizer->Peek();
			//printf("BuildBool, done, next token=%s\n",token);
		}
	}  else {
		//printf("BuildBool, no more data\n");
	}
	return exp;
}

BaseNode *ExpSolver::BuildIf() 
{
	BaseNode *exp;
	exp = BuildBool();
	if (tokenizer->HasMore())
	{
		const char *token = tokenizer->Peek();
		//printf("BuildIf, HasMore, token=%s\n",token);
		while ((token != NULL) && (token[0]=='?'))
		{
			token = tokenizer->Next();
			//printf("BuildIf, build true\n");
			BaseNode *pTrue = BuildTree();
			if (pTrue == NULL) {
				printf("[!] Error: Operator mismatch, use <exp>?<true>:<false>\n");
				return NULL;
			}

			token = tokenizer->Peek();
			if ((token == NULL) || (token[0] != ':')) {
				printf("[!] Error: token error, expected ':' got '%s'\n",token);
				return NULL;
			}
			token = tokenizer->Next();
			BaseNode *pFalse = BuildTree();
			exp = new IfOperatorNode(exp, pTrue, pFalse);

			token = tokenizer->Peek();
		}
	} else {
		//printf("BuildIf, no more data\n");
	}
	//printf("BuildIf, done, exp=%p\n", exp);
	return exp;
}

BaseNode *ExpSolver::BuildTree()
{
	BaseNode *exp = BuildIf();
	return exp;
}

// boolean stuff here
//
// Prepare the expression = build the expression tree
//
bool ExpSolver::Prepare()
{
	if (tree != NULL)
	{
		delete tree;
		tree = NULL;
	}
	// This allows for multi-expression and is the basis for a proper interpreter
	while(tokenizer->HasMore()) {
		BaseNode *exp = BuildTree();
		//printf("Next: %s\n", tokenizer->Peek());
		nodes.push_back(exp);
	}

	// backwards compatible
	tree = nodes[0];

	return true;
}

//
// Evaluate a prepared expression
//
double ExpSolver::Evaluate()
{
	double result = 0.0;
	//printf("Nodes: %d\n", nodes.size());
	if (tree != NULL)
	{
		result = tree->Evaluate();
	}
	return result;
}