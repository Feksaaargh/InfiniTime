#include <sys/unistd.h>
#include "displayapp/screens/WatchFaceMaze.h"

#include "Tile.h"

using namespace Pinetime::Applications::Screens;

// Despite being called Maze, this really is only a relatively simple wrapper for the specialized
// (fake) 2d array on which the maze structure is built. It should only have manipulations for
// the structure, generating and printing should be handled elsewhere.
Maze::Maze() {
  std::fill_n(mazemap, FLATSIZE, 0);
}

// only returns 4 bits (since that's all that's stored)
// returns 0 in case of out of bounds access
MazeTile Maze::get(int x, int y) {
  if (x<0||x>WIDTH||y<0||y>HEIGHT) {return MazeTile(0b0011);}
  return get((y * WIDTH) + x);
}
MazeTile Maze::get(int index) {
  if (index < 0 || index/2 >= FLATSIZE) {return MazeTile(0b0011);}
  // odd means right (low) nibble, even means left (high) nibble
  if (index & 0b1) return MazeTile(mazemap[index/2] & 0b00001111);
  else             return MazeTile(mazemap[index/2] >> 4);
}

// only stores the low 4 bits of the value
// if out of bounds, does nothing
void Maze::set(int x, int y, MazeTile tile) {
  if (x<0||x>WIDTH||y<0||y>HEIGHT) {return;}
  set(y * WIDTH + x, tile);
}
void Maze::set(int index, MazeTile tile) {
  if (index < 0 || index/2 >= FLATSIZE) {return;}
  // odd means right (low) nibble, even means left (high) nibble
  if (index & 0b1) mazemap[index/2] = (mazemap[index/2] & 0b11110000) | tile.map;
  else             mazemap[index/2] = (mazemap[index/2] & 0b00001111) | tile.map << 4;
}

// only operates on the low 4 bits of the uint8_t.
// only sets the bits from the value that are also on in the mask, rest are left alone
// e.g. existing = 1010, value = 0001, mask = 0011, then result = 1001
// (mask defaults to 0xFF which keeps all bits)
void Maze::fill(uint8_t value, uint8_t mask) {
  value = value & 0b00001111;
  value |= value << 4;
  if (mask == 0xFF) {
    // did not include a mask
    std::fill_n(mazemap, FLATSIZE, value);
  } else {
    // included a mask
    mask = mask & 0b00001111;
    mask |= mask << 4;
    value = value & mask; // preprocess mask for value
    mask = ~mask;         // this mask will be applied to the value
    for (uint8_t& mapitem : mazemap) {
      mapitem = (mapitem & mask) + value;
    }
  }
}
inline void Maze::fill(MazeTile tile, uint8_t mask) {
  fill(tile.map, mask);
}

// For quickly manipulating. Also allows better abstraction by allowing setting of down and right sides.
// Silently does nothing if given invalid values.
void Maze::setSide(int index, TileAttr attr, bool value) {
  switch(attr) {
    case up:    set(index, get(index).setUp(value)); break;
    case down:  set(index+WIDTH, get(index+WIDTH).setUp(value)); break;
    case left:  set(index, get(index).setLeft(value)); break;
    case right: set(index+1, get(index+1).setLeft(value)); break;
    case flagempty: set(index, get(index).setFlagEmpty(value)); break;
    case flaggen: set(index, get(index).setFlagGen(value)); break;
  }
}
void Maze::setSide(int x, int y, TileAttr attr, bool value) {
  setSide(y*WIDTH+x, attr, value);
}
bool Maze::getSide(int index, TileAttr attr) {
  switch(attr) {
    case up:    return get(index).getUp();
    case down:  return get(index+WIDTH).getUp();
    case left:  return get(index).getLeft();
    case right: return get(index+1).getLeft();
    case flagempty: return get(index).getFlagEmpty();
    case flaggen: return get(index).getFlagGen();
  }
  return false;
}
bool Maze::getSide(int x, int y, TileAttr attr) {
  return getSide(y*WIDTH+x, attr);
}

