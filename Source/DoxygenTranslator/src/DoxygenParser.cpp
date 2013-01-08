/* ----------------------------------------------------------------------------- 
 * This file is part of SWIG, which is licensed as a whole under version 3 
 * (or any later version) of the GNU General Public License. Some additional
 * terms also apply to certain portions of SWIG. The full details of the SWIG
 * license and copyrights can be found in the LICENSE and COPYRIGHT files
 * included with the SWIG source code as distributed by the SWIG developers
 * and at http://www.swig.org/legal.html.
 *
 * DoxygenParser.cpp
 * ----------------------------------------------------------------------------- */

#include "DoxygenParser.h"
#include "DoxygenCommands.h"
#include "swig.h"
#include "swigwarn.h"

#include <iostream>
#include <algorithm>
#include <vector>

using std::string;
using std::cout;
using std::endl;

// Define static class members
DoxygenParser::DoxyCommandsMap DoxygenParser::doxygenCommands;
std::set<std::string> DoxygenParser::doxygenSectionIndicators;

const int TOKENSPERLINE = 8;    //change this to change the printing behaviour of the token list


DoxygenParser::DoxygenParser(bool noisy) : noisy(noisy)
{
	fillTables();
}

DoxygenParser::~DoxygenParser() {
}

void DoxygenParser::fillTables() {
    // run it only once
    if (doxygenCommands.size())
        return;

    // fill in tables with data from DxygenCommands.h
    for (int i = 0; i < simpleCommandsSize; i++)
        doxygenCommands[simpleCommands[i]] = SIMPLECOMMAND;

    for (int i = 0; i < commandWordsSize; i++)
        doxygenCommands[commandWords[i]] = COMMANDWORD;

    for (int i = 0; i < commandLinesSize; i++)
        doxygenCommands[commandLines[i]] = COMMANDLINE;

    for (int i = 0; i < commandParagraphSize; i++)
        doxygenCommands[commandParagraph[i]] = COMMANDPARAGRAPH;

    for (int i = 0; i < commandEndCommandsSize; i++)
        doxygenCommands[commandEndCommands[i]] = COMMANDENDCOMMAND;

    for (int i = 0; i < commandWordParagraphsSize; i++)
        doxygenCommands[commandWordParagraphs[i]] = COMMANDWORDPARAGRAPH;

    for (int i = 0; i < commandWordLinesSize; i++)
        doxygenCommands[commandWordLines[i]] = COMMANDWORDLINE;

    for (int i = 0; i < commandWordOWordOWordsSize; i++)
        doxygenCommands[commandWordOWordOWords[i]] = COMMANDWORDOWORDWORD;

    for (int i = 0; i < commandOWordsSize; i++)
        doxygenCommands[commandOWords[i]] = COMMANDOWORD;

    for (int i = 0; i < commandErrorThrowingsSize; i++)
        doxygenCommands[commandErrorThrowings[i]] = COMMANDERRORTHROW;

    for (int i = 0; i < commandUniquesSize; i++)
        doxygenCommands[commandUniques[i]] = COMMANDUNIQUE;

    for (int i = 0; i < htmlCommandsSize; i++)
        doxygenCommands[htmlCommands[i]] = COMMANDUNIQUE;

    for (int i = 0; i < commandUniquesSize; i++)
        doxygenCommands[commandUniques[i]] = COMMANDUNIQUE;

    // fill section indicators command set
    for (int i = 0; i < sectionIndicatorsSize; i++)
        doxygenSectionIndicators.insert(sectionIndicators[i]);
}


std::string DoxygenParser::stringToLower(const std::string &stringToConvert) {

	string result(stringToConvert.size(), ' ');

	for (size_t i = 0; i < result.size(); i++) {
	    result[i] = tolower(stringToConvert[i]);
	}

	return result;
}


bool DoxygenParser::findCommand(const std::string &smallString) {
    // I'm not sure if we can really do so, because there are different commands
    // in doxygenCommands and original commandArray

    std::map<std::string, DoxyCommandEnum>::iterator it =
                    doxygenCommands.find(stringToLower(smallString));

	return it != doxygenCommands.end(); // returns true if command was found
}


bool DoxygenParser::isSectionIndicator(const std::string &smallString) {

    std::set<std::string>::iterator it =
                    doxygenSectionIndicators.find(stringToLower(smallString));

	return it != doxygenSectionIndicators.end();
}


void DoxygenParser::printTree(const DoxygenEntityList &rootList) {
  DoxygenEntityList::const_iterator p = rootList.begin();
  while (p != rootList.end()) {
    (*p).printEntity(0);
    p++;
  }
}


int DoxygenParser::commandBelongs(const std::string &theCommand) {

  std::string smallString = stringToLower(theCommand);
  //cout << " Looking for command " << theCommand << endl;
  DoxyCommandsMapIt it = doxygenCommands.find(smallString);

  if (it != doxygenCommands.end()) {
    return it->second;
  }
  return 0;
}


std::string DoxygenParser::trim(const std::string &text)
{
    size_t start = text.find_first_not_of(" \t");
    size_t end = text.find_last_not_of(" \t");

    if (start == string::npos  ||  start > end) {
        return "";
    }
    return text.substr(start, end - start + 1);
}


