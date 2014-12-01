#pragma once

#include <string>

namespace gnilk
{
	// Simple string tokenizer
	// note: the tokenizer keeps states between calls to 'next'
	class Tokenizer
	{
	private:
		std::string operators;
		std::string::iterator it;
		std::string data;

		// check if a character at iterator is an operator
		bool isOperator(std::string::iterator op)
		{
			return (operators.find(*op) != std::string::npos);
		}

		// Initialize data structures
		void Initialize(std::string _data, std::string _operators)
		{
			data = _data;
			it = data.begin();
			operators = _operators;
		}
	public:

		// initialization without operators
		Tokenizer(std::string str) {
			Initialize(str, "");
		}

		//
		// Initialization with single character operators
		// and operator causes the tokenizer to halt and return token
		// next token will be the operator
		//
		Tokenizer(std::string str, std::string operators) {
			Initialize(str, operators);
		}

		//
		// Returns curent token and moves to next
		//
		std::string Next() {
			while(it != data.end() && isspace(*it)) it++;
			if (it == data.end()) return std::string();

			if (isOperator(it)) {
				return std::string(1,*it++);
			}
			std::string::iterator start = it;
			while (!isspace(*it) && !isOperator(it) && (it != data.end())) it++;
			return std::string(start, it);
		}

		// Function for switch statements
		// Tokenizer::Case("hej","apa hej wef") - returns 1
		// -1 means no hit
		// use like: switch(Tokenizer::Case(...)) {
		static int Case(std::string tok, std::string opts) {
			int optCounter = 0;
			std::string tmp;
			Tokenizer tokenizer(opts);
			while(!(tmp = tokenizer.Next()).empty()) {
				if (tmp == tok) return optCounter;
				optCounter++;
			}
			return -1;
		}

	};
}