void Maze::pasteMazeSeed(int x1, int y1, int x2, int y2, const uint8_t toPaste[]) {
  // Assumes a maze with empty flags all true, and all walls present
  uint16_t flatcoord = 0;  // the position in the array (inside the byte, so index 1 would be mask 0b00110000 in the first byte)
  for (int y = y1; y <= y2; y++) {
    for (int x = x1; x <= x2; x++) {
      // get target wall out of the paste buffer into the low two bits of a byte
      uint8_t working = (toPaste[flatcoord/4] & (0b11 << ((3-(flatcoord%4))*2))) >> ((3-(flatcoord%4))*2);
      // handle left wall
      if (!(working & 0b10)) {
        setSide(x, y, TileAttr::left, false);
        setSide(x, y, TileAttr::flagempty, false);
        if (x > 0) setSide(x-1, y, TileAttr::flagempty, false);
      }
      // handle up wall
      if (!(working & 0b01)) {
        setSide(x, y, TileAttr::up, false);
        setSide(x, y, TileAttr::flagempty, false);
        if (y > 0) setSide(x, y-1, TileAttr::flagempty, false);
      }
      flatcoord++;
    }
  }
}




WatchFaceMaze::WatchFaceMaze(Pinetime::Components::LittleVgl& lvgl,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Settings& settingsController,
                             Controllers::MotorController& motor)
  : dateTimeController {dateTimeController},
    settingsController {settingsController},
    motor {motor},
    lvgl {lvgl},
    maze {Maze()},
    prng {MazeRNG()} {

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  //Refresh();
}

WatchFaceMaze::~WatchFaceMaze() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

void WatchFaceMaze::Refresh() {
  realTime = dateTimeController.CurrentDateTime();
  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(realTime);
  // refresh time if either a minute has passed, or screen refresh timer expired.
  // if minute rolls over while screenrefresh is required, ignore it. the refresh timer will handle it.
  if (pausedGeneration ||  // if generation paused, need to complete it
      (currentState == Displaying::watchface && !screenRefreshRequired && currentDateTime.IsUpdated()) ||  // already on watchface, not waiting for a screen refresh, and time updated
      (screenRefreshRequired && realTime > screenRefreshTargetTime)) {  // waiting on a refresh
    // if generation wasn't paused (i.e. doing a ground up maze gen), set everything up
    if (!pausedGeneration) {
      // only reseed PRNG if got here by the minute rolling over
      if (!screenRefreshRequired) prng.seed(currentDateTime.Get().time_since_epoch().count());
      // prepare maze by filling it with all walls and empty tiles
      maze.fill(MazeTile().setLeft(true).setUp(true).setFlagEmpty(true));
      // seed the maze with whatever is required at the moment
      if (currentState == Displaying::watchface) {PutNumbers();}
      else if (currentState == Displaying::blank) {
        // seed maze with 4 tiles
        uint8_t seed[1] = {0xD5};
        int randx = prng.rand(0,20);
        int randy = prng.rand(3,20);
        maze.pasteMazeSeed(randx, randy, randx + 3, randy, seed);
      }
      else if (currentState == Displaying::loss) {
        maze.pasteMazeSeed(2, 2, 22, 21, loss);}
      else if (currentState == Displaying::amogus) {
        maze.pasteMazeSeed(3, 0, 21, 23, amogus);}
      else if (currentState == Displaying::autismcreature) {
        maze.pasteMazeSeed(0, 2, 11, 22, autismcreature);}
      else if (currentState == Displaying::foxgame) {
        maze.pasteMazeSeed(0, 1, 23, 22, foxgame);}
      else if (currentState == Displaying::reminder) {
        maze.pasteMazeSeed(0, 3, 23, 19, reminder);}
      else if (currentState == Displaying::pinetime) {
        maze.pasteMazeSeed(2, 0, 21, 23, pinetime);}
    }
    GenerateMaze();
    // only draw once maze is fully generated (not paused)
    if (!pausedGeneration) {
      ForceValidMaze();
      DrawMaze();
      screenRefreshRequired = false;
    }
  }
}

