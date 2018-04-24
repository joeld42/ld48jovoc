#include "glm/gtc/random.hpp"

#include "CardFishGame.h"

// Cards TODO:

// Scoop net (scoops up all dead fish)
// Chum (pulls next 3 fish from deck into lake)
// Submersible Cloner (Duplicates any fish card value 3 or less)
// Capsize (Shuffles tackle back into deck)


// Note: Spacing here is important for the janky parse-o-tron
void CardFishGame::AddCardDef  ( Oryol::String cardId,
                               Oryol::String title,
                               Oryol::String artwork,
                               Oryol::String flavorText,
                               Oryol::String actionText,
                               CardType cardType,
                               int reelPow,
                               int fishPoints,
                               int depth )
{
    Card card = {};
    card.cardId = cardId;
    card.cardType = cardType;
    card.title = title;
    //flavorText  // Don't need these in here, used by mk_cards
    //artwork;
    //actionText
    card.reelPow = reelPow;
    card.fishPoints = fishPoints;
    card.depth = depth;
    card.cardFaceIndex = cardDefs.Size();
    
    // Special rules for some cards
    if (card.cardId == "cast2") {
        card.charges = 1;
    }
    
    cardDefs.Add( card );
}

Card CardFishGame::FindCardDef(  Oryol::String cardId )
{
    for (int i=0; i < cardDefs.Size(); i++) {
        if (cardDefs[i].cardId==cardId) {
            return cardDefs[i];
        }
    }
    
    printf("WARNING: FindCardDef didn't find card, this shouldn't happen.\n");
    return cardDefs.Back();
}

