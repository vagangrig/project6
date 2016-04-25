/*
 * CSc103 Project 5: Syntax highlighting, part two.
 * See readme.html for details.
 * Please list all references you made use of in order to complete the
 * assignment: your classmates, websites, etc.  Aside from the lecture notes
 * and the book, please list everything.  And remember- citing a source does
 * NOT mean it is okay to COPY THAT SOURCE.  What you submit here **MUST BE
 * YOUR OWN WORK**.
 * References:
 *
 *
 * Finally, please indicate approximately how many hours you spent on this:
 * #hours:
	Haresh:1
	Areesha:1
	Vagan:1
 */

#include "fsm.h"
using namespace cppfsm;
#include <vector>
using std::vector;
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <string>
using std::string;
using std::to_string;
#include <set>
using std::set;
#include <map>
using std::map;
#include <initializer_list> // for setting up maps without constructors.

// enumeration for our highlighting tags:
enum {
	hlstatement,  // used for "if,else,for,while" etc...
	hlcomment,    // for comments
	hlstrlit,     // for string literals
	hlpreproc,    // for preprocessor directives (e.g., #include)
	hltype,       // for datatypes and similar (e.g. int, char, double)
	hlnumeric,    // for numeric literals (e.g. 1234)
	hlescseq,     // for escape sequences
	hlerror,      // for parse errors, like a bad numeric or invalid escape
	hlident       // for other identifiers.  Probably won't use this.
};

// usually global variables are a bad thing, but for simplicity,
// we'll make an exception here.
// initialize our map with the keywords from our list:
map<string, short> hlmap = {
#include "res/keywords.txt"
};
// note: the above is not a very standard use of #include...

// map of highlighting spans:
map<int, string> hlspans = {
	{hlstatement, "<span class='statement'>"},
	{hlcomment, "<span class='comment'>"},
	{hlstrlit, "<span class='strlit'>"},
	{hlpreproc, "<span class='preproc'>"},
	{hltype, "<span class='type'>"},
	{hlnumeric, "<span class='numeric'>"},
	{hlescseq, "<span class='escseq'>"},
	{hlerror, "<span class='error'>"}
};
// note: initializing maps as above requires the -std=c++0x compiler flag,
// as well as #include<initializer_list>.  Very convenient though.
// to save some typing, store a variable for the end of these tags:
string spanend = "</span>";

string translateHTMLReserved(char c) {
	switch (c) {
		case '"':
			return "&quot;";
		case '\'':
			return "&apos;";
		case '&':
			return "&amp;";
		case '<':
			return "&lt;";
		case '>':
			return "&gt;";
		case '\t': // make tabs 4 spaces instead.
			return "&nbsp;&nbsp;&nbsp;&nbsp;";
		default:
			char s[2] = {c,0};
			return s;
	}
}


string firstTranslation(string l);
string secondTranslation(string a, string b);
string findKeyword(string k);
string handleEsc(string e);





int main() {
	// TODO: write the main program.
	// It may be helpful to break this down and write
	// a function that processes a single line, which
	// you repeatedly call from main().
  string line;
	vector<string>lines;
	while(getline(cin,line))
	{
		if(line != "EOF")
			lines.push_back(line);
		else
			break;
	}
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		cout << secondTranslation(lines[i], firstTranslation(lines[i])) << endl;
	}
	return 0;
}





string firstTranslation(string l)
{
	int s = 0;
	char c;
	string oldTranslation;
	for(unsigned int i = 0; i < l.length(); i++)
	{
		c = l[i];
		updateState(s,c);
		oldTranslation += to_string(s);
	}
	return oldTranslation;
}

string secondTranslation(string a, string b)
{
	// a is the original string
	// b is the state translation
	if(a.length() != b.length())
		cout << "Fatal error has occured.\n";
	string newTranslation;
  for(unsigned int i = 0; i < a.length(); i++)
	{
		if(b[i] == '1') // scanid
		{
			string currentKey;
			int endOfKey = i;
			int len = 0;
			while(b[endOfKey] == '1')
			{
				endOfKey++;
				len++;
			}
			currentKey = a.substr(i, len);
			if(hlmap.count(currentKey) > 0)
				newTranslation += findKeyword(currentKey);
			else
				newTranslation += handleEsc(currentKey);
			i = endOfKey - 1;
		}

    else if(b[i] == '0') // start
		{
			if(a[i] == '\"')
			{
				newTranslation += hlspans[hlstrlit] + translateHTMLReserved(a[i]) + spanend;
			}
			else
			{
				newTranslation += translateHTMLReserved(a[i]);
			}
		}

		else if(b[i] == '3') // strlit
		{
			int endOfStr = i + 1;
			int id = 1;
			while(b[endOfStr] == '3')
			{
				endOfStr++;
				if(b[endOfStr] == '5')
				{
					if(b[endOfStr + 1] == '7')
						id = 3;
					else
						id = 2;
					break;
				}
			}
			switch(id)
			{
				case 1:
					newTranslation += hlspans[hlstrlit] + handleEsc(a.substr(i, endOfStr - i + 1)) + spanend;
					i = endOfStr;
					break;
				case 2:
					newTranslation += hlspans[hlstrlit] + handleEsc(a.substr(i, endOfStr - i)) + spanend + hlspans[hlescseq] + a.substr(endOfStr, 2) + spanend;
					i = endOfStr + 1;
					break;
				case 3:
					newTranslation += hlspans[hlstrlit] + handleEsc(a.substr(i, endOfStr - i)) + spanend + hlspans[hlerror] + a.substr(endOfStr) + spanend;
					i = a.length();
					break;
			}
		}

    else if(b[i] == '4') // readfs
		{
			if(a[i] == '/' && a[i+1] == '/') // comment
			{
				newTranslation += hlspans[hlcomment] + handleEsc(a.substr(i)) + spanend;
				i = a.length();
			}
      else
        newTranslation += translateHTMLReserved(a[i]);
		}

    else if(b[i] == '5') // escseq
		{
			if(b[i + 1] == '7')
			{
				newTranslation += hlspans[hlerror] + handleEsc(to_string(b[i])) + spanend;
			}
			else
			{
				newTranslation += hlspans[hlescseq] + handleEsc(to_string(b[i])) + spanend;
			}
		}

    else if(b[i] == '6') // scannum
		{
			int endOfNum = i;
			while(INSET(a[endOfNum],num))
				endOfNum++;
			newTranslation += hlspans[hlnumeric] + handleEsc(a.substr(i, endOfNum - i)) + spanend;
			i = endOfNum - 1;
		}

    else if(b[i] == '7') // error
		{
			if(b[i - 1] == '5')
			{
				int pos = newTranslation.length() - 1;
				newTranslation += hlspans[hlerror] + handleEsc(a.substr(i - 1)) + spanend;
				newTranslation.erase(pos, 0);
			}
			else
				newTranslation += hlspans[hlerror] + handleEsc(a.substr(i)) + spanend;
			i = a.length();
		}
  }
	return newTranslation;
}

string findKeyword(string k)
{
	if(hlmap.count(k) > 0)
	{
		return hlspans[hlmap[k]] + k + spanend;
	}
	return k;
}

string handleEsc(string e)
{
	string f;
	for(unsigned int i = 0; i < e.length(); i++)
	{
		if(translateHTMLReserved(e[i]) == to_string(e[i]))
		{
			f += e[i];
		}
		else
		{
			f += translateHTMLReserved(e[i]);
		}
	}
	return f;
}