// allow pushing the button to go back to the main watchface
bool WatchFaceMaze::OnButtonPushed() {
  if (currentState != Displaying::watchface) {
    screenRefreshRequired = true;
    currentState = Displaying::watchface;
    return true;
  }
  return false;
}

bool WatchFaceMaze::OnTouchEvent(TouchEvents event) {
  // if generation is paused, let it continue working on that. This should really never trigger.
  if (pausedGeneration) {return false;}
  auto time = realTime;
  //std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time {};
  if (event == Pinetime::Applications::TouchEvents::LongTap) {
    // LONG TAPPED
    if (currentState ==  Displaying::watchface) {
      // on watchface; either refresh maze or go to blank state
      if (lastInputTime + std::chrono::milliseconds(2500) > time) {
        // long tapped twice in sequence; switch to blank maze
        currentState = Displaying::blank;
        screenRefreshRequired = true;
        std::fill_n(currentCode, sizeof(currentCode), 255);  // clear current code in preparation for code entry
      } else {
        // long tapped not in main watchface; go back to previous state
        screenRefreshRequired = true;
      }
      lastInputTime = time;
      motor.RunForDuration(20);
      return true;
    } else {
      screenRefreshRequired = true;
      currentState = Displaying::watchface;
      // reset lastInputTime so it always needs two long taps to get back to blank, even if you're fast
      lastInputTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>();
      motor.RunForDuration(20);
      return true;
    }
  }
  // handle swipes to input code on blank screen
  if (currentState != Displaying::watchface) {
    uint8_t swipeDir = 255;
    switch(event) {
      case Pinetime::Applications::TouchEvents::SwipeUp:    swipeDir = 0; break;
      case Pinetime::Applications::TouchEvents::SwipeRight: swipeDir = 1; break;
      case Pinetime::Applications::TouchEvents::SwipeDown:  swipeDir = 2; break;
      case Pinetime::Applications::TouchEvents::SwipeLeft:  swipeDir = 3; break;
      default: return false;  // only handle swipe events
    }
    for (int i = sizeof(currentCode)-1; i > 0; i--) {currentCode[i] = currentCode[i-1];}
    currentCode[0] = swipeDir;
    // check if valid code has been entered on non-main screen
    // structure also has the effect that if code gets entered while on the destination page, it doesn't refresh.
    Displaying newState = currentState;
    if (std::memcmp(currentCode, lossCode, sizeof(lossCode)) == 0)               {newState = Displaying::loss;}   // loss
    else if (std::memcmp(currentCode, amogusCode, sizeof(amogusCode)) == 0)      {newState = Displaying::amogus;}  // amogus
    else if (std::memcmp(currentCode, autismCode, sizeof(autismCode)) == 0)      {newState = Displaying::autismcreature;}  // autismcreature/tbh
    else if (std::memcmp(currentCode, foxCode, sizeof(foxCode)) == 0)            {newState = Displaying::foxgame;}  // foxxo game
    else if (std::memcmp(currentCode, reminderCode, sizeof(reminderCode)) == 0)  {newState = Displaying::reminder;}  // reminder
    else if (std::memcmp(currentCode, pinetimeCode, sizeof(pinetimeCode)) == 0)  {newState = Displaying::pinetime;}  // pinetime logo
    if (newState != currentState) {
      currentState = newState;
      screenRefreshRequired = true;
      motor.RunForDuration(10);
    }
    return true;
  }
  return false;
}


