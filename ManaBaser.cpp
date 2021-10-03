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
#include <map>
#include <list>
#include <math.h>

void getSingleInput(int* var);
void getSingleInput(std::string* var);
double pdfprob(int desiredCards, int deckSize, int draws = 1, int successes = 0);
double cdfprob(int desiredCards, int deckSize, int draws = 1, int successes = 0);
void mulligan(std::vector<double> mulliganTable, double mulliganChance);
int parseColorInput(std::string deckColors, std::map<std::string, int>& map);
void openingDrawStats(std::string deckColors, std::map<std::string, int> map, int landCount);

int main()
{
    using namespace boost::math;
    using std::cin;
    using std::cout;
    using std::string;
    using std::endl;

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
            std::vector<string> inputCategories;
            string inputBuffer = "";

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
            cout << "\nNext, I'll get you to tell me about your deck. e.g. 40 60 99 for Draft, Standard, or EDH respectively." <<
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
            int colorCount;
            int landCount;
            string colorString;
            std::map<string, int> colorMap;

            //Parse input properly
            cout << "\nWhat colors are your deck? E.G. BGRUW or BW\n";
            getSingleInput(&colorString);
            std::sort(colorString.begin(), colorString.end());
            cout << colorString << endl;
            colorCount = colorString.length();
            cout << "Okay, a " << colorCount << " color deck.\n";

            //Monocolors
            for (int i = 0; i < colorCount; i++) {
                colorMap.insert(std::pair<string, int>(string(1, colorString[i]), 0));
                //Bicolors
                for (int j = i + 1; j < colorCount; j++) {
                    if (colorCount > 1)
                        colorMap.insert(std::pair<string, int>(string(1, colorString[i]) + colorString[j], 0));
                    //tricolors
                    for (int k = j + 1; k < colorCount; k++) {
                        if (colorCount > 2)
                            colorMap.insert(std::pair<string, int>(string(1, colorString[i]) + colorString[j] + colorString[k], 0));

                    }
                }
            }
            if (colorCount > 3) //4 & 5 colors
                colorMap.insert(std::pair<string, int>("BGRUW", 0));

            landCount = parseColorInput(colorString, colorMap);

            openingDrawStats(colorString, colorMap, landCount);
            }
            break;
            
        case 3: {
            //Ask the user for each color they care about, then ask them which turns they care about, then ask them for how much mana they need
            //then calculate
            cout << "\nFeature coming soon! (TM)";
        }
              break;
        
        case 4: {            
            //Learn networking shenanigans
            cout << "\nFeature coming soon! (TM)";
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
void getSingleInput(std::string* var) {
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
    cout << "Overall, your deck's chances of being screwed/flooded are: " << screwedChance * 100 << "%.\n\n";
    return;
}

/*iterates over all the color combinations a given color deck could use, 
asks the user for the count of each, updates the map's monocolored values, and returns the total land count
*/
int parseColorInput(std::string deckColors, std::map<std::string, int>& map) {
    int landCount = 0;
    std::map<std::string, int>::iterator it = map.begin();
    while (it != map.end()) {
        std::string colors = it->first;
        int currentCount = it->second;
        std::cout << "\nHow many " << colors << " lands are in your deck?\n";
        int temp;
        std::cin >> temp;
        landCount += temp;

        for (char c : colors) {
            if (map.count(std::string(1, c)))
            map.at(std::string(1,c)) += temp;
        }

        it++;
    }
    for (char c : deckColors) {
        std::cout << "You have " << map.at(std::string(1, c))<< " " << c<< " land sources in your deck." << std::endl;
    }
    return landCount;
}

void openingDrawStats(std::string deckColors, std::map<std::string, int> map, int landCount) {
    using std::cout;
    using namespace boost::math;

    //Query user for each category's statistics relevant info (fills args 1 and 4 of hypergeometric)
    //Handle inputs for valid types and stats (0 in distribution arg 1 throws) also making sure they aren't trying to draw more than 7
    //check to make sure deckSize isn't exceeded by the total of all categories
    cout << "\nHow many cards are in your deck?\n";
    int deckSize;
    getSingleInput(&deckSize);

    bool again = true;
    while (again) {
        int drawSum; //also the number of turns worth of land drops 
        int landsLeft = landCount;
        std::vector<int> statsInputs;
        do {
            drawSum = 0;
            for (int i = 0; i < deckColors.length(); i++) {
                cout << "\nHow many " << deckColors[i] << " would you like in your opening hand?\n";
                int temp;
                getSingleInput(&temp);
                statsInputs.push_back(temp);
                drawSum += temp;
            }
            if (drawSum > 7) {
                cout << "You are attempting to draw too many cards, try again.\n";
                statsInputs.clear();
            }
        } while (drawSum > 7);

        int drawsLeft = 7;

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
        cout << "Your chances of having an initially keepable hand (2-5 lands) are: " << (1 - mulliganChance) * 100 << "%\n";
        cout << "\nIf we include a free mulligan, these odds become:\n";
        mulligan(mulliganTable, mulliganChance);

        //Disclaimer to not being entirely accurate
        cout << "*Due to dependency complications in cases incorporating multicolored lands, the following probabilities are considered estimates*\n";
        for (int i = 0; i < deckColors.length(); i++) {
            int sourceCount = map.at(std::string(1, deckColors[i]));
            int sourceDesired = statsInputs[i];

            //cout << sourceCount << " " << drawsLeft << " " << deckSize << " " << sourceDesired << std::endl;

            double pdfTemp = pdf(hypergeometric(sourceCount, drawsLeft, deckSize), std::min(sourceCount, sourceDesired));
            double cdfTemp = (cdf(hypergeometric(sourceCount, drawsLeft, deckSize), std::min(sourceCount, drawsLeft)) -
                              cdf(hypergeometric(sourceCount, drawsLeft, deckSize), 0));
            drawsLeft -= sourceDesired;
            deckSize -= sourceDesired;
            landsLeft -= sourceDesired;

            cout << "Your chances of drawing exactly " << sourceDesired << " " << deckColors[i] << " are: " << pdfTemp * 100 << "% on a given draw.\n";
            cout << "Your chances of drawing any " << deckColors[i] << " are: " << cdfTemp * 100 << "% on a given draw.\n";

        }

        cout << "\nSubsequent draws upon getting this hand give an [accurate] " <<
            (cdf(hypergeometric(landsLeft, drawSum, deckSize), drawSum) - cdf(hypergeometric(landsLeft, drawSum, deckSize), 0)) * 100 <<
            "% chance of you drawing further lands before you run out.\n\n";

        cout << "Enter 1 to check more stats or 0 to return to the main menu.\n";
        std::cin >> again;
    }
}

/* test this calculates the cumualtive probability of drawing from 35 lands, with a full hand of 7 cards, in a 99 card edh deck, of drawing up to 7 lands (0-7 lands inclusive)
std::cout << cdf(hypergeometric(35,7,99), 7);
*/