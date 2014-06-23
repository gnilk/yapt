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
 - implement negative numbers 
 - Multiple function arguments
</pre>


\History
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

using namespace Goat;

BaseNode::~BaseNode()
{
	// nothing
}

ConstNode::ConstNode(const char *input)
{
	numeric = atof(input);
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
	sFuncName = strdup(name);
	pArgument = pArg;
}

FuncNode::~FuncNode()
{
	free((void *)sFuncName);
	delete (pArgument);
}

double FuncNode::Evaluate()
{
	int ok = 0;
	double arg = pArgument->Evaluate();
	return pCallback(pUser, sFuncName, arg, &ok);
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
// constructor
//
ExpSolver::ExpSolver(const char *expression)
{
	//this->expression = strdup(expression);
	tokenizer = new Tokenizer(expression,"*/+-()");
	pVariableCallback = NULL;
	tree = NULL;
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
	pVariableContext = pUser;
}

//
// determines if a char is a numerical token or not
//
static bool IsNumeric(char c)
{
	static char *num="0123456789";
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
	const char *token = tokenizer->Next();

	const char *next = tokenizer->Peek();
	if ((next != NULL) && (next[0]=='('))
	{
		next = tokenizer->Next();
		BaseNode *arg = BuildTree();
		next = tokenizer->Peek();
		// TODO: if next == ',' create tree and push on argument stack
		if (next[0] == ')')
		{
			tokenizer->Next();
			if (pFuncCallback != NULL)
			{
				exp = new FuncNode(pFuncCallback, pFunctionContext, token, arg);
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
			printf("[!] Error: No variable callback defined\n");
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
	kTokenClass tc = kTokenClass_Unknown;
	const char *token = tokenizer->Peek();

	// classify next 
	if ((tc = ClassifyFactor(token)) != kTokenClass_Unknown)
	{
		switch (tc)
		{
		case kTokenClass_Numeric :
			token = tokenizer->Next();
			exp = new ConstNode(token);
			break;
		case kTokenClass_Variable :
			exp = BuildUserCall();
			break;
		}
	} else if (token[0]=='(')	// Start of new expression, ok, build tree..
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
BaseNode *ExpSolver::BuildTree()
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
	tree = BuildTree();
	return true;
}

//
// Evaluate a prepared expression
//
double ExpSolver::Evaluate()
{
	double result = 0.0;
	if (tree != NULL)
	{
		result = tree->Evaluate();
	}
	return result;
}