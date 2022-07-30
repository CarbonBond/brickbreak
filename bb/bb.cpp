#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void ballCollision(double);
void paddleMovement(double, const Uint8 *);
void winCondition(double, const Uint8 *);
void playGame(const Uint8 *);

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 600;

SDL_Window *window;
SDL_Renderer *renderer;

struct vector2d {
  float x;
  float y;
};

vector2d normalize(vector2d vector) {
  float magnitude = std::sqrt(vector.x * vector.x + vector.y * vector.y);
  vector2d v = vector;
  if (magnitude > 0) {
    v = {vector.x / magnitude, vector.y / magnitude};
  }
  return v;
}

struct posistion {
  float x;
  float y;
};

struct score {
  int left;
  int right;
};

score score = {0, 0};

struct ball {
  posistion pos;
  int radius;
  vector2d speed;
};

const int BALL_RADIUS = 8;
vector2d BALL_SPEED_VECTOR = {00., 400.};
const int BALL_SPEED = 500;

ball ball = {{WINDOW_WIDTH / 2., WINDOW_HEIGHT / 2.}, 7, BALL_SPEED_VECTOR};

struct brick {
  posistion pos;
  float width;
  float height;
  vector2d speed;
};

struct paddle {
  posistion pos;
  float width;
  float height;
  vector2d speed;
};

const int PADDLE_WIDTH = 70;
const int PADDLE_HEIGHT = 6;
vector2d PADDLE_SPEED = {350., 0.};
const int PADDLE_BOTTOM_SPACE = 60;
paddle paddle = {{WINDOW_WIDTH / 2. - PADDLE_WIDTH / 2.,
                  WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_BOTTOM_SPACE},
                 PADDLE_WIDTH,
                 PADDLE_HEIGHT,
                 PADDLE_SPEED};

int *map;

int level[9][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
                   {0, 0, 0, 0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
                   {0, 0, 0, 0, 0, 0, 0, 0, 0}};

void setupLevel(int *level) { map = level; }

enum gameState { delay, play, pause };
gameState gameState = pause;

void drawFont();
void redraw();

Uint64 ticksNow = SDL_GetTicks64();
Uint64 ticksLast = 0;
double deltaTime = 0;

bool handle_events() {
  SDL_Event event;
  SDL_PollEvent(&event);

  const Uint8 *keystate = SDL_GetKeyboardState(NULL);

  ticksNow = SDL_GetTicks64();
  deltaTime = (double)(ticksNow - ticksLast) / 1000;
  ticksLast = ticksNow;

  if (event.type == SDL_QUIT) {
    return false;
  }

  if (keystate[SDL_SCANCODE_ESCAPE]) {
    gameState = pause;
  }

  switch (gameState) {
  case delay:
    SDL_Delay(350);
    gameState = pause;
  case pause:
    playGame(keystate);
    break;
  case play:
    paddleMovement(deltaTime, keystate);
    ballCollision(deltaTime);
    ball.pos.x += ball.speed.x * deltaTime;
    ball.pos.y += ball.speed.y * deltaTime;
    winCondition(deltaTime, keystate);
    break;
  }

  redraw();
  SDL_Delay(1);
  return true;
}

void redraw() {

  TTF_Font *font = TTF_OpenFont("./RetroBlocky.ttf", 60);
  SDL_Color white = {255, 255, 255};
  {
    char text[2];
    sprintf(text, "%d", score.left);
    SDL_Surface *leftScore = TTF_RenderText_Solid(font, text, white);
    SDL_Texture *leftTexture =
        SDL_CreateTextureFromSurface(renderer, leftScore);
    SDL_FreeSurface(leftScore);
    SDL_Rect leftRect = {
        .x = WINDOW_HEIGHT / 6, .y = WINDOW_HEIGHT / 10, .w = 40, .h = 60};

    SDL_RenderCopy(renderer, leftTexture, NULL, &leftRect);
    SDL_DestroyTexture(leftTexture);
  }

  {
    char text[2];
    sprintf(text, "%d", score.right);
    SDL_Surface *leftScore = TTF_RenderText_Solid(font, text, white);
    SDL_Texture *leftTexture =
        SDL_CreateTextureFromSurface(renderer, leftScore);
    SDL_FreeSurface(leftScore);
    SDL_Rect leftRect = {.x = WINDOW_WIDTH - 40 - WINDOW_HEIGHT / 6,
                         .y = WINDOW_HEIGHT / 10,
                         .w = 40,
                         .h = 60};

    SDL_RenderCopy(renderer, leftTexture, NULL, &leftRect);
    SDL_DestroyTexture(leftTexture);
  }
  if (gameState == pause) {
    SDL_Surface *pauseMsg = TTF_RenderText_Solid(font, "move to start", white);
    SDL_Texture *texturePause =
        SDL_CreateTextureFromSurface(renderer, pauseMsg);
    SDL_FreeSurface(pauseMsg);
    SDL_Rect pauseRect = {.x = WINDOW_WIDTH / 4,
                          .y = WINDOW_HEIGHT / 6,
                          .w = WINDOW_WIDTH / 2,
                          .h = 60};
    SDL_RenderCopy(renderer, texturePause, NULL, &pauseRect);
    SDL_DestroyTexture(texturePause);
  }
  SDL_SetRenderDrawColor(renderer, /* RGBA: black */ 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, /* RGBA: white */ 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_FRect p = {.x = paddle.pos.x,
                 .y = paddle.pos.y,
                 .w = paddle.width,
                 .h = paddle.height};

  SDL_RenderFillRectF(renderer, &p);
  SDL_SetRenderDrawColor(renderer, /* RGBA: green */ 0x00, 0x80, 0x00, 0xFF);
  filledCircleRGBA(renderer, ball.pos.x, ball.pos.y, ball.radius,
                   /* RGBA: green */ 0x00, 0x80, 0x00, 0xFF);

  SDL_RenderPresent(renderer);
  TTF_CloseFont(font);
}

void playGame(const Uint8 *keystate) {
  if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_D] ||
      keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_RIGHT]) {
    gameState = play;
  }
}