bool DoxygenParser::isEndOfLine()
{
    if (m_tokenListIt == m_tokenList.end()) {
        return false;
    }
    Token nextToken = *m_tokenListIt;
    return nextToken.m_tokenType == END_LINE;
}


void DoxygenParser::skipWhitespaceTokens()
{
    if (m_tokenListIt == m_tokenList.end()) {
        return;
    }

    while (m_tokenListIt != m_tokenList.end()  &&
            (m_tokenListIt->m_tokenType == END_LINE  ||  trim(m_tokenListIt->m_tokenString).empty())) {

        m_tokenListIt++;
    }
}


std::string DoxygenParser::getNextWord() {

/*    if (m_tokenListIt == m_tokenList.end()) {
        return "";
    }
*/
	while (m_tokenListIt != m_tokenList.end()  &&  (m_tokenListIt->m_tokenType == PLAINSTRING)) {
		// handle quoted strings as words
		if (m_tokenListIt->m_tokenString[0] == '"'
				&& m_tokenListIt->m_tokenString[m_tokenListIt->m_tokenString.size() - 1] != '"') {

			string word = m_tokenListIt->m_tokenString + " ";
			m_tokenListIt++;
			while (true) {
				string nextWord = getNextWord();
				if (nextWord.empty()) { // maybe report unterminated string error
					return word;
				}
				word += nextWord;
				if (word[word.size() - 1] == '"') { // strip quotes
					return word.substr(1, word.size() - 2);
				}
				word += " ";
			}
		}

        string tokenStr = trim(m_tokenListIt->m_tokenString);
		m_tokenListIt++;
		if (!tokenStr.empty()) {
		    return tokenStr;
		}
	} /* else if (nextToken.m_tokenType == END_LINE) {
	    // this handles cases when command is the last item in line, for example:
	    // * This method returns line number \c
	    // * relative to paragraph.
        m_tokenListIt++;
        return getNextWord();
	} */

	return "";
}

/* TODO remove this m.
std::string DoxygenParser::getNextWordInComment() {

    while (m_tokenListIt != m_tokenList.end()  &&  (m_tokenListIt->m_tokenType == PLAINSTRING  ||  m_tokenListIt->m_tokenType == END_LINE)) {
        // handle quoted strings as words
        if (m_tokenListIt->m_tokenString[0] == '"'
                && m_tokenListIt->m_tokenString[m_tokenListIt->m_tokenString.size() - 1] != '"') {

            string word = m_tokenListIt->m_tokenString + " ";
            while (true) {
                string nextWord = getNextWord();
                if (nextWord.empty()) {// maybe report unterminated string error
                    return word;
                }
                word += nextWord;
                if (word[word.size() - 1] == '"') { // strip quotes
                    return word.substr(1, word.size() - 2);
                }
                word += " ";
            }
        }

        string tokenStr = trim(m_tokenListIt->m_tokenString);
        m_tokenListIt++;
        if (!tokenStr.empty()) {
            return tokenStr;
        }
    } * else if (nextToken.m_tokenType == END_LINE) {
        // this handles cases when command is the last item in line, for example:
        // * This method returns line number \c
        // * relative to paragraph.
        m_tokenListIt++;
        return getNextWord();
    } *

    return "";
} */


DoxygenParser::TokenListCIt DoxygenParser::getOneLine(const TokenList &tokList) {

	TokenListCIt endOfLineIt = m_tokenListIt;

	while (endOfLineIt != tokList.end()) {
		if (endOfLineIt->m_tokenType == END_LINE) {
			return endOfLineIt;
		}
		endOfLineIt++;
	}

	return tokList.end();
}


std::string DoxygenParser::getStringTilCommand(const TokenList & tokList) {

	if (m_tokenListIt == tokList.end()) {
		return "";
	}

    string description;

    while (m_tokenListIt->m_tokenType == PLAINSTRING) {
		const Token &currentToken = *m_tokenListIt++;
		if (currentToken.m_tokenType == PLAINSTRING) {
			description = description + currentToken.m_tokenString; // + " ";
		}
	}
	return description;
}


std::string DoxygenParser::getStringTilEndCommand(const std::string & theCommand, const TokenList & tokList) {

    if (m_tokenListIt == tokList.end()) {
        return "";
    }

	string description;
	while (m_tokenListIt != tokList.end()) {

		//TODO: it won't output doxygen commands, need a way to fix it
		if (m_tokenListIt->m_tokenType == PLAINSTRING) {
			description += m_tokenListIt->m_tokenString;
		} else if (m_tokenListIt->m_tokenType == END_LINE) {
			description += "\n";
	    } else if (m_tokenListIt->m_tokenString == theCommand) {
		    m_tokenListIt++;
			return description;
		}

		m_tokenListIt++;
	}

	printListError(WARN_DOXYGEN_COMMAND_EXPECTED, theCommand + " command expected");

	return description;
}


