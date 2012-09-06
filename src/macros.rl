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
#include <sstream>
#include <cstring>

%%{
	machine macros;
	write data;
}%%

void parseMacros(char const *data)
{
	int cs = 0;
	char const *p = data;
	char const *pe = p + std::strlen(data);
	char const *eof = p + std::strlen(data);

	char const *strStart = p;
	char const *strEnd = eof;

	std::ostringstream buf;
%%{
	action str_char {
		buf << fc;
	}

	action key {
		std::cout << "key: " << buf.str() << std::endl;
		buf.str("");
	}

	action queryStart {
		strStart = p;
	}

	action queryEnd {
		std::string query(strStart, p - 3);
		std::cout << "value: " << query << std::endl;
	}

	separator = "\"\"\"";
	whitespace = [\n\r\t ]+;

	key = ([A-Za-z0-9_]+) $ str_char % key;

	query = (any* -- separator);
	queryVal = (separator % queryStart) query (separator % queryEnd);

	main := (whitespace* key whitespace* '=' whitespace* queryVal)*;

	write init;
	write exec;
}%%
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

	parseMacros(data.c_str());
}