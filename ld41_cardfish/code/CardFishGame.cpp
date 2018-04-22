#include "CardFishGame.h"

void CardFishGame::resetRound( int roundNum )
{
    // Build lake deck (TODO per-level)
    lakeDeck.Clear();
    for (int i=0; i < 10; i++) {
        Card card;
        card.cardType = CardType_FISH;
        card.title = "Fish Card";
        card.cardId = "card1";
        card.reelPow = 2;
        card.fishPoints = 1;
        
        playerDeck.Add(card);
    }
    
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
    const char *cardnames[] = {
        "card0", "card1", "card2", "card3", "card4",
    };
    // Initial player deck
    playerDeck.Clear();
    for (int i=0; i < 10; i++) {
        Card card = {};
        card.cardType = CardType_TACKLE;
        card.title = "Tackle Card";
        card.cardId = cardnames[i % 5];
        card.reelPow = 2;
        card.fishPoints = 1;
        
        playerDeck.Add(card);
    }
    
    resetRound(0);
}


