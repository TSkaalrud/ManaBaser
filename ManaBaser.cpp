/*Requirements:
-Inputs :
    - Mode 1 : provide which / how many colors a deck is running & provide how many lands of each there are
    - Mode 2 : provide how much of X colors is required by Y - turn and a minimum acceptable chance of doing so.
    - Mode 3 : provide a public decklist url (various websites or formatted .txt)
- Outputs :
    - Mode 1 : respond with chances of draws, draws including mulligans, subsequent draws
    - Mode 2 : respond with source counts for each required by the deck
    - Mode 3 : Scan through the decklist and shit out statistics(stretch goal) 
*/

#include <boost/lambda/lambda.hpp>
#include <boost/math/distributions/hypergeometric.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <string>

int main()
{
    using namespace boost::math;
    using std::cin;
    using std::cout;

    int modeSelected;
    cout << "***Welcome to ManaBaser***\n";
    while (1) {
        //Starting prompt and input check
        cout << "What would you like to do? (1-2)\n" <<
            "(1) Check my draw chances based on my current deck\n" <<
            "(2) Calculate the colored sources required in my deck to reliably have them on-curve\n";
        cin >> modeSelected;
        while (cin.fail()) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Don't make me get sassy, gimme a real input!\n";
            cin >> modeSelected;
        }
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        //Mode 1
        if (modeSelected == 1) {
            std::vector<std::string> inputCategories;
            std::string inputBuffer = "";

            cout << "\nAlright, let's figure out if you just have bad luck or you suck at deckbuilding! \n" <<
            "How many different things are you looking to draw? (Usually colors/land, but you can check whatever) \n" <<
            "Just list 'em off with spaces e.g.:Black White Instants\n";
            do {
                cin >> inputBuffer;
                cout << inputBuffer << std::endl;
                inputCategories.push_back(inputBuffer);
            } while (cin.get() != '\n');

        }
        //Mode 2
        else if (modeSelected == 2) {

        }
        //Mode 3
        else if (modeSelected == 3) {
            cout << "Feature coming soon! (TM)";
        }
        //Nonexistent Modes
        else {
            cout << "Staaahhhhp! Please do as you're told. \n";
        }
    }




}

/* test this calculates the cumualtive probability of drawing from 35 lands, with a full hand of 7 cards, in a 99 card edh deck, of drawing up to 7 lands (0-7 lands inclusive)
std::cout << cdf(hypergeometric(35,7,99), 7);
*/

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