DoxygenParser::TokenListCIt DoxygenParser::getEndOfParagraph(const TokenList & tokList) {

    TokenListCIt endOfParagraph = m_tokenListIt;

    while (endOfParagraph != tokList.end()) {
        if (endOfParagraph->m_tokenType == END_LINE) {
            endOfParagraph++;
            if (endOfParagraph != tokList.end() && endOfParagraph->m_tokenType == END_LINE) {
                endOfParagraph++;
                //cout << "ENCOUNTERED END OF PARA" << endl;
                return endOfParagraph;
            }

        } else if (endOfParagraph->m_tokenType == COMMAND) {

            if (isSectionIndicator(endOfParagraph->m_tokenString)) {
                return endOfParagraph;
            } else {
                endOfParagraph++;
            }

        } else if (endOfParagraph->m_tokenType == PLAINSTRING) {
            endOfParagraph++;
        } else {
            return tokList.end();
        }
    }

    return tokList.end();
}


DoxygenParser::TokenListCIt DoxygenParser::getEndOfSection(const std::string & theCommand,
                                                                 const TokenList & tokList) {

    TokenListCIt endOfParagraph = m_tokenListIt;

	while (endOfParagraph != tokList.end()) {
		if (endOfParagraph->m_tokenType == COMMAND) {
			if (theCommand == endOfParagraph->m_tokenString)
				return endOfParagraph;
			else
				endOfParagraph++;
		} else if (endOfParagraph->m_tokenType == PLAINSTRING) {
			endOfParagraph++;
		} else if (endOfParagraph->m_tokenType == END_LINE) {
			endOfParagraph++;
			if (endOfParagraph->m_tokenType == END_LINE) {
				endOfParagraph++;
				return endOfParagraph;
			}
		}
	}
	return tokList.end();
}


DoxygenParser::TokenListCIt DoxygenParser::getEndCommand(const std::string & theCommand,
                                                              const TokenList & tokList) {

    TokenListCIt endOfCommand = m_tokenListIt;

	while (endOfCommand != tokList.end()) {
        endOfCommand++;
		if ((*endOfCommand).m_tokenType == COMMAND) {
			if (theCommand == (*endOfCommand).m_tokenString) {
				return endOfCommand;
			}
		}
	}
	//End command not found
	return tokList.end();
}


void DoxygenParser::skipEndOfLine()
{
    if (m_tokenListIt != m_tokenList.end()  &&
                    m_tokenListIt->m_tokenType == END_LINE) {
        m_tokenListIt++;
    }
}


int DoxygenParser::addSimpleCommand(const std::string &theCommand,
                                         DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;

	doxyList.push_back(DoxygenEntity(theCommand));
	return 1;
}


int DoxygenParser::addCommandWord(const std::string &theCommand,
                                       const TokenList &,
                                       DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;

	if (isEndOfLine()) {
	    // handles cases when command is at the end of line (for example "\c\nreally"
	    skipWhitespaceTokens();
	    doxyList.push_back(DoxygenEntity("plainstd::endl"));
	}
	std::string name = getNextWord();
    if (!name.empty()) {
      DoxygenEntityList aNewList;
      aNewList.push_back(DoxygenEntity("plainstd::string", name));
      doxyList.push_back(DoxygenEntity(theCommand, aNewList));
      return 1;
	} else {
		printListError(WARN_DOXYGEN_COMMAND_ERROR, "No word followed " +
		                theCommand + " command. Not added");
	}
	return 0;
}


int DoxygenParser::addCommandLine(const std::string &theCommand,
                                       const TokenList &tokList,
                                       DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;
	TokenListCIt endOfLine = getOneLine(tokList);
	DoxygenEntityList aNewList = parse(endOfLine, tokList);
	doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	skipEndOfLine();
	return 1;
}


int DoxygenParser::addCommandParagraph(const std::string &theCommand,
                                            const TokenList &tokList,
                                            DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;

	TokenListCIt endOfParagraph = getEndOfParagraph(tokList);
	DoxygenEntityList aNewList;
	aNewList = parse(endOfParagraph, tokList);
	doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	return 1;
}


int DoxygenParser::addCommandEndCommand(const std::string &theCommand,
                                             const TokenList & tokList,
                                             DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;
	TokenListCIt endCommand = getEndCommand("end" + theCommand, tokList);
	if (endCommand == tokList.end()) {
	  printListError(WARN_DOXYGEN_COMMAND_EXPECTED, "Expected end" + theCommand);
	  return 0;
	}
	DoxygenEntityList aNewList;
	aNewList = parse(endCommand, tokList);
	m_tokenListIt++;
	doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	return 1;
}


int DoxygenParser::addCommandWordParagraph(const std::string &theCommand,
                                           const TokenList &tokList,
                                           DoxygenEntityList &doxyList) {
    if (noisy)
        cout << "Parsing " << theCommand << endl;

    std::string name = getNextWord();

    if (name.empty()) {
        printListError(WARN_DOXYGEN_COMMAND_ERROR, "No word followed " + theCommand + " command. Not added");
        return 0;
    }
    TokenListCIt endOfParagraph = getEndOfParagraph(tokList);
    DoxygenEntityList aNewList;
    aNewList = parse(endOfParagraph, tokList);
    aNewList.push_front(DoxygenEntity("plainstd::string", name));
    doxyList.push_back(DoxygenEntity(theCommand, aNewList));
    return 1;
}


