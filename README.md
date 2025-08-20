# AsteroidsBot

<div align="center">
Autonomous asteroid dodging machine!

![Short gif of bot playing the game](Media/AsteroidBotGif.gif)
</div>

## Motivation

On top of a simple implementation of Asteroids written in Python using the PyGame library, I have created a bot that dodges asteroids!  I have found that my favorite subjects to study have consistently revolved around real-time decision-making since it combines mathematical modeling and high-performance computing toward a goal of automating a complex task.  What especially excites me about projects in automated decision-making such as this is that with a robust implementation, the results will often far surpass the capabilities of a human performing the same task.

After learning complex decision-making techniques in a numerical option pricing class at the University of Chicago, I have been eager to generalize what I learned and to apply it in other scenarios which allow for more tangible results.  Extending the techniques to play a video game in real time was a good next step!

## The Bot

The bot makes decisions by discretizing the next few seconds into small $\Delta t$, considering what the possible outcomes would be if it were to perform each possible action (in this case either accelerating, rotating left, or rotating right) over that small chunk of time.  

In the process of considering these paths, the bot essentially creates a tree of possibilities.  The current state (time $t = 0$) is the root node of the tree.  The next layer of the tree represents the possible states of the game at time $t = \Delta t$.  This first layer has 3 nodes in this case, one representing the resulting state for each of the 3 possible actions that could be taken from time $t=0$ to time $t=\Delta t$.  The second layer at time $t=2\Delta t$ has 9 nodes, the third layer has 27 nodes, and so on.  This is how the tree of possible trajectories is created!

The bot then values the actions that it could take over the next $\Delta t$ by calculating which of the three next subtrees (the trees starting from $t=\Delta t$) contain the trajectory which keeps them furthest from a collision across time.  Specifically, it calculates which subtree has the largest minimum distance to collision, and then deems that advancing to the root node of this subtree is the optimal path to take over the next $\Delta t$!  Of course, since the asteroids are also moving, the bot also calculates where they will be located at each time, allowing it to calculate the distance to each asteroid at each forward point in time along the tree.  Since the asteroids move with constant velocity, this is trivial.

Hence, by discretizing the decision space into small chunks of time, the bot is able to evaluate the different possible trajectories that it can take, enabling it to select the next best action after each decision.

## Optimizations

First, I implemented a working version of the decision making logic in Python.  With the fast-paced nature of Asteroids, however, this did not allow for adequate performance since the code was just too slow.  Calculating a single decision with a 1 second look ahead and a $\Delta t$ of 0.1 seconds took 16 seconds of runtime with the Python implementation.  The code was far too slow for three main reasons.

  1) In the creation of the tree of possible trajectories, the Python implementation would allocate memory for each node during the calculation of the tree.  Since the tree grows exponentially in size, this node-by-node allocation greatly weighed down the program.
  2) The initial implementation would exhaustively compute the possible trajectories, continuing down each path even after a collision.
  3) Using an interpreted language with dynamic typing (Python) significantly slowed down the code (as it normally would).  Especially since the tree is constructed recursively, the Python implementation has significant overhead whereas in C++, optimizations can be made at compile time to unroll some of these instructions, saving us lots of grief at runtime.

By simply implementing the bot in C++ with careful memory management and tree pruning, I greatly improved the runtime.  Specifically, I created a memory pool for tree nodes, allowing us to avoid expensive memory allocations on the critical path of the bot.  I also improved the tree creation logic such that the tree would not further investigate a trajectory if a collision was detected.

After these optimizations, the code ran over 1000 times faster, with a 1 second look ahead and a $\Delta t$ of 0.1 seconds taking just about 13 milliseconds of runtime!  With the new performance boost, the bot is more than fast enough to make decisions in real time!

## Final Implementation

With the decision-making code fast enough to play the game in real-time, I wired the bot up to interact with the game.  With the game running on one process and the bot running on another, a TCP connection was used to establish communication between the two over a socket.  The game in Python sends the current state of the ship and the asteroids to the C++ bot which receives it, parses it, makes a decision, and sends back a character-representation of a key-stroke to the game.  The result is an asteroid dodging beast!


