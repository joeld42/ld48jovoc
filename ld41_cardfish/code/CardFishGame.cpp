#include "glm/gtc/random.hpp"

#include "CardFishGame.h"


void CardFishGame::resetRound( int roundNum )
{
    // Build lake deck (TODO per-level)
    lakeDeck.Clear();
    addCardsToDeck(lakeDeck, "smol_bass", 5 );
    addCardsToDeck(lakeDeck, "lake_fish", 5 );
    
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
    Card card;
    card.cardId = cardId;
    card.cardType = cardType;
    card.title = title;
    //flavorText  // Don't need these in here, used by mk_cards
    //artwork;
    //actionText
    card.reelPow = reelPow;
    card.fishPoints = fishPoints;
    card.cardFaceIndex = cardDefs.Size();
    
    cardDefs.Add( card );
}

void CardFishGame::SetupCardDefs()
{
    AddCardDef( "cast", "Cast", "castart1.png",
               "A basic cast can still be the most effective.",
               "Hook a fish at random from the lake!",
               CardType_CAST, /*reelPow*/ 3, /*fishPoints*/ 3, /*depth*/ 0 );
    
    AddCardDef( "smol_bass", "Smol Bass", "fishart1.png",
               "A diminuative bass. Not a lot of meat on it but still tasty.",
               "",
               CardType_FISH, /*reelPow*/ 1, /*fishPoints*/ 5, /*depth*/ 8 );

    AddCardDef( "lake_fish", "Lake Fish", "fishart1.png",
                "A common lake fish.",
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
    
    AddCardDef( "shiny_lure", "Shiny Lure", "tackleart1.png",
               "Fish can be easily distracted by shiny objects.",
               "Casts hook the highest valued fish in the lake.",
               CardType_TACKLE, /*reelPow*/ 4, /*fishPoints*/ 0, /*depth*/ 0 );
    

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
    playerDeck.Clear();
    tackleCards.Clear();
    addCardsToDeck(playerDeck, "cast", 5 );
    addCardsToDeck(playerDeck, "gloves", 2 );
    addCardsToDeck(playerDeck, "high_test", 2 );
    addCardsToDeck(playerDeck, "shiny_lure", 1 );
    
    resetRound(0);
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
    int ndx = glm::linearRand( 0, lakeFish.Size()-1 );
    
    Card result = lakeFish[ndx];
    lakeFish.EraseSwapBack(ndx);
    return result;
}

void CardFishGame::UpdateLineTension()
{
    reelTension -= 1.0;
}


