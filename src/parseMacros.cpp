
#line 1 "parseMacros.rl"
// Experimental parser for macros
//
// TODO:
//
// - Eat whitespace between opening/closing quotes and the actual query.
//
// The C++ source file is automatically generated using Ragel. If you
// make changes to the automaton, use:
//
//  ragel parseMacros.rl -o parseMacros.cpp
//

#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <cstring>

#include <parseMacros.hh>

namespace {


#line 29 "parseMacros.cpp"
static const char _macros_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 2, 3, 0
	
};

static const char _macros_key_offsets[] = {
	0, 0, 11, 23, 28, 33, 34, 35, 
	37, 39, 41, 43, 45, 47, 49, 51, 
	62, 74, 85, 97
};

static const char _macros_trans_keys[] = {
	13, 32, 95, 9, 10, 48, 57, 65, 
	90, 97, 122, 13, 32, 61, 95, 9, 
	10, 48, 57, 65, 90, 97, 122, 13, 
	32, 61, 9, 10, 13, 32, 34, 9, 
	10, 34, 34, 34, 37, 34, 37, 34, 
	37, 34, 37, 34, 37, 34, 37, 34, 
	37, 34, 37, 13, 32, 95, 9, 10, 
	48, 57, 65, 90, 97, 122, 13, 32, 
	34, 95, 9, 10, 48, 57, 65, 90, 
	97, 122, 13, 32, 95, 9, 10, 48, 
	57, 65, 90, 97, 122, 13, 32, 34, 
	95, 9, 10, 48, 57, 65, 90, 97, 
	122, 13, 32, 95, 9, 10, 48, 57, 
	65, 90, 97, 122, 0
};

static const char _macros_single_lengths[] = {
	0, 3, 4, 3, 3, 1, 1, 2, 
	2, 2, 2, 2, 2, 2, 2, 3, 
	4, 3, 4, 3
};

static const char _macros_range_lengths[] = {
	0, 4, 4, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 4, 
	4, 4, 4, 4
};

static const char _macros_index_offsets[] = {
	0, 0, 8, 17, 22, 27, 29, 31, 
	34, 37, 40, 43, 46, 49, 52, 55, 
	63, 72, 80, 89
};

static const char _macros_indicies[] = {
	0, 0, 2, 0, 2, 2, 2, 1, 
	3, 3, 4, 2, 3, 2, 2, 2, 
	1, 5, 5, 6, 5, 1, 6, 6, 
	7, 6, 1, 8, 1, 9, 1, 11, 
	12, 10, 14, 15, 13, 16, 15, 13, 
	17, 15, 13, 19, 1, 18, 21, 22, 
	20, 23, 22, 20, 1, 22, 20, 0, 
	0, 2, 0, 2, 2, 2, 1, 24, 
	24, 25, 26, 24, 26, 26, 26, 1, 
	27, 27, 2, 27, 2, 2, 2, 1, 
	24, 24, 28, 26, 24, 26, 26, 26, 
	1, 24, 24, 26, 24, 26, 26, 26, 
	1, 0
};

static const char _macros_trans_targs[] = {
	1, 0, 2, 3, 4, 3, 4, 5, 
	6, 7, 8, 9, 11, 8, 9, 11, 
	10, 16, 12, 13, 12, 13, 8, 14, 
	17, 18, 2, 17, 19
};

static const char _macros_trans_actions[] = {
	0, 0, 1, 3, 3, 0, 0, 0, 
	0, 0, 5, 5, 5, 0, 0, 0, 
	0, 0, 9, 9, 0, 0, 11, 0, 
	7, 0, 13, 0, 0
};

static const char _macros_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	7, 0, 7, 7
};

static const int macros_start = 15;
static const int macros_first_final = 15;
static const int macros_error = 0;

static const int macros_en_main = 15;


#line 28 "parseMacros.rl"



struct Substitution
{
	size_t begin;
	size_t n;
	std::string macro;
};

}

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

#line 154 "parseMacros.cpp"
	{
	cs = macros_start;
	}

#line 159 "parseMacros.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _macros_trans_keys + _macros_key_offsets[cs];
	_trans = _macros_index_offsets[cs];

	_klen = _macros_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _macros_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _macros_indicies[_trans];
	cs = _macros_trans_targs[_trans];

	if ( _macros_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _macros_actions + _macros_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 61 "parseMacros.rl"
	{
		buf << (*p);
	}
	break;
	case 1:
#line 65 "parseMacros.rl"
	{
		lastKey = buf.str();
		buf.str("");
	}
	break;
	case 2:
#line 70 "parseMacros.rl"
	{
		strStart = p;
	}
	break;
	case 3:
#line 74 "parseMacros.rl"
	{
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
	break;
	case 4:
#line 97 "parseMacros.rl"
	{
		substStart = p;
	}
	break;
	case 5:
#line 101 "parseMacros.rl"
	{
		std::string macro(substStart, p);

		// substStart is the position of the macro name. Decrement by one to
		// get the position of the percentage sign.
		size_t begin = substStart - strStart - 1;

		// Add two, to account for both percentage signs.
		size_t n = p - substStart + 2;

		Substitution subst = {begin, n, macro};
		substitutions.push_back(subst);
	}
	break;
#line 299 "parseMacros.cpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _macros_actions + _macros_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
#line 74 "parseMacros.rl"
	{
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
	break;
#line 340 "parseMacros.cpp"
		}
	}
	}

	_out: {}
	}

#line 128 "parseMacros.rl"


	if (cs < macros_first_final)
	{
		if (p == pe)
			throw std::runtime_error("Unexpected end of file");
		else {
			std::ostringstream err;
			size_t errPos = p - data;
			err << "Error in macro file at position " << errPos << ":" << std::endl << std::endl;

			// Extract the line where the error occured
			std::string str(data);
			size_t ctxBegin = str.find_last_of("\r\n", errPos);
			ctxBegin = ctxBegin == std::string::npos ? 0 : ++ctxBegin;
			size_t ctxEnd = str.find_first_of("\r\n", errPos);

			std::string context = str.substr(ctxBegin, ctxEnd - ctxBegin);
			err << context << std::endl;

			size_t cursor = errPos - ctxBegin;
			for (size_t i = 0; i < cursor; ++i)
				err << " ";
			err << "^";

			throw std::runtime_error(err.str());
		}
	}

	return macros;
}
