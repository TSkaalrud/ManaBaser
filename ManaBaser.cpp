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

void getSingleInput(int* var);

int main()
{
    using namespace boost::math;
    using std::cin;
    using std::cout;

    int modeSelected;
    cout << "***Welcome to ManaBaser***\n";
    while (1) {
        //Starting prompt and input check
        cout << "\nWhat would you like to do? (0-4); 0 to close the program\n" <<
            "(0) Exit.\n" <<
            "(1) (Basic) Check my draw chances based on my current deck.\n" <<
            "(2) (Advanced) Check my mana draws based on my current deck.\n" <<
            "[Coming Soon] (3) Calculate the colored sources required in my deck to reliably have them on-curve.\n" <<
            "[Coming Soon] (4) Input a url to a premade deck and get stats based on it.\n";
        getSingleInput(&modeSelected);

        //Mode 0 exits
        switch (modeSelected)
        {
        case 0:
            return EXIT_SUCCESS;
            //Mode 1
        case 1:
        {
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
            getSingleInput(&deckSize);


            cout << "\nFor each of the prior categories, \n" <<
                "how many of the card type are in your deck and how many of that type do you want to draw in your opening hand.\n" <<
                "e.g.: 35 2\n\n";
            std::vector<std::vector<int>> statsInputs;
            int drawSum = 0; //also the number of turns worth of land drops 
            do {
                for (unsigned int i = 0; i < inputCategories.size(); i++) {
                    cout << "Category " << inputCategories[i] << ": ";
                    std::vector<int> categoryStats;
                    for (int j = 0; j < 2; j++) {
                        int temp;
                        getSingleInput(&temp);
                        categoryStats.push_back(temp);
                    }
                    drawSum += categoryStats[1];
                    statsInputs.push_back(categoryStats);
                }
                if (drawSum > 7) {
                    "You are attempting to draw too many cards, try again";
                    statsInputs.clear();
                }
            } while (drawSum > 7);
            

            cout << "\nBased on what you've given:\n";
            int drawsLeft = 7;
            int landsLeft = 0;
            for (unsigned int i = 0; i < inputCategories.size(); i++) {
                double pdfTemp = pdf(hypergeometric(statsInputs[i][0], drawsLeft, deckSize), statsInputs[i][1]);
                double cdfTemp = (cdf(hypergeometric(statsInputs[i][0], drawsLeft, deckSize), statsInputs[i][1]) - cdf(hypergeometric(statsInputs[i][0], drawsLeft, deckSize), 0));
                cout << "Your chances of drawing exactly " << statsInputs[i][1] << " " << inputCategories[i] << " are: " << pdfTemp * 100 << "%\n";
                cout << "Factoring in a free mulligan, this goes up to " << (pdfTemp + ((1 - pdfTemp) * pdfTemp)) * 100 << "%\n";
                cout << "Excluding none, your chances of drawing up to that many are: " << cdfTemp * 100 << "%\n";
                cout << "Factoring in a free mulligan, these cumulative chances increase to " << (cdfTemp + ((1 - cdfTemp) * cdfTemp)) * 100 << "%\n";

                drawsLeft -= statsInputs[i][1];
                deckSize -= statsInputs[i][1];
                landsLeft += (statsInputs[i][0] - statsInputs[i][1]);
            }
            cout << "Subsequent draws give a " <<
                (cdf(hypergeometric(landsLeft, drawSum, deckSize), drawSum) - cdf(hypergeometric(landsLeft, drawSum, deckSize), 0)) * 100 <<
                "% chance of you drawing further lands before you run out.\n\n";
            cout << "Press any key to return to main menu\n";
            cout << "Lands: " << landsLeft << std::endl;
            system("pause>0");
        }
        case 2:
            cout << "Feature coming soon! (TM)";
            break;
        case 3:
            cout << "Feature coming soon! (TM)";
            break;
        default:
            cout << "Staaahhhhp! Please do as you're told. \n";
            break;
        }
    }
}


void getSingleInput(int* var) {
    using std::cin;
    using std::cout;
    while (!(cin >> *var)) {
        cin.clear(); //clear bad input flag
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //discard input
        cout << "Don't make me get sassy, gimme a real input!\n\n";
    }
    return;
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