int DoxygenParser::addCommandWordLine(const std::string &theCommand,
                                           const TokenList & tokList,
                                           DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;
	std::string name = getNextWord();
	if (name.empty()) {
		printListError(WARN_DOXYGEN_COMMAND_ERROR, "No word followed " + theCommand + " command. Not added");
		return 0;
	}

	TokenListCIt endOfLine = getOneLine(tokList);
	DoxygenEntityList aNewList;
	aNewList = parse(endOfLine, tokList);
	aNewList.push_front(DoxygenEntity("plainstd::string", name));
	doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	return 1;
	//else cout << "No line followed " << theCommand <<  " command. Not added" << endl;
}


int DoxygenParser::addCommandWordOWordOWord(const std::string &theCommand,
                                                  const TokenList &,
                                                  DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;

	std::string name = getNextWord();
	if (name.empty()) {
		printListError(WARN_DOXYGEN_COMMAND_ERROR, "No word followed " + theCommand + " command. Not added");
		return 0;
	}
	std::string headerfile = getNextWord();
	std::string headername = getNextWord();
	DoxygenEntityList aNewList;
	aNewList.push_back(DoxygenEntity("plainstd::string", name));
	if (!headerfile.empty())
		aNewList.push_back(DoxygenEntity("plainstd::string", headerfile));
	if (!headername.empty())
		aNewList.push_back(DoxygenEntity("plainstd::string", headername));
	doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	return 1;
}


int DoxygenParser::addCommandOWord(const std::string &theCommand,
                                        const TokenList &,
                                        DoxygenEntityList &doxyList) {
	if (noisy)
		cout << "Parsing " << theCommand << endl;

	std::string name = getNextWord();
	DoxygenEntityList aNewList;
	aNewList.push_back(DoxygenEntity("plainstd::string", name));
	doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	return 1;
}


int DoxygenParser::addCommandErrorThrow(const std::string &theCommand,
                                             const TokenList &tokList,
                                             DoxygenEntityList &) {

	printListError(WARN_DOXYGEN_COMMAND_ERROR, "Encountered: " + theCommand +
			"\nThis command should not have been encountered. Behaviour past this may be unpredictable");
	m_tokenListIt = getOneLine(tokList);
	return 0;
}


