#ifndef _RULES_H_
#define _RULES_H_

#include <string>
#include <vector>

using namespace std;
class ManaCost;
class Player;
class MTGPlayerCards;
class MTGDeck;
class MTGCardInstance;

#define MAX_RULES_CARDS 4096;

class RulesPlayerZone{
 public:
  vector<int> cards;
  void add(int cardid);
  RulesPlayerZone();
  void cleanup();
};

class RulesPlayerData{
 public:
  vector <string> extraRules;
  int life;
  int poisonCount;
  int damageCount;
  int preventable;
  string avatar;
  ManaCost * manapool;
  RulesPlayerZone zones[5];
  RulesPlayerData();
  ~RulesPlayerData();
  void cleanup();

};

class RulesState{
 public:
  int phase;
  int player;
  void parsePlayerState(int playerId, string s);
  RulesState();
  RulesPlayerData playerData[2];
  void cleanup();
};


class Rules{
protected:
  Player * loadPlayerMomir(int isAI);
  Player * loadPlayerRandom(int isAI, int mode);
  Player * initPlayer(int playerId);
  MTGDeck * buildDeck( int playerId);
  int strToGameMode(string s);
public:
  enum {
    PARSE_UNDEFINED,
    PARSE_INIT,
    PARSE_PLAYER1,
    PARSE_PLAYER2,
    PARSE_PLAYERS
  };

  string bg;

  Rules(string filename, string bg = "");
  int load(string filename);
  int gamemode;
  void initPlayers();
  void addExtraRules();
  void initGame();
  void cleanup();
  vector <string> extraRules;
  RulesState initState;
  static int getMTGId(string name);
  static MTGCardInstance * getCardByMTGId(int mtgid);

};

#endif