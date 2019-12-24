# Stamina: STabilisation Monoids IN Automata theory

Stamina is a tool implementing algebraic techniques to solve decision problems from automata theory.
More specifically, it uses stabilisation monoids as an algorithmic back-end to solve two problems: the value 1 problem for probabilistic automata
and that boundedness problem for automata with counters.
Most importantly, it is, to the best of our knowledge, the very first implementation of an algorithm solving the star-height problem.

It has been written in C++ by [Nathanaël Fijalkow](nathanael-fijalkow.github.io/), [Hugo Gimbert](https://www.labri.fr/perso/gimbert/), [Edon Kelmendi](https://www.cs.ox.ac.uk/people/edon.kelmendi/) and [Denis Kuperberg](https://perso.ens-lyon.fr/denis.kuperberg/).

Stamina is the successor of [ACME](https://github.com/nathanael-fijalkow/acme), which also implements the transformation of an automaton into a stabilisation monoid. 
The point of Stamina is that it is much much faster, and does a lot of space optimisations.
This allows to handle much bigger automata, which is required to solve the star-height problem.

See the [tool paper presentation at CIAA'2017](https://github.com/nathanael-fijalkow/stamina/blob/master/CIAA_2017_Stamina.pdf).

### Installation

To compile, open a terminal, go to the Stamina directory, type

cmake .
make

The command
./StaminaTest file.txt -o out.dot

reads the automaton from file.txt, and outputs what is computed in out.dot, a graphic format. 
If the automaton is a probabilistic automaton, it runs the Markov Monoid algorithm, if it is a classical non-deterministic automaton, it computes its star-height. 
Line by line description of the input file format for automata:
the first line is the size of the automaton (number of states).
the second line is the type of the automaton: c for classical, p for probabilistic, or a number n for a cost automaton with n counters.
the third line is the alphabet. Each character is a letter, they should not be separated.
the fourth line is the initial states. Each state should be separated by spaces.
the fifth line is the final states. Each state should be separated by spaces.
the next lines are the transition matrices, one for each letter in the input order. A transition matrix is given by actions (1 and _) separated by spaces. Each matrix is preceded by a single character line, the letter (for readability and checking purposes).
For cost automata, the coefficients of the matrix are I1, I2,... for increments of counter 1,2, etc, R1,R2,... for resets, E for epsilon, and _ for no transition.

Careful: if the input is a classical automaton for star-height computation, it has to be deterministic. In future versions, we might accept instead the dual (accepting states reversed) of a non-deterministic automaton for the complement language.

### Sage Library 

After compiling Stamina, copy the files stamina.py and libacme.so to sage/stamina-0.1/src/. 
To create a Sage package:
$ sage --pkg stamina-0.1


It produces a file stamina-0.1.spkg. It can be installed by
$ sage -p stamina-0.1.spkg


Now run Sage:
$ sage

sage: import stamina

sage: aut = Automaton({0:[(1,'a')],1:[(1,'a')]})

sage: aut.state(0).is_initial = True

sage: aut.state(1).is_final = True

sage: m = stamina.to_monoid(aut)

sage: m.has_val1()

sage: m.starheight()






