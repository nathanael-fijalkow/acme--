
#include "Parser.hpp"
#include "Automata.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <exception>
using namespace std;
int linenumber=0;
istream& getfline(istream& is,string& str, char delim='\n')
{
    do {
        is.peek();
        getline(is,str,delim);
        if(delim!=' ')
            linenumber++;
    } while(str.empty()||str[0]=='%');
    
    return is;
}

int lttoi(string s)
{
    if(!s.compare("_"))
        return 0;
    int res;
    try {
        res = stoi(s)+1;
    }
    catch(exception& e) {
        cerr << "Syntax error at line: " << linenumber <<  endl << "Exception: " << string(e.what()) << endl;
        exit(-1);
    }
    return res;
}

ExplicitAutomaton* Parser::parseFile(std::istream &file)
{
    string line;
    
    getfline(file,line);
    int size = stoi(line);
    
    int type;
    getfline(file,line);
    if(!line.compare("p"))
        type=PROB;
    else {
        if (!line.compare("c"))
            type=CLASSICAL;
        else
            type=stoi(line);
    }
    
    string alphabet;
    getfline(file,alphabet);
    
    ExplicitAutomaton* ret = new ExplicitAutomaton(size,alphabet.length());
    ret->matrices.clear();//backward compatibility
    ret->type = type;
    ret->alphabet = alphabet;
    
    getfline(file,line);
    try
    {
        ret->initialState = stoi(line);
        //	  cout << "Initial state: " << stoi(line) << endl;
    }
    catch(const exception & exc){ throw runtime_error("Error while parsing initial states, could not parse '" + line + "' to integer"); }
    
    ret->finalStates.clear();
    
    getfline(file,line);
    istringstream iss(line);
    while(getfline(iss,line,' ')) {
        try
        {
            ret->finalStates.push_back(stoi(line));
        }
        catch(const exception & exc)
        {
            throw runtime_error("Error while parsing final states, could not parse line" + line + " to integer");
        }
    }
    
    
    for(int i=0;i<alphabet.length();i++) {
        ExplicitMatrix mat(size);
        string lt;
        getfline(file,lt);
        for(int j=0;j<size;j++) {
            getfline(file,line);
            istringstream iss2(line);
            for(int k=0;k<size;k++) {
                getfline(iss2,line,' ');
                if(type <= 0)
                    mat.coefficients[j][k]=lttoi(line);
                else
                    mat.coefficients[j][k]=MultiCounterAut::coef_to_char(line,type);
                
            }
        }
        ret->matrices.push_back(mat);
    }
    
    return ret;
}
