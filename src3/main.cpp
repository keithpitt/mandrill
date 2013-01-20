#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdint.h>

// Stolen from:
// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define DEBUG true
#define debug(fmt, ...) \
  if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt "\n", __FILE__, \
      __LINE__, __func__, __VA_ARGS__);

const int FPS    = 60;
const int WIDTH  = 1280;
const int HEIGHT = 720;

const int BLOCK  = 25;
const int TOP    = 0 + BLOCK;
const int BOTTOM = HEIGHT - BLOCK;

namespace Direction {
  enum type {
    UP, DOWN
  };
};

void drawDottedLine() {
  int blocks  = (BOTTOM - TOP + BLOCK) / (BLOCK * 2);
  int left    = WIDTH / 2 - BLOCK;

  for(int i = 0; i < blocks; i++) {
    int top = (TOP + BLOCK) + ((i * BLOCK) * 2);

    glBegin(GL_QUADS);
    glVertex2f(left, top);
    glVertex2f(left + BLOCK, top);
    glVertex2f(left + BLOCK, top + BLOCK);
    glVertex2f(left, top + BLOCK);
    glEnd();
  }
}

class Box {
  public:

  int x, y, w, h;

  void init(int xx, int yy, int ww = BLOCK, int hh = BLOCK*7) {
    x = xx;
    y = yy;
    w = ww;
    h = hh;
  }

  int left() {
    return x;
  }

  int top() {
    return y;
  }

  int bottom() {
    return y + h;
  }

  int right() {
    return x + w;
  }

  bool collision(Box box) {
    return !(box.left() > right()
        || box.right() < left()
        || box.top() > bottom()
        || box.bottom() < top());
  }

  void render() {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
  }
};

class Player {
  int upVelocity, downVelocity;
  SDL_Keycode upKey, downKey;

  public:

  int speed;
  Box box;

  void init(SDL_Keycode up, SDL_Keycode down) {
    upKey    = up;
    downKey  = down;

    upVelocity    = 0;
    downVelocity  = 0;

    speed = 10;
  }

  void update(Box top, Box bottom) {
    int yy = box.y + (upVelocity + downVelocity);
    int topY = top.y + top.h;
    int bottomY = bottom.y - box.h;

    if(yy <= topY)
      yy = topY;
    else if(yy >= bottomY)
      yy = bottomY;

    box.y = yy;
  }

  void render() {
    box.render();
  }

  void travel(Direction::type direction) {
    if(direction == Direction::DOWN) {
      downVelocity = speed;
    } else if (direction == Direction::UP) {
      upVelocity = speed * -1;
    }
  }

  void stop(Direction::type direction) {
    if(direction == Direction::DOWN) {
      downVelocity = 0;
    } else if (direction == Direction::UP) {
      upVelocity = 0;
    }
  }

  void keyDown(SDL_Keycode key) {
    if(key == downKey) {
      travel(Direction::DOWN);
    } else if (key == upKey) {
      travel(Direction::UP);
    }
  }

  void keyUp(SDL_Keycode key) {
    if(key == downKey) {
      stop(Direction::DOWN);
    } else if (key == upKey) {
      stop(Direction::UP);
    }
  }
};

class Ball {
  int speed;
  int xVelocity;
  int yVelocity;

  int startX;
  int startY;

  public:

  Box box;

  void init() {
    startX = WIDTH/2-BLOCK;
    startY = HEIGHT/2-BLOCK;
    speed = 6;
    xVelocity = speed;
    yVelocity = speed * -1;

    box.init(0, 0, BLOCK, BLOCK);
    reset();
  }

  void update(Player player1, Player player2, Box top, Box bottom, Box left, Box right) {
    if(box.collision(bottom)) {
      box.y = bottom.top() - box.h;
      yVelocity = yVelocity * -1;
    }

    if(box.collision(top)) {
      box.y = top.bottom();
      yVelocity = yVelocity * -1;
    }

    if(box.collision(player1.box)) {
      box.x = player1.box.right();
      xVelocity = xVelocity * -1;
    }

    if(box.collision(player2.box)) {
      box.x = player2.box.left() - box.w;
      xVelocity = xVelocity * -1;
    }

    if(box.collision(left)) {
      reset();
    }

    if(box.collision(right)) {
      reset();
    }

    box.x += xVelocity;
    box.y += yVelocity;
  }

  void reset() {
    box.x = startX;
    box.y = startY;
  }

  void render() {
    box.render();
  }
};

void init() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, WIDTH, HEIGHT, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv) {
  // SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Init(SDL_INIT_VIDEO); // Init SDL2
  
  //SDL_Surface * screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE|SDL_OPENGL);

  // Create a window. Window mode MUST include SDL_WINDOW_OPENGL for use with OpenGL.
  SDL_Window *window = SDL_CreateWindow(
    "Pong", 0, 0, 0, 0, SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_FULLSCREEN
  );

  // Create an OpenGL context associated with the window.
  SDL_GLContext glcontext = SDL_GL_CreateContext(window);

  bool running = true;
  uint32_t start;
  SDL_Event event;

  init();

  Player player;
  player.init(SDLK_w, SDLK_s);
  player.box.init(80, 280);

  Player player2;
  player2.init(SDLK_UP, SDLK_DOWN);
  player2.box.init(1130, 280);

  Ball ball;
  ball.init();

  // Bounds
  Box top; top.init(0, TOP, WIDTH, BLOCK);
  Box bottom; bottom.init(0, BOTTOM-BLOCK, WIDTH, BLOCK);
  Box left; left.init(0, TOP, 0, HEIGHT);
  Box right; right.init(WIDTH, TOP, 0, HEIGHT);

  while(running) {
    start = SDL_GetTicks();

    glClear(GL_COLOR_BUFFER_BIT);
    player.render();
    player2.render();
    top.render();
    bottom.render();
    ball.render();
    drawDottedLine();
    glFlush();

    while(SDL_PollEvent(&event)) {

      // Handle OS quit events
      if(event.type == SDL_QUIT) {
        running = false;
        break;
      }

      // Handle key down events
      if(event.type == SDL_KEYDOWN) {

        // debug("%s key pressed.", SDL_GetKeyName(key));
        SDL_Keycode key      = event.key.keysym.sym;

        if(key == SDLK_ESCAPE) {
running = false;
break;
}

        player.keyDown(key);
        player2.keyDown(key);
      }

      if(event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;

        player.keyUp(key);
        player2.keyUp(key);
      }

      // debug("unhandled event %i", event.type);
    }

    player.update(top, bottom);
    player2.update(top, bottom);
    ball.update(player, player2, top, bottom, left, right);

    SDL_GL_SwapWindow(window);

    int offset = SDL_GetTicks() - start;
    if((1000 / FPS) > offset)
      SDL_Delay((1000 / FPS) - offset);
  }

  return 0;
}