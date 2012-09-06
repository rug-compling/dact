// Experimental parser for macros
//
// TODO:
//
// - Store macros and keys nicely in a map
// - Expand queries, marked with percent signs.
// - If everything works, remove main.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <cstring>

%%{
	machine macros;
	write data;
}%%

typedef std::map<std::string, std::string> Macros;

Macros parseMacros(char const *data)
{
	int cs = 0;
	char const *p = data;
	char const *pe = p + std::strlen(data);
	char const *eof = p + std::strlen(data);

	char const *strStart = p;

	char const *substStart = 0;

	Macros macros;

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
		std::string query(strStart, p - 3);
		macros[lastKey] = query;
	}

	action substBegin {
		substStart = p;
	}

	action substEnd {
		std::string subst(substStart, p);
		std::cerr << "Found substitution: " << subst << std::endl;
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