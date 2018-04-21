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
}

void CardFishGame::resetGame()
{
    // Initial player deck
    playerDeck.Clear();
    for (int i=0; i < 10; i++) {
        Card card = {};
        card.cardType = CardType_TACKLE;
        card.title = "Tackle Card";
        card.cardId = "card0";
        card.reelPow = 2;
        card.fishPoints = 1;
        
        playerDeck.Add(card);
    }
    
    resetRound(0);
}


