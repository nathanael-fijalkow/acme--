
#include <iostream>
#include "Expressions.hpp"
#include "Matrix.hpp"
#include "MarkovMonoid.hpp"
#include "StabilisationMonoid.hpp"
#include "StarHeight.hpp"

#include <fstream>
#include <sstream>

#ifdef MSVC
#include <windows.h>
#include <time.h>
#endif

using namespace std;


ExplicitAutomaton * expa = NULL;

int letters_nb = 2;

//return true iff an increment was performed
bool inc_mat_and_states() {
    auto stnb = expa->size;
    auto & matrices = expa->matrices;

    bool done = false;
    for(int letter = 0; !done && (letter < letters_nb); letter++) {
        auto & m= matrices[letter];
        for(int i = 0 ; !done && (i < stnb); i++) {
            for(int j =0; !done && (j < stnb); j++) {
                if(m.coefficients[i][j]==0) {
                    m.coefficients[i][j] = 1;
                    for(int i0 = 0 ; i0 < i; i0++) {
                        for(int j0 =0; j0 < stnb; j0++) {
                            m.coefficients[i0][j0] = 0;
                        }
                    }
                    for(int j0 =0; j0 < j; j0++) {
                            m.coefficients[i][j0] = 0;
                    }
                    done = true;
                }
            }
        }
    }
    return done;
}

int main(int argc, char **argv)
{
    
    cout << "Stamina rules" << endl;
    
    //    unsigned int seed = time(NULL);
    
    
    int max_state_nb = 5;
    
    stringstream filename;
    filename << "StarHeight_maxstatenb " << max_state_nb;
    
    ofstream file(filename.str() + ".csv");
    file << "#;Size;StatesNb;ElementsNb;RewriteRulesNb;VectorNb;StarHeight" << endl;
    file.close();
    
    uint nb = 0;
    
    
    for(int stnb = 1 ; stnb < max_state_nb; stnb++) {
        
        expa = new ExplicitAutomaton(stnb,letters_nb);
        expa->initialState = 0;
        expa->finalStates.push_back(stnb -1);

        while(true) {
            nb++;

            ClassicAut aut(*expa);
            if(!aut.iscomplete()) {
               // cout << "Automaton #" << nb << " not complete" << endl;
            } else if (!aut.isdet()){
                //cout << "Automaton #" << nb << " not deterministic" << endl;
            } else {
                cout << "Automaton #" << nb << " complete and deterministic" << endl;
                UnstableMultiMonoid * monoid = NULL;
                const ExtendedExpression * witness = NULL;
                auto h = computeStarHeight(aut, monoid, witness, false, false);
                
                if(monoid != NULL && h>0) {
                    cout << "SharpHeight " << h << endl;
                    ofstream file(filename.str() + ".csv", ofstream::app);
                    file << nb << ";" << stnb << ";" << monoid->expr_to_mat.size();
                    file << ";" << monoid->rewriteRules.size() << ";" << int_vectors.size();
                    file << ";" << h << endl;
                    file.close();
                } else {
                    cout << "Automaton #" << nb << " empty language" << endl;
                }
                delete monoid;
            }
            
            if(!inc_mat_and_states()) break;
        }
    }
    
    
    file.close();
    
    cout << "Experiment over " << endl;
    
}
