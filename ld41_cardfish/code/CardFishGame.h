#ifndef CARDFISH_GAME_H
#define CARDFISH_GAME_H

#include "SceneObject.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"

enum CardType {
    CardType_CAST,
    CardType_FISH,
    CardType_TACKLE,
};

struct Card {
    CardType cardType;
    Oryol::String title;
    Oryol::String cardId;
    int reelPow;
    int fishPoints; // Victory Points
    int depth;
    int cardFaceIndex;
    Tapnik::SceneObject *sceneObj; // May be NULL if this card isn't drawn yet
    
    // For interfacey stuff
    float seed;
    float drawAnimTimer;
    glm::vec3 lakePos; // for lake fish
};

struct CardFishGame {
    
    // 1 copy of each card
    Oryol::Array<Card> cardDefs;
    
    Oryol::Array<Card> playerDeck;
    Oryol::Array<Card> lakeDeck;
    
    // Gameplay
    Oryol::Array<Card> deck;
    Oryol::Array<Card> lakeFish;
    
    bool gameStarted;
    bool gameOver;
    
    void SetupCardDefs();
    void shuffle( Oryol::Array<Card> &cards );
    void addCardsToDeck( Oryol::Array<Card> &cards, Oryol::String cardId, int howMany );
    
    void AddCardDef  ( Oryol::String cardId,
                        Oryol::String title,
                        Oryol::String artwork,
                        Oryol::String flavorText,
                        Oryol::String actionText,
                        CardType cardType,
                        int reelPow,
                        int fishPoints,
                        int depth );

    
    // Tackle cards in play
    Oryol::Array<Card> tackleCards;
    
    void PlayTackleCard( Card card );
    void PlayCardToLake( Card card );
    void UpdateLineTension();
    Card castLakeCard();
    
    int fishPoints;
    int reelDistance;
    int reelTension;
    int reelTensionMax;
    int reelSlackMin;
    Card reelCard;
    
    void resetGame();
    void resetRound( int roundNum );
    
};

#endif
