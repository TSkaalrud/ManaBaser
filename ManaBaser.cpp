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
double pdfprob(int desiredCards, int deckSize, int draws = 1, int successes = 0);
double cdfprob(int desiredCards, int deckSize, int draws = 1, int successes = 0);
void mulligan(std::vector<double> mulliganTable, double mulliganChance);

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
        case 1: {
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
                drawsLeft -= statsInputs[i][1];
                deckSize -= statsInputs[i][1];
                cout << "Your chances of drawing exactly " << statsInputs[i][1] << " " << inputCategories[i] << " are: " << pdfTemp * 100 << "%\n";
                cout << "Factoring in a free mulligan, this goes up to " << (pdfTemp + ((1 - pdfTemp) * pdfTemp)) * 100 << "%\n";
                cout << "Excluding none, your chances of drawing up to that many are: " << cdfTemp * 100 << "%\n";
                cout << "Factoring in a free mulligan, these cumulative chances increase to " << (cdfTemp + ((1 - cdfTemp) * cdfTemp)) * 100 << "%\n";
                landsLeft += (statsInputs[i][0] - statsInputs[i][1]);
            }
            cout << "Subsequent draws give a " <<
                (cdf(hypergeometric(landsLeft, drawSum, deckSize), drawSum) - cdf(hypergeometric(landsLeft, drawSum, deckSize), 0)) * 100 <<
                "% chance of you drawing further lands before you run out.\n\n";
            cout << "Press any key to return to main menu\n";
            system("pause>0");
        }
        break;
      
        case 2: {
            /*This mode will provide stats on land drops incorporating lands that can provide multiple colors
            Step 1 will ask for the number of colors used in the deck in order to simplify some cases (5-color being the worst as it has access to 1,2,3 and 5 color lands)
                2 Color combos include the allies: Dimir, Azorius, Selesnya, Gruul, & Rakdos and the enemies: Orzhov, Boros, Izzet, Simic, & Golgari.
                3 Color combos include the shards: Bant, Esper, Grixis, Jund, & Naya and the wedges: Abzan, Jeskai, Sultai, Mardu, Temur
                The 5 Nephilim and 5 colors can fall into the same category, as will mono color decks
            Step 2 */
            cout << "\nHow many colors is your deck?\n";
            int colorCount;
            getSingleInput(&colorCount);

            switch (colorCount)
            {
            case 1: {
                cout << "\nHow many cards are in your deck?\n";
                int deckSize;
                getSingleInput(&deckSize);
                cout << "\nHow many lands are in your deck?\n";
                int landCount;
                getSingleInput(&landCount);
                double mulliganChance = 0;
                std::vector<double> mulliganTable;

                for (int i = 0; i <= 7; i++) {
                    double pdfResult = pdfprob(landCount, deckSize, 7, i);
                    if (i == 0 || i == 1 || i == 6 || i == 7) {
                        mulliganChance += pdfResult;
                    }
                    mulliganTable.push_back(pdfResult);
                    cout << "Your odds of a " << i << " land hand are: " << pdfResult * 100 << "%.\n";
                }
                cout << "\nIf we include a free mulligan, these odds become:\n";
                mulligan(mulliganTable, mulliganChance);
                cout << "Press any key to return to main menu\n";
                system("pause>0");
            }
            break;
            
            //starting with case 2, begin with the lands that cover the most colors first and recursively add them down the line 5>3>2>1 to get final counts
            case 2: {
                //3 steps, 1 for each color and 1 for the combination

            }
                break;
            
            case 3: {
                //7 steps, 1 for each color, 1 for each pair, and 1 for the tri

            }
                break;
            
            case 4://case 4 falls through to case 5
            case 5: {
                //21 steps 1 for each color, 10 for each pair, 5 for each tri, and 1 for 5c
            }
                break;
            
            default: {

            }
                break;
            }
        }
            break;
        case 3: {
            //Ask the user for each color they care about, then ask them which turns they care about, then ask them for how much mana they need
            //then calculate
            cout << "Feature coming soon! (TM)"; 
        }
            break;
        case 4: {
            //Learn networking shenanigans
            cout << "Feature coming soon! (TM)";
        }
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

double pdfprob(int desiredCards, int deckSize, int draws, int successes) {
    return boost::math::pdf(boost::math::hypergeometric(desiredCards, draws, deckSize), successes);
}

double cdfprob(int desiredCards, int deckSize, int draws, int successes) {
    return boost::math::cdf(boost::math::hypergeometric(desiredCards, draws, deckSize), successes);
}

void mulligan(std::vector<double> mulliganTable, double mulliganChance) {
    using std::cout;
    double screwedChance = 0;
    for (int i = 0; i <= 7; i++) {
        if (i == 0 || i == 1 || i == 6 || i == 7) {
            double result = mulliganTable[i] * mulliganChance;
            cout << "Your odds of a " << i << " land hand are: " << result * 100 << "%.\n";
            screwedChance += result;
        }
        else {
            double result = mulliganTable[i] + mulliganTable[i] * mulliganChance;
            cout << "Your odds of a " << i << " land hand are: " << result * 100 << "%.\n";
        }
    }
    cout << "Overall, your deck's chances of being screwed/flooded are: " << screwedChance * 100 << "%.\n";
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