int DoxygenParser::addCommandUnique(const std::string &theCommand,
                                         const TokenList &tokList,
                                         DoxygenEntityList &doxyList) {

    static std::map<std::string, std::string> endCommands;
	DoxygenEntityList aNewList;
	if (theCommand == "arg" || theCommand == "li") {
		TokenListCIt endOfSection = getEndOfSection(theCommand, tokList);
		DoxygenEntityList aNewList;
		aNewList = parse(endOfSection, tokList);
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
	// \xrefitem <key> "(heading)" "(std::list title)" {text}
	else if (theCommand == "xrefitem") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string key = getNextWord();
		if (key.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No key followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string heading = getNextWord();
		if (key.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No heading followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string title = getNextWord();
		if (title.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No title followed " + theCommand + " command. Not added");
			return 0;
		}
		TokenListCIt endOfParagraph = getEndOfParagraph(tokList);
		aNewList = parse(endOfParagraph, tokList);
		aNewList.push_front(DoxygenEntity("plainstd::string", title));
		aNewList.push_front(DoxygenEntity("plainstd::string", heading));
		aNewList.push_front(DoxygenEntity("plainstd::string", key));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
		return 1;
	}
	// \ingroup (<groupname> [<groupname> <groupname>])
	else if (theCommand == "ingroup") {
		std::string name = getNextWord();
		aNewList.push_back(DoxygenEntity("plainstd::string", name));
		name = getNextWord();
		if (!name.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", name));
		name = getNextWord();
		if (!name.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", name));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
		return 1;
	}
	// \par [(paragraph title)] { paragraph }
	else if (theCommand == "par") {
	    TokenListCIt endOfLine = getOneLine(tokList);
		aNewList = parse(endOfLine, tokList);
		DoxygenEntityList aNewList2;
		TokenListCIt endOfParagraph = getEndOfParagraph(tokList);
		aNewList2 = parse(endOfParagraph, tokList);
		aNewList.splice(aNewList.end(), aNewList2);
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
		return 1;
	}
	// \headerfile <header-file> [<header-name>]
	else if (theCommand == "headerfile") {
		DoxygenEntityList aNewList;
		std::string name = getNextWord();
		aNewList.push_back(DoxygenEntity("plainstd::string", name));
		name = getNextWord();
		if (!name.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", name));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
		return 1;
	}
	// \overload [(function declaration)]
	else if (theCommand == "overload") {
	    TokenListCIt endOfLine = getOneLine(tokList);
		if (endOfLine != m_tokenListIt) {
			DoxygenEntityList aNewList;
			aNewList = parse(endOfLine, tokList);
			doxyList.push_back(DoxygenEntity(theCommand, aNewList));
		} else
			doxyList.push_back(DoxygenEntity(theCommand));
		return 1;
	}
	// \weakgroup <name> [(title)]
	else if (theCommand == "weakgroup") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string name = getNextWord();
		if (name.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No word followed " + theCommand + " command. Not added");
			return 0;
		}
		DoxygenEntityList aNewList;
		TokenListCIt endOfLine = getOneLine(tokList);
		if (endOfLine != m_tokenListIt) {
			aNewList = parse(endOfLine, tokList);
		}
		aNewList.push_front(DoxygenEntity("plainstd::string", name));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
	// \ref <name> ["(text)"]
	else if (theCommand == "ref") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string name = getNextWord();
		if (name.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No key followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string text = getNextWord();
		aNewList.push_back(DoxygenEntity("plainstd::string", name));
		if (!text.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", text));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
	// \subpage <name> ["(text)"]
	else if (theCommand == "subpage") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string name = getNextWord();
		if (name.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No name followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string text = getNextWord();
		aNewList.push_back(DoxygenEntity("plainstd::string", name));
		if (!text.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", text));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
    // \code ... \endcode
    // \verbatim ... \endverbatim
	// \dot dotcode \enddot
	// \msc msccode \endmsc
	// \f[ ... \f]
	// \f{ ... \f}
	// \f{env}{ ... \f}
	// \f$ ... \f$
    else if (theCommand == "code" || theCommand == "verbatim"
        || theCommand == "dot" || theCommand == "msc"
        || theCommand == "f[" || theCommand == "f{"
        || theCommand == "f$") {
        if (!endCommands.size()) {
          // fill in static table of end commands
          endCommands["f["] = "f]";
          endCommands["f{"] = "f}";
          endCommands["f$"] = "f$";
        }
        if (noisy)
            cout << "Parsing " << theCommand << endl;

        std::string endCommand;
        std::map <std::string, std::string>::iterator it;
        it = endCommands.find(theCommand);
        if (it != endCommands.end())
          endCommand = it->second;
        else
          endCommand = "end" + theCommand;

        std::string content = getStringTilEndCommand(endCommand, tokList);
        aNewList.push_back(DoxygenEntity("plainstd::string", content));
        doxyList.push_back(DoxygenEntity(theCommand, aNewList));
    }
	// \dotfile <file> ["caption"]
	// \mscfile <file> ["caption"]
	else if (theCommand == "dotfile" || theCommand == "mscfile") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string file = getNextWord();
		if (file.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No file followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string caption = getNextWord();
		aNewList.push_back(DoxygenEntity("plainstd::string", file));
		if (!caption.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", caption));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
	// \image <format> <file> ["caption"] [<sizeindication>=<size>]
	else if (theCommand == "image") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string format = getNextWord();
		if (format.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No format followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string file = getNextWord();
		if (file.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "No name followed " + theCommand + " command. Not added");
			return 0;
		}
		std::string caption = getNextWord();
		std::string size = getNextWord();

		DoxygenEntityList aNewList;
		aNewList.push_back(DoxygenEntity("plainstd::string", format));
		aNewList.push_back(DoxygenEntity("plainstd::string", file));
		if (!caption.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", caption));
		if (!size.empty())
			aNewList.push_back(DoxygenEntity("plainstd::string", size));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
	// \addtogroup <name> [(title)]
	else if (theCommand == "addtogroup") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;
		std::string name = getNextWord();
		if (name.empty()) {
			printListError(WARN_DOXYGEN_COMMAND_ERROR, "There should be at least one word following the '" + theCommand + "' command. Command ignored.");
			return 0;
		}
		DoxygenEntityList aNewList;
		TokenListCIt endOfLine = getOneLine(tokList);
		if (endOfLine != m_tokenListIt) {
			aNewList = parse(endOfLine, tokList);
		}
		aNewList.push_front(DoxygenEntity("plainstd::string", name));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
		skipEndOfLine();
	}
	// \if <cond> [\else ...] [\elseif <cond> ...] \endif
	else if (theCommand == "if" || theCommand == "ifnot" ||
					 theCommand == "else" || theCommand == "elseif") {
		if (noisy)
			cout << "Parsing " << theCommand << endl;

		std::string cond;
		bool skipEndif = false; // if true then we skip endif after parsing block of code
		bool needsCond = (theCommand == "if" || theCommand == "ifnot" || theCommand == "elseif");
		if (needsCond) {
			cond = getNextWord();
			if (cond.empty()) {
				printListError(WARN_DOXYGEN_COMMAND_ERROR, "No word followed " + theCommand + " command. Not added");
				return 0;
			}
		}

		int nestedCounter = 1;
		TokenListCIt endCommand = tokList.end();

		// go through the commands and find closing endif or else or elseif
		for (TokenListCIt it = m_tokenListIt; it != tokList.end(); it++) {
			if (it->m_tokenType == COMMAND) {
				if (it->m_tokenString == "if" || it->m_tokenString == "ifnot")
					nestedCounter++;
				else if (it->m_tokenString == "endif")
					nestedCounter--;
				if (nestedCounter == 1 && (it->m_tokenString == "else" || it->m_tokenString == "elseif")) { // else found
					endCommand = it;
					break;
				}
				if (nestedCounter == 0) { // endif found
					endCommand = it;
					skipEndif = true;
					break;
				}
			}
		}

		if (endCommand == tokList.end()) {
			printListError(WARN_DOXYGEN_COMMAND_EXPECTED, "No corresponding endif found");
			return 0;
		}

		DoxygenEntityList aNewList;
		aNewList = parse(endCommand, tokList);
		if (skipEndif)
			m_tokenListIt++;
		if (needsCond)
			aNewList.push_front(DoxygenEntity("plainstd::string", cond));
		doxyList.push_back(DoxygenEntity(theCommand, aNewList));
	}
	return 0;
}


int DoxygenParser::addCommand(const std::string &commandString,
                                  const TokenList &tokList,
                                  DoxygenEntityList &doxyList) {

	string theCommand = stringToLower(commandString);

	if (theCommand == "plainstd::string") {
		string nextPhrase = getStringTilCommand(tokList);
		if (noisy)
			cout << "Parsing plain std::string :" << nextPhrase << endl;
		doxyList.push_back(DoxygenEntity("plainstd::string", nextPhrase));
		return 1;
	}

	switch (commandBelongs(theCommand)) {
		case SIMPLECOMMAND:
			return addSimpleCommand(theCommand, doxyList);
		case COMMANDWORD:
			return addCommandWord(theCommand, tokList, doxyList);
		case COMMANDLINE:
			return addCommandLine(theCommand, tokList, doxyList);
		case COMMANDPARAGRAPH:
			return addCommandParagraph(theCommand, tokList, doxyList);
		case COMMANDENDCOMMAND:
			return addCommandEndCommand(theCommand, tokList, doxyList);
		case COMMANDWORDPARAGRAPH:
			return addCommandWordParagraph(theCommand, tokList, doxyList);
		case COMMANDWORDLINE:
			return addCommandWordLine(theCommand, tokList, doxyList);
		case COMMANDWORDOWORDWORD:
			return addCommandWordOWordOWord(theCommand, tokList, doxyList);
		case COMMANDOWORD:
			return addCommandOWord(theCommand, tokList, doxyList);
		case COMMANDERRORTHROW:
			return addCommandErrorThrow(theCommand, tokList, doxyList);
		case COMMANDUNIQUE:
			return addCommandUnique(theCommand, tokList, doxyList);
	}
	return 0;
}


/**
 * This method converts TokenList to DoxygenEntryList.
 */
DoxygenEntityList DoxygenParser::parse(TokenListCIt endParsingIndex,
                                          const TokenList &tokList,
                                          bool root) {
	// if we are root, than any strings should be added as 'partofdescription', else as 'plainstd::string'
	std::string currPlainstringCommandType = root ? "partofdescription" : "plainstd::string";
	DoxygenEntityList aNewList;

	while (m_tokenListIt != endParsingIndex) {

		Token currToken = *m_tokenListIt;

		if (noisy)
			cout << "Parsing for phrase starting in:" << currToken.toString() << endl;

		if (currToken.m_tokenType == END_LINE) {
		    aNewList.push_back(DoxygenEntity("plainstd::endl"));
		    m_tokenListIt++;
		} else if (currToken.m_tokenType == COMMAND) {
		    m_tokenListIt++;
		    addCommand(currToken.m_tokenString, tokList, aNewList);
		} else if (currToken.m_tokenType == PLAINSTRING) {
			addCommand(currPlainstringCommandType, tokList, aNewList);
		}

		if (endParsingIndex != tokList.end() && m_tokenListIt == tokList.end()) {
		  // this could happen if we can't reach the original endParsingIndex
		  printListError(WARN_DOXYGEN_UNEXPECTED_END_OF_COMMENT, "Unexpected end of comment encountered");
		  break;
		}
	}
	return aNewList;
}


DoxygenEntityList DoxygenParser::createTree(const std::string &doxygenBlob,
                                               const std::string &fileName,
                                               int lineNumber) {

  tokenizeDoxygenComment(doxygenBlob, fileName, lineNumber);
  if (noisy) {
    cout << "---TOKEN LIST---" << endl;
    printList();
  }

  DoxygenEntityList rootList = parse(m_tokenList.end(), m_tokenList, true);

  if (noisy) {
    cout << "PARSED LIST" << endl;
    printTree(rootList);
  }
  return rootList;
}


/**
 * This is one of the most important methods - it breaks the original
 * doxygen comment into tokens - one token per word.
 * See replacement, which also handles html comments below.
DoxygenParser::TokenList DoxygenParser::tokenizeDoxygenComment(const std::string &doxygenComment,
                                                                      const std::string &fileName,
                                                                      int fileLine) {
  TokenList tokList;
  m_fileLineNo = fileLine;
  m_fileName = fileName;

  bool isPlainString = false;
  string::size_type pos, lastPos = 0;
  char prevChar = doxygenComment[lastPos];
  string currentWord;
  while (true) {
    isPlainString = false;
    pos = doxygenComment.find_first_of("\\@\t\n ", lastPos);
    if (pos == string::npos) {
      pos = doxygenComment.size();
    // } else {
      // preserve whitespaces
    //  while (pos != string::npos  &&  (doxygenComment[pos] == ' ' ||  doxygenComment[pos] == '\t')) {
    //    pos++;
    //  }
    //  if (pos == string::npos) {
    //    pos = doxygenComment.size();
    //  }
    }

    currentWord = doxygenComment.substr(lastPos, pos-lastPos);

    if (prevChar == '\n') {

      tokList.push_back(Token(END_LINE, "\n"));

    } else if (prevChar == '\\' || prevChar == '@') {
      // it's a doxygen command
      // hack to get commands like \\ or \@ or @\ or @@
      if (doxygenComment[pos] == '@' || doxygenComment[pos] == '\\') {
        currentWord += doxygenComment[pos];
        pos++;
      }
      // also strip the command till the first non-alpha char
      for (size_t i = 2; i < currentWord.size(); i++) {
        if (!isalpha(currentWord[i])) {
          currentWord = currentWord.substr(0, i);
          // set current parsing pos back, to parse the rest of the command
          pos = lastPos + i - 1;
          break;
        }
      }

      if (findCommand(currentWord)) {
          tokList.push_back(Token(COMMAND, currentWord));
      } else {
          // unknown commands are not translated - treated as literal string
          tokList.push_back(Token(PLAINSTRING, currentWord));
      }

    } else if (currentWord.size() && (currentWord[0] == '!' || currentWord[0] == '*' || currentWord[0] == '/')) {

      // check if it's one of the '!!!', '***', '///' of any length
      char c = currentWord[0];
      isPlainString = false;
      for (size_t i = 0; i < currentWord.size(); i++)
        if (currentWord[i] != c) {
          isPlainString = true;
          break;
        }
    } else {
      isPlainString = true;
    }

    if (isPlainString && currentWord.size()) {
      tokList.push_back(Token(PLAINSTRING, currentWord));
    }

    prevChar = doxygenComment[pos];
    lastPos = pos + 1;
    if (lastPos >= doxygenComment.size())
      break;
  }

  m_tokenListIt = tokList.begin();

  return tokList;
}
 */


// Splits 'text' on 'separator' chars. Separator chars are not part of the strings.
DoxygenParser::StringVector DoxygenParser::split(const std::string &text, char separator)
{
    StringVector lines;
    size_t prevPos = 0, pos = 0;

    while (pos < string::npos) {
        pos = text.find(separator, prevPos);
        lines.push_back(text.substr(prevPos, pos - prevPos));
        prevPos = pos + 1;
    }

    return lines;
}


bool DoxygenParser::isStartOfDoxyCommentChar(char c)
{
    return (strchr("*/!", c) != NULL);
}


void DoxygenParser::addDoxyCommand(DoxygenParser::TokenList &tokList,
                                        const std::string &cmd) {
    if (findCommand(cmd)) {
        tokList.push_back(Token(COMMAND, cmd));
    } else {
        // Unknown commands are ignored, because they are
        // also ignored by Doxygen - see test doxygen_misc_constructs.h, f. backslashB().
        // This differs from original implementation in this class. Uncomment
        // the line below to put unknown commands to output.
        // tokList.push_back(Token(PLAINSTRING, cmd));
    }
}


size_t DoxygenParser::processVerbatimText(size_t pos, const std::string &line)
{
    if (line[pos] == '\\'  ||  line[pos] == '@') {
        pos++;
        // characters '$[]{}' are used in commands \f$, \f[, ...
        size_t endOfWordPos = line.find_first_not_of("abcdefghijklmnopqrstuvwxyz$[]{}", pos);
        string cmd = line.substr(pos , endOfWordPos - pos);

        if (cmd == CMD_END_HTML_ONLY  ||  cmd == CMD_END_VERBATIM) {
            m_isVerbatimText = false;
            addDoxyCommand(m_tokenList, cmd);
        } else {
            cmd = line[pos] + cmd; // prepend '\\' or '@'
            m_tokenList.push_back(Token(PLAINSTRING,
                                  line.substr(pos, endOfWordPos - pos)));
        }
        pos = endOfWordPos;
    } else {
        // whitespaces are stored as plain strings
        size_t startOfPossibleEndCmd = line.find_first_of("\\@", pos);
        m_tokenList.push_back(Token(PLAINSTRING,
                              line.substr(pos, startOfPossibleEndCmd - pos)));
        pos = startOfPossibleEndCmd;
    }

    return pos;
}


size_t DoxygenParser::processNormalComment(size_t pos, const std::string &line)
{
    switch (line[pos]) {
    case '\\':  // process doxy command or escaped char
       /* switch (line[pos + 1]) {
        case '$':
        case '@':
        case '\':
        case '&':
        case '~':
        case '<':
        case '>':
        case '#': \% \" \. \::
        }
        TODO break case if any of escaped chars */
    case '@': {
      pos++;
      // characters '$[]{}' are used in commands \f$, \f[, ...
      size_t endOfWordPos = line.find_first_not_of("abcdefghijklmnopqrstuvwxyz$[]{}", pos);
      string cmd = line.substr(pos , endOfWordPos - pos);
      addDoxyCommand(m_tokenList, cmd);
      if (cmd == CMD_HTML_ONLY  ||  cmd == CMD_VERBATIM) {
          m_isVerbatimText = true;
      }
      // skip any possible spaces after command, because some commands have parameters,
      // and spaces between command and parameter must be ignored.
      if (endOfWordPos != string::npos) {
          pos = line.find_first_not_of(" \t", endOfWordPos);
      } else {
          pos = string::npos;
      }
    } break;

    case ' ':  // whitespace
    case '\t': {
      // whitespaces are stored as plain strings
      size_t startOfNextWordPos = line.find_first_not_of(" \t", pos + 1);
      m_tokenList.push_back(Token(PLAINSTRING,
                               line.substr(pos, startOfNextWordPos - pos)));
      pos = startOfNextWordPos;
    } break;

    case '<': { // process html commands

      size_t endHtmlPos = line.find_first_of("\t >", pos + 1);
      if (endHtmlPos != string::npos) {
        // will push plain string Token. If the command is not HTML supported by
        // Doxygen, < and > will be replaced by HTML entities &lt; and &gt; respectively,
        // but only if 'htmlOnly' flag == false. The flag is set/reset by \htmlonly \verbatim,
        // \endhtmlonly \endverbatim Doxygen commands.
        // handleHTMLCommand(line.substr(pos + 1), endHtmlPos - pos - 1);
      }
      pos = endHtmlPos;
    } break;

    case '&': { // process HTML entities
        size_t endOfWordPos = line.find_first_not_of("abcdefghijklmnopqrstuvwxyz", pos + 1);
        if (endOfWordPos != string::npos) {
            if (line[endOfWordPos] == ';') {
                addDoxyCommand(m_tokenList, line.substr(pos, endOfWordPos));
            } else {
                // it is not an entity - push plain string
                m_tokenList.push_back(Token(PLAINSTRING,
                                      line.substr(pos, endOfWordPos - pos)));
                pos = endOfWordPos;
            }
        } else {
            pos = string::npos;
        }
    }
    break;
    default:
      printListError(WARN_DOXYGEN_COMMAND_ERROR, "Unknown special character: " + line[pos]);
    }

    return pos;
}


/**
 * This method tokenizes Doxygen comment to words and doxygen commands.
 */
void DoxygenParser::tokenizeDoxygenComment(const std::string &doxygenComment,
                                                 const std::string &fileName,
                                                 int fileLine)
{
  m_isVerbatimText = false;
  m_tokenList.clear();
  m_fileLineNo = fileLine;
  m_fileName = fileName;

  StringVector lines = split(doxygenComment, '\n');

  for (StringVectorCIt it = lines.begin(); it != lines.end(); it++) {
    const string &line = *it;
    size_t pos = line.find_first_not_of(" \t");

    // skip sequences of '*', '/', and '!' of any length
    while (pos != string::npos  &&  isStartOfDoxyCommentChar(line[pos])) {
      pos++;
    }

    if (pos == string::npos) {
        m_tokenList.push_back(Token(END_LINE, "\n"));
      continue;
    }

    // line[pos] may be ' \t' or start of word, it there was no '*', '/' or '!'
    // at beginning of the line. Make sure it points to start of the first word
    // in the line.
    pos = line.find_first_not_of(" \t", pos);
    if (pos == string::npos) {
      m_tokenList.push_back(Token(END_LINE, "\n"));
      continue;
    }

    while (pos != string::npos) {
      // find the end of the word
      size_t doxyCmdOrHtmlTagPos = line.find_first_of("\\@< \t", pos);
      if (doxyCmdOrHtmlTagPos != pos) {
        // plain text found
        m_tokenList.push_back(Token(PLAINSTRING,
                                 line.substr(pos, doxyCmdOrHtmlTagPos - pos)));
      }

      pos = doxyCmdOrHtmlTagPos;
      if (pos != string::npos) {
        if (m_isVerbatimText) {
            pos = processVerbatimText(pos, line);
        } else {
            pos = processNormalComment(pos, line);
        }
      }
    }
    m_tokenList.push_back(Token(END_LINE, "\n")); // add when pos == npos - end of line
  }

  m_tokenListIt = m_tokenList.begin();
}


void DoxygenParser::printList() {

  int tokNo = 0;
  for (TokenListCIt it = m_tokenList.begin(); it != m_tokenList.end();
                                                              it++, tokNo++) {

    cout << it->toString() << " ";

    if ((tokNo % TOKENSPERLINE) == 0) {
      cout << endl;
    }
  }
}


void DoxygenParser::printListError(int warningType,
                                        const std::string &message) {
  int curLine = m_fileLineNo;
  for (TokenListCIt it = m_tokenList.begin(); it != m_tokenListIt; it++) {
    if (it->m_tokenType == END_LINE) {
      curLine++;
    }
  }

  Swig_warning(warningType, m_fileName.c_str(), curLine,
                 "Doxygen parser warning: %s. \n", message.c_str());
}