void WatchFaceMaze::PutNumbers() {
  uint8_t hours = dateTimeController.Hours();
  uint8_t minutes = dateTimeController.Minutes();
  // modify hours to account for 12 hour format
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    if (hours == 0) hours = 12;
    if (hours > 12) {
      maze.pasteMazeSeed(18, 15, 23, 22, pm);
      hours -= 12;
    } else {
      maze.pasteMazeSeed(18, 15, 23, 22, am);
    }
  }
  // put numbers on screen
  // top left: hours major digit
  maze.pasteMazeSeed(3, 1, 8, 10, numbers[hours / 10]);
  // top right: hours minor digit
  maze.pasteMazeSeed(10, 1, 15, 10, numbers[hours % 10]);
  // bottom left: minutes major digit
  maze.pasteMazeSeed(3, 13, 8, 22, numbers[minutes / 10]);
  // bottom right: minutes minor digit
  maze.pasteMazeSeed(10, 13, 15, 22, numbers[minutes % 10]);
}

// goes through the maze, finds disconnected segments and connects them
void WatchFaceMaze::ForceValidMaze() {
  // flood fill
  // this function repurposes flaggen for traversed tiles, so it expects it to be false on all tiles (should be in this program)
  // initialize x and y to bottom right
  int x = Maze::WIDTH-1, y = Maze::HEIGHT - 1;
  while (true) {
    ForceValidMazeLoop:
    maze.setSide(x, y, TileAttr::flaggen, true);
    // move cursor
    if (y > 0 && !maze.getSide(x, y, TileAttr::up) && !maze.getSide(x, y-1, TileAttr::flaggen)) {y--;}
    else if (x < Maze::WIDTH-1 && !maze.getSide(x, y, TileAttr::right) && !maze.getSide(x+1, y, TileAttr::flaggen)) {x++;}
    else if (y < Maze::HEIGHT-1 && !maze.getSide(x, y, TileAttr::down) && !maze.getSide(x, y+1, TileAttr::flaggen)) {y++;}
    else if (x > 0 && !maze.getSide(x, y, TileAttr::left) && !maze.getSide(x-1, y, TileAttr::flaggen)) {x--;}
    else {
      int pokeLocationCount = 0;
      // couldn't find any position to move to, need to set cursor to a different usable location
      for (int proposedy = 0; proposedy < Maze::HEIGHT; proposedy++) {
        for (int proposedx = 0; proposedx < Maze::WIDTH; proposedx++) {
          bool ownState = maze.getSide(proposedx, proposedy, TileAttr::flaggen);
          // if tile to the left is of a different traversal state (is traversed boundary)
          if (proposedx > 0 && (maze.getSide(proposedx-1, proposedy, TileAttr::flaggen) != ownState)) {
            // if found boundary AND can get to it, just continue working from here
            if (maze.getSide(proposedx, proposedy, TileAttr::left) == false) {x = proposedx, y = proposedy; goto ForceValidMazeLoop;}
            pokeLocationCount++;
          }
          // if tile to up is of a different traversal state (is traversed boundary)
          if (proposedy > 0 && (maze.getSide(proposedx, proposedy-1, TileAttr::flaggen) != ownState)) {
            // if found boundary AND can get to it, just continue working from here
            if (maze.getSide(proposedx, proposedy, TileAttr::up) == false) {x = proposedx, y = proposedy; goto ForceValidMazeLoop;}
            pokeLocationCount++;
          }
        }
      }
      // if no poke locations found, maze is finished
      if (pokeLocationCount == 0) {return;}
      // if execution gets here, then no valid position to start filling from was found. need to poke a hole.
      // choose a random poke location to poke a hole through. pokeLocationCount is now used as an index
      pokeLocationCount = prng.rand(1, pokeLocationCount);
      for (int proposedy = 0; proposedy < Maze::HEIGHT; proposedy++) {
        for (int proposedx = 0; proposedx < Maze::WIDTH; proposedx++) {
          bool ownState = maze.getSide(proposedx, proposedy, TileAttr::flaggen);
          if (proposedx > 0 && (maze.getSide(proposedx-1, proposedy, TileAttr::flaggen) != ownState)) {
            pokeLocationCount--;
            if (pokeLocationCount == 0) {
              maze.setSide(proposedx, proposedy, TileAttr::left, false);
              x = proposedx, y = proposedy;
              continue;
            }
          }
          // if tile to up is of a different traversal state (is traversed boundary)
          if (proposedy > 0 && (maze.getSide(proposedx, proposedy-1, TileAttr::flaggen) != ownState)) {
            pokeLocationCount--;
            if (pokeLocationCount == 0) {
              maze.setSide(proposedx, proposedy, TileAttr::up, false);
              x = proposedx, y = proposedy;
              continue;
            }
          }
        }
      }
    }
  }
}

