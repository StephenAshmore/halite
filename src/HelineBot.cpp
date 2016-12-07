#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <string>
#include <vector>
#include <queue>          // std::queue


#include "hlt.hpp"
#include "networking.hpp"
#include "log.h"

const float STILL_MODIFIER = 10;

using namespace std;

// cluster struct:
struct cluster {
  unsigned int id;
  unsigned int strength;

};

struct location {
  unsigned int x;
  unsigned int y;
}

void floyd-warshall(unsigned int ** dist, unsigned int ** next, const unsigned int size);
void clustering(hlt::GameMap& map, vector<cluster> & clusters);
int wrap(int l, int size);
void resetClusters(unsigned int** clustering);

int main() {
  // Initialization:
  srand(time(NULL));

  std::cout.sync_with_stdio(0);

  unsigned char myID;
  hlt::GameMap presentMap;
  getInit(myID, presentMap);
  sendInit("heline-bot");
  // end init.

  // All Pairs Shortest Paths Distances Matrix:
  unsigned int size = presentMap.width * presentMap.height;
  unsigned int** distances;
  distances = new unsigned int*[size];
  for ( unsigned int i = 0; i < size; i++ ) {
    distances[i] = new unsigned int[size];
  }

  std::set<hlt::Move> final_moves;
  std::string tempStr = "map height: ";
  tempStr += presentMap.height + " width: " + presentMap.width;
  log(tempStr.c_str());
  //Clustering map:
  unsigned int** clustering = new unsigned int*[presentMap.height];
  for ( unsigned int i = 0; i < presentMap.height; i++ ) {
    clustering[i] = new unsigned int[presentMap.width];
  }

  // Cluster list:
  vector<cluster> clusters;
  std::queue<location> m_queue;

  bool moved = false;

  location neighbor_right;
  location neighbor_left;
  location neighbor_above;
  location neighbor_below;

  unsigned int clusterID = 1;
  while(true) {
    resetClusters(clustering);
    final_moves.clear();

    getFrame(presentMap);

    // All pairs shortest paths:

    // Find clusters:
    // clustering(presentMap, clusters);
    bool done = false;
    bool stop = false;
    while ( !done ) {
      stop = false;
      // find un-clustered cell:
      for ( unsigned short y = 0; y < map.height && !stop; y++ ) {
        for ( unsigned short x = 0; x < map.width; x++ ) {
          if ( clustering[y][x] == 0 ) {
            // add this one to queue.
            location newLoc;
            newLoc.x = x;
            newLoc.y = y;
            m_queue.push(newLoc);
            clustering[y][x] = clusterID;
            clusterID++;
            stop = true;
            break;
          }
        }
      }

      if ( !stop ) {
        done = true;
        break;
      }

      // go through queue:
      location& cur = m_queue.front();
      hlt::Site& site = presentMap.getSite({cur.x,cur.y});
      unsigned int curStr = site.strength;

      // Add neighbors of first originator to m_queue.
      // dequee this guy.
      neighbor_right.x = wrap(cur.x + 1);   neighbor_right.y = wrap(cur.y);
      neighbor_left.x = wrap(cur.x - 1);    neighbor_left.y = wrap(cur.y);
      neighbor_above.x = wrap(cur.x);       neighbor_above.y = wrap(cur.y - 1);
      neighbor_below.x = wrap(cur.x);       neighbor_below.y = wrap(cur.y + 1);
      m_queue.push(neighbor_right); m_queue.push(neighbor_left);
      m_queue.push(neighbor_above); m_queue.push(neighbor_below);
      m_queue.pop();

      while ( !m_queue.empty()) {
        location& cur = m_queue.front();

        hlt::Site& site = presentMap.getSite({cur.x, cur.y});
        if ( site.strength <= curStr + 1 && site.strength >= curstr - 1) {}
          // check if it is +- 1 of curStr.
          // if so, add it to the clustering matrix
          // and add neighbors to queue:
          clustering[cur.y][cur.x] = clusterID;
          neighbor_right.x = wrap(cur.x + 1);   neighbor_right.y = wrap(cur.y);
          neighbor_left.x = wrap(cur.x - 1);    neighbor_left.y = wrap(cur.y);
          neighbor_above.x = wrap(cur.x);       neighbor_above.y = wrap(cur.y - 1);
          neighbor_below.x = wrap(cur.x);       neighbor_below.y = wrap(cur.y + 1);
          m_queue.push(neighbor_right); m_queue.push(neighbor_left);
          m_queue.push(neighbor_above); m_queue.push(neighbor_below);
        }
        // remove this element.
        m_queue.pop();
      }
    }

    // Compute cluster data.

    for(unsigned short a = 0; a < presentMap.height; a++) {
      for(unsigned short b = 0; b < presentMap.width; b++) {
        hlt::Site& site = presentMap.getSite({ b, a });

      }
    }

    sendFrame(final_moves);
  }

  return 0;
}

void resetClusters(unsigned int** clustering, unsigned int width, unsigned int height) {
  for ( unsigned short y = 0; y < height; y++ ) {
    for ( unsigned short x = 0; x < width; x++ ) {
      clustering[y][x] = 0;
    }
  }
}

int wrap(int l, int size) {
  if ( l < 0 )
    return size - l;
  else if ( l > size )
    return l - size;
  else
    return l;
}

// void clustering(hlt::GameMap& map, vector<cluster> & clusters) {
//   bool inCluster = false;
//   for ( unsigned short y = 0; y < map.height; y++ ) {
//     for ( unsigned short x = 0; x < map.width; x++ ) {
//       hlt::Site& site = map.getSite({x,y});
//       if ( )
//     }
//   }
// }
// always a square graph
void floyd-warshall(unsigned int ** dist, unsigned int ** map, const unsigned int size) {
  // The Wikipedia article is horrible. Besides mis-representing (in my opinion) the canonical form of the Floyd–Warshall algorithm, it presents a buggy pseudocode.
  // It’s much easier (and more direct) not to iterate over indices but over vertices. Furthermore, each predecessor (usually denoted π, not next), needs to point to its, well, predecessor, not the current temporary vertex.
  // With that in mind, we can fix their broken pseudocode.

  for ( unsigned int i = 0; i < size; i++ ){
    for ( unsigned int j = 0; j < size; j++ ) {
      dist[i][j] = 0;
    }
  }

  // for each vertex i
  //     for each vertex j
  //         if w(u,v) = ∞ then
  //             next[i][j] ← NIL
  //         else
  //             next[i][j] ← i


                // for each edge (u,v)
                    // dist[u][v] ← w(u,v)  // the weight of the edge (u,v)

  for ( unsigned int i = 0; i < size; i++ ) {
    for ( unsigned int j = 0; j < size; j++ ) {
      if ( ) {

      }
    }
  }


  for ( unsigned int k = 0; k < size; k++ ) {
    for ( unsigned int i = 0; i < size; i++ ) {
      for ( unsigned int j = 0; j < size; j++ ) {
        if ( dist[i][k] + dist[k][j] < dist[i][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          next[i][j] = next[k][j];
        }
      }
    }
  }

}
