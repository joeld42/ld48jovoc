#ifndef CARDFISH_GAME_H
#define CARDFISH_GAME_H

#include "SceneObject.h"

enum CardType {
    CardType_FISH,
    CardType_TACKLE,
};

struct Card {
    CardType cardType;
    Oryol::String title;
    Oryol::String cardId;
    int reelPow;
    int fishPoints; // Victory Points
    int cardFaceIndex;
    Tapnik::SceneObject *sceneObj; // May be NULL if this card isn't drawn yet
    
    // For interfacey stuff
    float drawAnimTimer;
};

struct CardFishGame {
    
    Oryol::Array<Card> playerDeck;
    Oryol::Array<Card> lakeDeck;
    
    Oryol::Array<Card> deck;
    
    void shuffle( Oryol::Array<Card> &cards );
    
    // Tackle cards in play
    Oryol::Array<Card> tackleCards;
    
    int reelDistance;
    int reelTension;
    int reelTensionMax;
    int reelSlackMin;
    Card reelCard;
    
    void resetGame();
    void resetRound( int roundNum );
    
};

#endif