void CardFishGame::SetupCardDefs()
{
    AddCardDef( "cast", "Cast", "castart1.png",
               "A basic cast can still be the most effective.",
               "Hook a fish at random from the lake!",
               CardType_CAST, /*reelPow*/ 3, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "cast2", "Repeater", "castart2.png",
               "If you make one good cast try and do it again.",
               "Returns to deck the first time it's played.",
               CardType_CAST, /*reelPow*/ 1, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "trout", "Rainbow Trout", "trout.png",
               "Not a lot of meat on it but still tasty.",
               "",
               CardType_FISH, /*reelPow*/ 1, /*fishPoints*/ 4, /*depth*/ 5 );
    
    AddCardDef( "bass", "Smallmouth", "small_bass.png",
               "The Smallmouth Bass hunts with both keen vision and scent.",
               "",
               CardType_FISH, /*reelPow*/ 3, /*fishPoints*/ 6, /*depth*/ 8 );

    AddCardDef( "large_bass", "Largemouth", "large_bass.png",
               "Scientific research has indicated that bass like red better than any other color on the spectrum.",
               "",
               CardType_FISH, /*reelPow*/ 4, /*fishPoints*/ 8, /*depth*/ 9 );

    AddCardDef( "phat_bass", "Phat Bass", "phat_bass.png",
               "Yo immabouta drop the bass.",
               "",
               CardType_FISH, /*reelPow*/ 3, /*fishPoints*/ 5, /*depth*/ 7 );

    
    AddCardDef( "dead_fish", "Dead Fish", "dead_fish.png",
               "This fish is ded.",
               "",
               CardType_FISH, /*reelPow*/ 1, /*fishPoints*/ 2, /*depth*/ 4 );

    AddCardDef( "minnow", "Minnow", "minnow.png",
               "A teeny tiny fishy.",
               "",
               CardType_FISH, /*reelPow*/ 1, /*fishPoints*/ 1, /*depth*/ 3 );

    AddCardDef( "minnows", "Minnows", "minnows.png",
               "A school of teeny tiny fishies.",
               "Will spawn 3 minnows when played into lake.",
               CardType_FISH, /*reelPow*/ 2, /*fishPoints*/ 1, /*depth*/ 3 );

    AddCardDef( "shoe", "Old Shoe", "boot.png",
               "It's an old boot covered in slime.",
               "Cannot be trashed.",
               CardType_FISH, /*reelPow*/ 4, /*fishPoints*/ 0, /*depth*/ 8 );

    AddCardDef( "bluegill", "Bluegill", "bluegill.png",
                "Also known as sunfish= bream= or copper nose.",
                "",
               CardType_FISH, /*reelPow*/ 1, /*fishPoints*/ 3, /*depth*/ 4 );
    
    AddCardDef( "gloves", "Angler's Gloves", "tackleart1.png",
               "A precision grip and military-grade neoprene is the key to line control.",
               "Allowed Slack -1",
               CardType_TACKLE, /*reelPow*/ 2, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "high_test", "High-Test Line", "tackleart1.png",
               "Line is the most important factor between you and the fish.",
               "Max Tension +1",
               CardType_TACKLE, /*reelPow*/ 3, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "gearbox", "Gearbox", "tackleart1.png",
               "Crank it up.",
               "Doubles Reel Power",
               CardType_TACKLE, /*reelPow*/ 3, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "shiny_lure", "Shiny Lure", "tackleart1.png",
               "Fish can be easily distracted by shiny objects.",
               "Casts hook the highest valued fish in the lake.",
               CardType_TACKLE, /*reelPow*/ 4, /*fishPoints*/ 0, /*depth*/ 0 );
    
    AddCardDef( "power_reel", "Powered Reel", "actionart1.png",
               "Lots of torque but terrible battery life.",
               "Instantly reel in your line.",
               CardType_ACTION, /*reelPow*/ 3, /*fishPoints*/ 0, /*depth*/ 0 );
    
    AddCardDef( "zap_line", "Electric Line", "zap.png",
               "ZZZzzzzaaappp..",
               "Kills the fish on the line.",
               CardType_ACTION, /*reelPow*/ 2, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "scoop_net", "Scoop Net", "actionart1.png",
               "It's a lot easier getting them in the boat this way.",
               "Scoops up any Dead Fish instantly.",
               CardType_ACTION, /*reelPow*/ 4, /*fishPoints*/ 0, /*depth*/ 0 );

    AddCardDef( "capsize", "Capsize", "actionart1.png",
               "Never stand up in a canoe.",
               "Shuffles all equipped tackle back into your deck.",
               CardType_ACTION, /*reelPow*/ 4, /*fishPoints*/ 0, /*depth*/ 0 );

    
    AddCardDef( "dynamite", "Dynamite", "dynamite.png",
               "Hold my beer...",
               "Kills all the fish in the lake.",
               CardType_ACTION, /*reelPow*/ 2, /*fishPoints*/ 0, /*depth*/ 0 );

    
}

void CardFishGame::addCardsToDeck( Oryol::Array<Card> &cards, Oryol::String cardId, int howMany )
{
    // Find the requested card
    for (int i=0; i < cardDefs.Size(); i++) {
        if (cardId == cardDefs[i].cardId) {
            for (int j=0; j < howMany; j++) {
                cards.Add( cardDefs[i] );
            }
        }
    }
}

void CardFishGame::returnCardToDeck( Card card )
{
    int cardNdx = 0;
    if (deck.Size() > 1) {
        cardNdx = (rand() % (deck.Size() - 1));
    }
    deck.Insert(cardNdx, card );
    printf("ReturnCardToHand: Put card '%s' at index %d\n", card.title.AsCStr(), cardNdx );
}

void CardFishGame::shuffle( Oryol::Array<Card> &cards )
{
    // Fisher-yates shuffle
    for (int i=0; i < cards.Size()-2; i++) {
        int j = i + (rand() % (cards.Size() - i));
        Card tmp = cards[i];
        cards[i] = cards[j];
        cards[j] = tmp;
    }
}

