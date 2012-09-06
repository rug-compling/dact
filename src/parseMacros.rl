// Experimental parser for macros
//
// TODO:
//
// - Eat whitespace between opening/closing quotes and the actual query.
// - Add error handling.
// - If everything works, remove main.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>

#include <cstring>

namespace {

%%{
	machine macros;
	write data;
}%%


struct Substitution
{
	size_t begin;
	size_t n;
	std::string macro;
};

}

typedef std::map<std::string, std::string> Macros;
typedef std::vector<Substitution> Substitutions;

Macros parseMacros(char const *data)
{
	int cs = 0;
	char const *p = data;
	char const *pe = p + std::strlen(data);
	char const *eof = p + std::strlen(data);

	char const *strStart = p;

	char const *substStart = 0;

	Macros macros;

	Substitutions substitutions;

	std::string lastKey;

	std::ostringstream buf;
%%{
	action str_char {
		buf << fc;
	}

	action key {
		lastKey = buf.str();
		buf.str("");
	}

	action queryStart {
		strStart = p;
	}

	action queryEnd {
		// This action is executed when the query is closed ("""), we have to
		// three to avoid including the quotes.
		std::string query(strStart, p - 3);

		// Apply substitutions that were found in the macro.
		for (Substitutions::const_reverse_iterator iter = substitutions.rbegin();
			iter != substitutions.rend(); ++iter)
		{
			Macros::const_iterator mIter = macros.find(iter->macro);
			if (mIter == macros.end())
			{
				std::cerr << "Unknown macro: " << iter->macro << std::endl;
				continue;
			}

			query.replace(iter->begin, iter->n, mIter->second);
		}

		substitutions.clear();
		macros[lastKey] = query;
	}

	action substBegin {
		substStart = p;
	}

	action substEnd {
		std::string macro(substStart, p);

		// substStart is the position of the macro name. Decrement by one to
		// get the position of the percentage sign.
		size_t begin = substStart - strStart - 1;

		// Add two, to account for both percentage signs.
		size_t n = p - substStart + 2;

		Substitution subst = {begin, n, macro};
		substitutions.push_back(subst);
	}

	separator = "\"\"\"";
	whitespace = [\n\r\t ]+;

	key = ([A-Za-z0-9_]+) $ str_char % key;

	substitution = ('%' % substBegin) (any - '%')+ ('%' > substEnd);
	query = (substitution | (any - '%')+)* -- separator;
	queryVal = (separator % queryStart) query (separator % queryEnd);

	main := (whitespace* key whitespace* '=' whitespace* queryVal)*;

	write init;
	write exec;
}%%

	return macros;
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
		return 1;

	std::ifstream in(argv[1]);
	if (!in)
		return 1;

	in >> std::noskipws;

	std::ostringstream iss;
	std::copy(std::istream_iterator<char>(in),
		std::istream_iterator<char>(),
		std::ostream_iterator<char>(iss));

	std::string data = iss.str();
	//std::string test = "\"\"\"test\"\"\"";

	//parseMacros(test.c_str());

	Macros macros = parseMacros(data.c_str());
	for (Macros::const_iterator iter = macros.begin(); iter != macros.end(); ++iter)
		std::cout << iter->first << ": " << iter->second << std::endl;
}