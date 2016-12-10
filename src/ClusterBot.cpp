#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <iomanip>

#include "hlt.hpp"
#include "networking.hpp"
// #include "log.h"

using namespace std;

ofstream out;

int threshold = 20;
const float STILL_MODIFIER = 7.5;
const unsigned int frameMod = 20;


// cluster struct:
struct cluster {
  unsigned int id;
  unsigned int strength;
  float strengthToSize;
  unsigned int size;
  unsigned short int minX;
  unsigned short int maxX;
  unsigned short int minY;
  unsigned short int maxY;
  unsigned short int centerX;
  unsigned short int centerY;
};

struct location {
  unsigned short int x;
  unsigned short int y;
};

// void floydWarshall(unsigned int ** dist, unsigned int ** next, const unsigned int size);
void clustering(hlt::GameMap& map, vector<cluster> & clusters);
int wrap(int l, int size);
void resetClusters(unsigned int** clustering, unsigned int width, unsigned int height);
bool compareByStrength(const cluster& a, const cluster& b) {
  return ( a.strengthToSize < b.strengthToSize);
}
// bool shouldAttackCluster(const vector<cluster>& clusters, hlt::site& cur, unsigned int **clustering, unsigned int x, unsigned int y);


int main() {
  // Initialization:
  out.open("cluster.txt", std::ios_base::out);
  srand(time(NULL));
  // out << "LOG FILE:" << endl;

  std::cout.sync_with_stdio(0);

  unsigned char myID;
  hlt::GameMap map;
  getInit(myID, map);
  sendInit("Cluster-bot");
  out << map.width << " " << map.height << endl;
  /* END INITIALIZATION */

  // All Pairs Shortest Paths Distances Matrix:
  unsigned int size = map.width * map.height;
  // unsigned int** distances;
  // distances = new unsigned int*[size];
  // for ( unsigned int i = 0; i < size; i++ ) {
  //   distances[i] = new unsigned int[size];
  // }

  std::set<hlt::Move> final_moves;

  // log(tempStr.c_str());
  //Clustering map:
  unsigned int** clustering = new unsigned int*[map.height];
  for ( unsigned int i = 0; i < map.height; i++ ) {
    clustering[i] = new unsigned int[map.width];
  }

  // Cluster list:
  vector<cluster> clusters;
  std::queue<location> m_queue;

  bool moved = false;

  location neighbor_right;
  location neighbor_left;
  location neighbor_above;
  location neighbor_below;

  unsigned int frame = 0;

  while(true) {
    std::queue<location> empty;
    std::swap(m_queue,empty);
    unsigned int clusterID = 0;

    getFrame(map);

    if ( frame % frameMod == 0 ) {
      resetClusters(clustering, map.width, map.height);
      // compute max and min strength
      unsigned int min = 500; unsigned int max = 0;
      for ( unsigned short y = 0; y < map.height; y++ ) {
        for ( unsigned short x = 0; x < map.width; x++ ) {
          hlt::Site& s = map.getSite({x,y});
          unsigned int t_strength = s.strength;
          if ( t_strength > max ) max = t_strength;
          if ( t_strength < min ) min = t_strength;
          clustering[y][x] = 0;
        }
      }
      clusters.clear();
      threshold = (max - min) / 20;
    }

    final_moves.clear();



    // Find clusters:
    bool done = false;
    bool stop = false;
    int count = 0;
    unsigned short initY = 0; unsigned short initX = 0;
    unsigned short minX = 100; unsigned short maxX = 0;
    unsigned short minY = 100; unsigned short maxY = 0;

    // If I find clusters with no wrap around, compute the center points, then cluster again I will
    // have different clusters. What I could attempt to do is a merge. Cluster based on no wrap around,
    // then try to merge the clusters that are on the edges. If they should be merged then I would try
    // to manually calculate the new center points
    if ( frame % frameMod == 0 ) {
      while ( !done ) {
        unsigned int clusterCount = 0;
        stop = false;
        // find un-clustered cell:
        // remember where we stopped
        for ( unsigned short y = 0; y < map.height && !stop; y++ ) {
          for ( unsigned short x = 0; x < map.width; x++ ) {
            hlt::Site& site = map.getSite({x, y});
            if ( clustering[y][x] == 0 && site.owner != myID ) {
              clusterID++;
              // out << "CLUSTER ID: " << clusterID << endl;
              // add this one to queue.
              location newLoc;
              newLoc.x = x;
              newLoc.y = y;
              initX = x; initY = y;
              m_queue.push(newLoc);
              clustering[y][x] = clusterID;
              clusterCount++;
              count++;
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
        hlt::Site& site = map.getSite({cur.x,cur.y});
        unsigned int curStr = site.strength;

        // add to clusters:
        cluster newCluster;
        newCluster.id = clusterID;
        newCluster.strength = curStr;
        if ( cur.x < minX ) minX = cur.x;
        if ( cur.x > maxX ) maxX = cur.x;
        if ( cur.y < minY ) minY = cur.y;
        if ( cur.y > maxY ) maxY = cur.y;

        // Add neighbors of first originator to m_queue
        neighbor_right.x = wrap(cur.x + 1, map.width);   neighbor_right.y = wrap(cur.y, map.height);
        neighbor_left.x = wrap(cur.x - 1, map.width);    neighbor_left.y = wrap(cur.y, map.height);
        neighbor_above.x = wrap(cur.x, map.width);       neighbor_above.y = wrap(cur.y - 1, map.height);
        neighbor_below.x = wrap(cur.x, map.width);       neighbor_below.y = wrap(cur.y + 1, map.height);

        if ( cur.x + 1 < map.width )
          m_queue.push(neighbor_right);
        if ( cur.x - 1 >= 0 )
          m_queue.push(neighbor_left);
        if ( cur.y - 1 >= 0 )
          m_queue.push(neighbor_above);
        if ( cur.y + 1 < map.height)
          m_queue.push(neighbor_below);

        // dequeue this guy.
        m_queue.pop();

        while ( !m_queue.empty()) {
          location& cur = m_queue.front();

          hlt::Site& site = map.getSite({cur.x, cur.y});
          if ( clustering[cur.y][cur.x] == 0 && site.owner != myID &&
            (site.strength <= curStr + threshold && site.strength >= curStr - threshold ) )
          {
            // out << "Adding " << cur.x << ", " << cur.y << " to the cluster." << endl;
            // if so, add it to the clustering matrix
            // and add neighbors to queue:
            clustering[cur.y][cur.x] = clusterID;
            newCluster.strength += site.strength;
            neighbor_right.x = wrap(cur.x + 1, map.width);   neighbor_right.y = wrap(cur.y, map.height);
            neighbor_left.x = wrap(cur.x - 1, map.width);    neighbor_left.y = wrap(cur.y, map.height);
            neighbor_above.x = wrap(cur.x, map.width);       neighbor_above.y = wrap(cur.y - 1, map.height);
            neighbor_below.x = wrap(cur.x, map.width);       neighbor_below.y = wrap(cur.y + 1, map.height);
            if ( cur.x + 1 < map.width )
              m_queue.push(neighbor_right);
            if ( cur.x - 1 >= 0 )
              m_queue.push(neighbor_left);
            if ( cur.y - 1 >= 0 )
              m_queue.push(neighbor_above);
            if ( cur.y + 1 < map.height)
              m_queue.push(neighbor_below);

            if ( cur.x < minX ) minX = cur.x;
            if ( cur.x > maxX ) maxX = cur.x;
            if ( cur.y < minY ) minY = cur.y;
            if ( cur.y > maxY ) maxY = cur.y;

            clusterCount++;
            count++;
          }
          m_queue.pop();
        }
        newCluster.size = clusterCount;
        newCluster.strengthToSize = newCluster.strength / clusterCount;
        newCluster.minX = minX; newCluster.maxX = maxX;
        newCluster.minY = minY; newCluster.maxY = maxY;
        newCluster.centerX = maxX - (int)((maxX - minX) * 0.5);
        newCluster.centerY = maxY - (int)((maxY - minY) * 0.5);
        clusters.push_back(newCluster);
      }

      // sort clusters:
      std::sort(clusters.begin(), clusters.end(), compareByStrength);
      unsigned short attackID = clusters[0].id;

      // DEBUG CLUSTERS:
      for(unsigned short a = 0; a < map.height; a++) {
        for(unsigned short b = 0; b < map.width; b++) {
          out << setw(4) << clustering[a][b];
        }
        out << endl;
      }
      out << endl;
    }

    // compute moves:
    for(unsigned short a = 0; a < map.height; a++) {
      for(unsigned short b = 0; b < map.width; b++) {
        hlt::Site& site = map.getSite({ b, a });
        if ( site.owner == myID ) {
          if ( site.strength < site.production * STILL_MODIFIER) {
            final_moves.insert({ { b, a }, STILL });
          }
          else {
            bool moved = false; // check if we can be greedy
            for ( int i = 0; i < 4; i++ ) {
              hlt::Site& adj = map.getSite({ b,a }, CARDINALS[i]);
              if ( adj.owner != myID && adj.strength < site.strength ) {
                final_moves.insert({ {b,a }, (unsigned char)CARDINALS[i] });
                moved = true;
                break;
              }
            }

            if ( !moved ) { // move towards a cluster
              hlt::Site& centre = map.getSite({clusters[0].centerX, clusters[0].centerY});

              bool movedForReal = false;
              // This isn't working. What we need to do instead is to move out from the
              // center of the cluster and take over all of the cluster.
              if ( clustering[b][a] == clusters[0].id ) {
                final_moves.insert({ { b,a }, (unsigned char)(rand() % 4)} );
              }
              else if ( clustering[b][a] != clusters[0].id || !movedForReal){
                if ( clusters[0].centerY < a ) {
                  final_moves.insert({ { b, a }, NORTH });
                  movedForReal = true;
                }
                else if ( clusters[0].centerY > a ) {
                  final_moves.insert({ { b, a }, SOUTH });
                  movedForReal = true;
                }
                else if ( clusters[0].centerX < b ) {
                  final_moves.insert({ { b, a }, WEST });
                  movedForReal = true;
                }
                else if ( clusters[0].centerX > b ) {
                  final_moves.insert({ { b, a }, EAST });
                  movedForReal = true;
                }
              }
            }
          }
        }
      }
    }

    sendFrame(final_moves);
  }

  out.close();
  return 0;
}

// bool shouldAttackCluster(const vector<cluster>& clusters, hlt::site& cur, unsigned int **clustering, unsigned int x, unsigned int y) {
//     unsigned int id = clusters[i].id;
//     for ( unsigned int y = 0; y < height; y++ ) {
//       for ( unsigned int x = 0; x < width; x++ ) {
//         // find the closest thing to this
//         if ( )
//       }
//     }
//
//
// }

void resetClusters(unsigned int** clustering, unsigned int width, unsigned int height) {
  for ( unsigned short y = 0; y < height; y++ ) {
    for ( unsigned short x = 0; x < width; x++ ) {
      clustering[y][x] = 0;
    }
  }
}

int wrap(int l, int size) {
  size = size - 1;
  int ans;
  if ( l < 0 )
    ans = size - (size % (l * -1)); // modulus here
  else if ( l >= size )
    ans = ((size+1) % l);
  else
    ans = l;

  // out << "Wrapping: " << l << " to " << ans << endl;
  return ans;
}