void WatchFaceMaze::GenerateMaze() {
  int x, y, oldx, oldy;
  // task should only run for 3/4 the time it takes for the task to refresh.
  // Will go over; only checks once it's finished with current line. It won't go too far over though.
  auto maxGenTarget = dateTimeController.CurrentDateTime() + std::chrono::milliseconds((taskRefresh->period*3)/4);
  while (true) {
    // FIND POSITION TO START BRANCH FROM
    for (uint8_t i = 0; i < 30; i++) {
      x = prng.rand(0, Maze::WIDTH-1);
      y = prng.rand(0, Maze::HEIGHT-1);
      if (maze.getSide(x,y, TileAttr::flagempty)) {break;}  // found solution tile
      if (i == 29) {
        // failed all 30 attempts (this is inside the for loop for 'organization')
        // find solution tile slowly but guaranteed
        int count = 0;
        // count number of valid tiles
        for (int j = 0; j < Maze::WIDTH*Maze::HEIGHT; j++)
        {if (maze.getSide(j, TileAttr::flagempty)) {count++;}}
        if (count == 0) {
          // all tiles filled; maze gen done
          pausedGeneration = false;
          return;
        }
        // if maze gen not done, select random index from valid tiles to start from
        // 'count' is now used as an index
        count = prng.rand(1, count);
        for (int j = 0; j < Maze::WIDTH*Maze::HEIGHT; j++) {
          if (maze.getSide(j, TileAttr::flagempty)) {count--;}
          if (count == 0) {
            y = j / Maze::WIDTH;
            x = j % Maze::WIDTH;
            break;
          }
        }
      }
    }
    // function now has a valid position a maze line can start from in x and y
    oldx = -1, oldy = -1;
    // GENERATE A SINGLE PATH
    uint8_t direction;  // which direction the cursor moved in
    while (true) {
      maze.setSide(x, y, TileAttr::flagempty, false);  // no longer empty
      maze.setSide(x, y, TileAttr::flaggen, true);     // in generation
      oldx = x, oldy = y;
      // move to next tile
      // this is very scuffed, but it prevents backtracking.
      while (true) {
        switch (direction = prng.rand(0,3)) {
          case 0:  // moved up
            if (y <= 0 || !maze.getSide(x,y,TileAttr::up)) {continue;}
            y -= 1; break;
          case 1:  // moved left
            if (x <= 0 || !maze.getSide(x,y,TileAttr::left)) {continue;}
            x -= 1; break;
          case 2:  // moved down
            if (y >= Maze::HEIGHT-1 || !maze.getSide(x,y,TileAttr::down)) {continue;}
            y += 1; break;
          case 3:  // moved right
            if (x >= Maze::WIDTH-1 || !maze.getSide(x,y,TileAttr::right)) {continue;}
            x += 1; break;
        }
        break;
      }
      // moved to next tile. check if looped in on self
      if (!maze.getSide(x, y, TileAttr::flaggen)) {
        // did NOT loop in on self, simply remove wall and move on
        switch (direction) {
          case 0:  // moved up
            maze.setSide(x,y,TileAttr::down, false); break;
          case 1:  // moved left
            maze.setSide(x,y,TileAttr::right, false); break;
          case 2:  // moved down
            maze.setSide(x,y,TileAttr::up, false); break;
          case 3:  // moved right
            maze.setSide(x,y,TileAttr::left, false); break;
        }
        // if attached to main maze, path finished generating
        if (!maze.getSide(x, y, TileAttr::flagempty)) {break;}
      } else {
        // DID loop in on self, track down and eliminate loop
        // targets are the coordinates of where it needs to backtrack to
        int targetx = x, targety = y;
        x = oldx, y = oldy;
        while (x != targetx || y != targety) {
          if (y > 0 && (maze.getSide(x, y, TileAttr::up) == false)) {  // backtrack up
            maze.setSide(x, y, TileAttr::up, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            y -= 1;
          } else if (x > 0 && (maze.getSide(x, y, TileAttr::left) == false)) {  // backtrack left
            maze.setSide(x, y, TileAttr::left, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            x -= 1;
          } else if (y < Maze::HEIGHT-1 && (maze.getSide(x, y, TileAttr::down) == false)) {  // backtrack down
            maze.setSide(x, y, TileAttr::down, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            y += 1;
          } else if (x < Maze::WIDTH && (maze.getSide(x, y, TileAttr::right) == false)) {  // backtrack right
            maze.setSide(x, y, TileAttr::right, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            x += 1;
          } else {
            // bad backtrack; die
            std::abort();
          }
        }
      }
    }
    // mark all tiles as finalized and not in generation by removing ALL flaggen's
    maze.fill(0, MazeTile::FLAGGENMASK);
    if (dateTimeController.CurrentDateTime() > maxGenTarget) {
      pausedGeneration = true;
      return;
    }
  }
  // execution never gets here! it returns earlier in the function.
}

void WatchFaceMaze::DrawMaze() {
  // this used to be nice code, but it was retrofitted to print offset by 1 pixel for a fancy border.
  // I'm not proud of the logic but it works.
  lv_area_t area;
  lv_color_t *curbuf = buf1;
  // print horizontal lines
  area.x1 = 1;
  area.x2 = 238;
  for (int y = 1; y < Maze::HEIGHT; y++) {
    for (int x = 0; x < Maze::WIDTH; x++) {
      if (maze.get(x, y).getUp())  {std::fill_n(&curbuf[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_WHITE);}
      else                         {std::fill_n(&curbuf[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_BLACK);}
    }
    std::copy_n(curbuf, 238, &curbuf[238]);
    area.y1 = Maze::TILESIZE * y - 1;
    area.y2 = Maze::TILESIZE * y;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, curbuf);
    curbuf = (curbuf==buf1) ? buf2 : buf1;  // switch buffer
  }
  // print vertical lines
  area.y1 = 1;
  area.y2 = 238;
  for (int x = 1; x < Maze::WIDTH; x++) {
    for (int y = 0; y < Maze::HEIGHT; y++) {
      MazeTile curblock = maze.get(x,y);
      // handle corners: if any of the touching lines are present, add corner. else leave it black
      if (curblock.getUp() || curblock.getLeft() || maze.get(x-1,y).getUp() || maze.get(x,y-1).getLeft())
            {std::fill_n(&curbuf[y*Maze::TILESIZE*2], 4, LV_COLOR_WHITE);}
      else  {std::fill_n(&curbuf[y*Maze::TILESIZE*2], 4, LV_COLOR_BLACK);}

      if (curblock.getLeft()) {std::fill_n(&curbuf[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_WHITE);}
      else                    {std::fill_n(&curbuf[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_BLACK);}
    }
    area.x1 = Maze::TILESIZE * x - 1;
    area.x2 = Maze::TILESIZE * x;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, &curbuf[4]);
    curbuf = (curbuf==buf1) ? buf2 : buf1;  // switch buffer
  }
  // print borders
  std::fill_n(curbuf, 240, LV_COLOR_GRAY);
  for (int i = 0; i < 4; i++) {
    if (i==0)      {area.x1=0;   area.x2=239; area.y1=0;   area.y2=0;  } // top
    else if (i==1) {area.x1=0;   area.x2=239; area.y1=239; area.y2=239;} // bottom
    else if (i==2) {area.x1=0;   area.x2=0;   area.y1=0;   area.y2=239;} // left
    else if (i==3) {area.x1=239; area.x2=239; area.y1=0;   area.y2=239;} // right
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, curbuf);
  }
}