A simple brick break game that I will compile to emscripten

Game Data:

struct position{x, y}
struct vector2d{x, y}

Game engine:

Main Loop:
  
  Input Loop:
    a/d or <-/->for player movement
    esc to pause game

  Logic:
    Ball movement: position + speed

    collision:
      bottom screen: End game
      screen edge: bounce ball
      brick: Bounce based on normal
      paddle:  Ball Y inside paddle, hit center -> gain speed. 
               Hit off center -> slow X and increase Y;

    Player:
      collide with screen

    Bricks: 
      delete function to remove
      2D array to constuct all blocks.
      delete when ball touches
      _________
      __XXXXX__
      __XXXXX__
      _________
      _________

  Drawing:
    Paddle

    Ball

    Bricks

    Score


note:
DELETE LATER https://web.dev/drawing-to-canvas-in-emscripten/ 