void CardFishGame::resetGame()
{
    
    // Initial player deck
    fishPoints = 0;
    playerDeck.Clear();
    tackleCards.Clear();
    addCardsToDeck(playerDeck, "cast", 5 );
    addCardsToDeck(playerDeck, "cast2", 2 );
    addCardsToDeck(playerDeck, "gloves", 2 );
    addCardsToDeck(playerDeck, "high_test", 2 );
    addCardsToDeck(playerDeck, "shiny_lure", 1 );
    addCardsToDeck(playerDeck, "gearbox", 2 );
    addCardsToDeck(playerDeck, "power_reel", 1 );
    addCardsToDeck(playerDeck, "zap_line", 1 );
    addCardsToDeck(playerDeck, "dynamite", 1 );
    addCardsToDeck(playerDeck, "scoop_net", 2 );
    addCardsToDeck(playerDeck, "capsize", 1 );
    
    resetRound(0);
}

void CardFishGame::resetRound( int roundNum )
{
    // Build lake deck (TODO per-level)
    lakeDeck.Clear();
    addCardsToDeck(lakeDeck, "trout", 3 );
    addCardsToDeck(lakeDeck, "bass", 4 );
    addCardsToDeck(lakeDeck, "large_bass", 2 );
    addCardsToDeck(lakeDeck, "phat_bass", 1 );
    addCardsToDeck(lakeDeck, "bluegill", 4 );
    addCardsToDeck(lakeDeck, "minnow", 3 );
    addCardsToDeck(lakeDeck, "minnows", 1 );
    addCardsToDeck(lakeDeck, "shoe", 2 );
    
    // Build the deck for this round
    deck.Clear();
    for (int i=0; i < playerDeck.Size(); i++ ) {
        deck.Add( playerDeck[i] );
    }
    for (int i=0; i < lakeDeck.Size(); i++ ) {
        deck.Add( lakeDeck[i] );
    }
    
    // Shuffle combined deck
    shuffle( deck );
    
    Card dummy = {};
    reelCard = dummy;
    reelTension = 0;
    reelDistance = -1;
    reelSlackMin = 3;
    reelTensionMax = 7;
    
    lakeFish.Clear();
}


void CardFishGame::PlayTackleCard( Card card )
{
    tackleCards.Add( card );
    
    // card effect...
    if (card.cardId == "high_test") {
        if (reelTensionMax < 10) {
            reelTensionMax += 1;
        }
    } else if (card.cardId == "gloves") {
        if (reelSlackMin > 1) {
            reelSlackMin -= 1;
        }
    }
}

void CardFishGame::PlayCardToLake( Card card )
{
    lakeFish.Add( card );
}

Card CardFishGame::castLakeCard()
{
    assert( lakeFish.Size() > 0);
    
    // Normal behavior is random pick
    int ndx = glm::linearRand( 0, lakeFish.Size()-1 );

    // If they have shiny lure, they get the most valuable fish
    int hasLure = countTackle("shiny_lure");
    if (hasLure>0) {
        for (int i=0; i < lakeFish.Size(); i++) {
            if (lakeFish[i].fishPoints > lakeFish[ndx].fishPoints) {
                ndx = i;
            }
        }
    }

    // Get the fish
    Card result = lakeFish[ndx];
    lakeFish.EraseSwapBack(ndx);
    return result;
}

void CardFishGame::UpdateLineTension()
{
    reelTension -= 1.0;
}

int CardFishGame::countTackle( Oryol::String cardId )
{
    int count = 0;
    for (int i=0; i < tackleCards.Size(); i++) {
        if (tackleCards[i].cardId == cardId) {
            count++;
        }
    }
    return count;
}

int CardFishGame::calcReelPower( Card &card )
{
    int multiplier = 1;
    
    // Gearbox
    int numGearbox = countTackle("gearbox");
    for (int i=0; i< numGearbox; i++) {
        multiplier *= 2;
    }
    
    return card.reelPow * multiplier;
}

