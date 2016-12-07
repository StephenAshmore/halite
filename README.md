# heline-bot
A bot for the halite competition.


## Strategy
Gather Strength on weak controlled tiles.  

Take advantage of weak tiles.  

Leverage nearby units to attack tiles.

Reinforce weakened borders first, and attacking units second.

## Implementation
Compute a set of possible moves that takes into account the entire board.
To account for the entire board, compute a matrix of boolean values that represent adjacent tiles.


- Flood fill to find clusters of similar strengthness.
  - Sum up clusters to determine how much strength is needed to conquer the cluster
- Dijkstra to minimum path to minimum clusters.
- Select closest tiles to attack the respective closest target.