void resetPositions() {
  paddle.pos.y = WINDOW_HEIGHT / 2. - paddle.height / 2;
  ball.pos.x = WINDOW_WIDTH / 2.;
  ball.pos.y = WINDOW_HEIGHT / 2.;
}

void winCondition(double deltaTime, const Uint8 *keystate) {}

void paddleMovement(double deltaTime, const Uint8 *keystate) {
  if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT]) {
    paddle.pos.x -= paddle.speed.x * deltaTime;
  }

  if ((keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT])) {
    paddle.pos.x += paddle.speed.x * deltaTime;
  }
}

void ballHitsPaddle(double deltaTime, struct paddle paddle) {

  if (ball.pos.x - ball.radius + (ball.speed.x * deltaTime) <
          paddle.width + paddle.pos.x && // right paddle, left ball
      ball.pos.x + ball.radius + (ball.speed.x * deltaTime) >
          paddle.pos.x && // left paddle, right ball.
      ball.pos.y + ball.radius + (ball.speed.y * deltaTime) >
          paddle.pos.y && // top paddle, bottom ball;
      ball.pos.y - ball.radius + (ball.speed.y * deltaTime) <
          paddle.height + paddle.pos.y) { // bottom paddle, top ball.

    posistion paddleCenter = {paddle.pos.x + paddle.width / 2,
                              paddle.pos.y + paddle.height / 2};
    vector2d hitDirection = {ball.pos.x - paddleCenter.x,
                             ball.pos.y - paddleCenter.y};

    hitDirection = normalize(hitDirection);

    // //Make paddles influense less strong by pushing it towards -y 0.
    // hitDirection.x += 0;
    // hitDirection.y -= 0.3;
    // hitDirection = normalize(hitDirection);

    //get the angle of approch and add it with the paddles influense. 
    vector2d approchAngle = {ball.speed.x , ball.speed.y *-1};
    approchAngle = normalize(approchAngle);
    hitDirection.x += approchAngle.x;
    hitDirection.y += approchAngle.y;

    //Get correct speed.
    hitDirection = normalize(hitDirection);
    hitDirection.x *= BALL_SPEED;
    hitDirection.y *= BALL_SPEED;

    ball.speed = hitDirection;
  }
}

void ballCollision(double deltaTime) {
  // Bounce of left and right of screen
  if (ball.pos.x + (ball.speed.x * deltaTime) + ball.radius > WINDOW_WIDTH ||
      ball.pos.x - ball.radius + (ball.speed.x * deltaTime)  < 0) {
    ball.speed.x *= -1;
  }
  // Bounce off top of screen
  if (ball.pos.y + (ball.speed.y * deltaTime)< 0) {
    ball.speed.y *= -1;
  }

  // Bounce off Bottom for testing.
  if (ball.pos.y > WINDOW_HEIGHT) {
    ball.speed.y *= -1;
  }

  ballHitsPaddle(deltaTime, paddle);
}

void run_main_loop() {
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]() { handle_events(); }, 0, true);
#else
  while (handle_events())
    ;
#endif
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window,
                              &renderer);

  redraw();
  run_main_loop();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}
