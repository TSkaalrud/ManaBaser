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
        cout << "What would you like to do? (1-2); 0 to close the program\n" <<
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
        //Mode 0 exits
        if (modeSelected == 0)
            return EXIT_SUCCESS;
        //Mode 1
        if (modeSelected == 1) {
            std::vector<std::string> inputCategories;
            std::string inputBuffer = "";

            //Query user for how many things they're interested in when drawing
            cout << "\nAlrighty then, let's figure out if you have bad luck or you just suck at math! \n" <<
            "How many exclusive categories of things are you looking to draw? (Usually lands, but you can check whatever) \n" <<
            "name your categories off with spaces\n e.g.:Swamps Plains\n";
            do {
                cin >> inputBuffer;
                inputCategories.push_back(inputBuffer);
            } while (cin.get() != '\n');

            //Query user for each category's statistics relevant info (fills args 1 and 4 of hypergeometric)
            //Handle inputs for valid types and stats (0 in distribution arg 1 throws) also making sure they aren't trying to draw more than 7
            //check to make sure deckSize isn't exceeded by the total of all categories
            cout << "\nNext, I'll get you to tell me about your deck. " <<
                "How large is your deck?\n";
            int deckSize;
            cin >> deckSize;
            cout<< "\nFor each of the prior categories, \n" <<
                "how many of the card type are in your deck and how many of that type do you want to draw in your opening hand.\n" <<
                "e.g.: 35 2\n\n";
            std::vector<std::vector<int>> statsInputs;
            for (int i = 0; i < inputCategories.size(); i++) {
                cout << "Category " << inputCategories[i] << ": ";
                std::vector<int> categoryStats;
                for (int j = 0; j < 2; j++) {
                    int temp;
                    cin >> temp;
                    categoryStats.push_back(temp);
                }
                statsInputs.push_back(categoryStats);
            }
            cout << "Based on what you've given:\n";
            int drawsLeft = 7;
            int landsLeft = 0;
            int totalDrawn = 0; //the number of turns worth of land drops 
            for (int i = 0; i < inputCategories.size(); i++) {
                float pdfTemp = pdf(hypergeometric(statsInputs[i][0], drawsLeft, deckSize), statsInputs[i][1]);
                float cdfTemp = (cdf(hypergeometric(statsInputs[i][0], drawsLeft, deckSize), statsInputs[i][1]) - cdf(hypergeometric(statsInputs[i][0], drawsLeft, deckSize), 0));
                cout << "Your chances of drawing exactly " << statsInputs[i][1] << " " << inputCategories[i] << " are: " << pdfTemp * 100 << "%\n";
                cout << "Factoring in a free mulligan, this goes up to " << (pdfTemp + ((1 - pdfTemp) * pdfTemp)) * 100 << "%\n";
                cout << "Excluding none, your chances of drawing up to that many are: " << cdfTemp * 100 << "%\n";
                cout << "Factoring in a free mulligan, these cumulative chances increase to " << (cdfTemp + ((1 - cdfTemp) * cdfTemp)) * 100 << "%\n";

                drawsLeft -= statsInputs[i][1];
                deckSize -= statsInputs[i][1];
                landsLeft += (statsInputs[i][0] - statsInputs[i][1]);
                totalDrawn += statsInputs[i][1];
            }
            cout << "Assuming that hand was interested only in land, subsequent draws give a " <<
                (cdf(hypergeometric(landsLeft, totalDrawn, deckSize), totalDrawn) - cdf(hypergeometric(landsLeft, totalDrawn, deckSize), 0)) * 100 <<
                "% chance of you drawing further lands before you run out.\n\n";
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
