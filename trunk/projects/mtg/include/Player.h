#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "JGE.h"
#include "MTGGameZones.h"
#include "Damage.h"
#include "Targetable.h"

class MTGDeck;
class MTGPlayerCards;
class MTGInPlay;
class ManaPool;

class Player: public Damageable
{
protected:
    ManaPool * manaPool;

public:
    enum ENUM_PLAY_MODE
    {
        MODE_TEST_SUITE, MODE_HUMAN, MODE_AI,
    };

    JTexture * mAvatarTex;
    JQuad * mAvatar;
    int playMode;
    int canPutLandsIntoPlay;
    int nomaxhandsize;
    int castedspellsthisturn;
    int onlyonecast;
    int castcount;
    int nocreatureinstant;
    int nospellinstant;
    int onlyoneinstant;
    int castrestrictedcreature;
    int castrestrictedspell;
    MTGPlayerCards * game;
    string deckFile;
    string deckFileSmall;
    string deckName;

    Player(MTGDeck * deck, string deckFile, string deckFileSmall);
    virtual ~Player();

    virtual void End();
    virtual int displayStack()
    {
        return 1;
    }
    const string getDisplayName() const;
    int typeAsTarget()
    {
        return TARGET_PLAYER;
    }

    int afterDamage();
    int poisoned();
    int damaged();
    int prevented();
    void unTapPhase();
    MTGInPlay * inPlay();
    ManaPool * getManaPool();
    void cleanupPhase();
    virtual int Act(float dt)
    {
        return 0;
    }


    virtual int isAI()
    {
        return 0;
    }

    Player * opponent();
    int getId();
    JQuad * getIcon();

    virtual int receiveEvent(WEvent * event)
    {
        return 0;
    }

    virtual void Render()
    {
    }

    void loadAvatar(string file);
};

class HumanPlayer: public Player
{
public:
    HumanPlayer(MTGDeck * deck, string deckFile, string deckFileSmall);
    HumanPlayer(string deckFile);

};

ostream& operator<<(ostream&, const Player&);

#endif