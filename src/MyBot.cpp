#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>

#include "hlt.hpp"
#include "networking.hpp"
// #include "log.h"

const float STILL_MODIFIER = 7.5;

int main() {
  srand(time(NULL));

  std::cout.sync_with_stdio(0);

  unsigned char myID;
  hlt::GameMap presentMap;
  getInit(myID, presentMap);
  sendInit("betterRandom-bot");

  std::set<hlt::Move> moves;
  bool moved = false;
  while(true) {
    moves.clear();

    getFrame(presentMap);

    for(unsigned short a = 0; a < presentMap.height; a++) {
      for(unsigned short b = 0; b < presentMap.width; b++) {
        hlt::Site& site = presentMap.getSite({ b, a });
        if (site.owner == myID) {
          moved = false;
          // check if an adjacent enemy has less strength, and move there if so.
          for ( int i = 0; i < 4; i++ ) {
            hlt::Site& adj = presentMap.getSite({ b,a }, CARDINALS[i]);
            if ( adj.owner != myID && adj.strength < site.strength ) {
              moves.insert({ {b,a }, (unsigned char)CARDINALS[i] });
              moved = true;
              break;
            }
          }

          if ( !moved && site.strength < site.production * STILL_MODIFIER) {
            moves.insert({ { b, a }, STILL });
          }
          else if ( !moved ) {
            moves.insert({ { b, a }, (unsigned char)(rand() % 5) });
          }
        }
      }
    }

    sendFrame(moves);
  }

  return 0;
}
