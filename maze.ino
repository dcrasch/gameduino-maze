#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>

/* MAZE */
#define WIDTH 48
#define HEIGHT 24
#define LEFT 240 - WIDTH * 4
#define TOP 136 - HEIGHT * 4

byte isclosed(byte row, byte col);
byte getvisited(byte row, byte col);

static byte maze[HEIGHT][WIDTH];
static byte visitedrow = 0, visitedcol = 0;

byte inside(int row, int col) {
  return ((col >= 0) && (row >= 0) && (col < WIDTH) && (row < HEIGHT));
}

byte goleft(byte row, byte col) {
  return inside(row, col) && (maze[row][col] & 1);
}

void placeleft(byte row, byte col, byte block) {
  maze[row][col] = maze[row][col] & 14 | block;
}

byte goright(byte row, byte col) {
  return inside(row, col + 1) && (maze[row][col + 1] & 1);
}

void placeright(byte row, byte col, byte block) {
  maze[row][col + 1] = maze[row][col + 1] & 14 | block;
}

byte goup(byte row, byte col) {
  return inside(row, col) && (maze[row][col] & 2);
}

void placeup(byte row, byte col, byte block) {
  maze[row][col] = maze[row][col] & 13 | block << 1;
}

byte godown(byte row, byte col) {
  return inside(row + 1, col) && (maze[row + 1][col] & 2);
}

void placedown(byte row, byte col, byte block) {
  maze[row + 1][col] = maze[row + 1][col] & 13 | block << 1;
}

byte candidates(byte row, byte col, byte*directions) {
  byte numcandidates = 0;
  if (inside(row - 1, col) && isclosed((byte)row - 1, col)) directions[numcandidates++] = 0;
  if (inside(row, col - 1) && isclosed(row, (byte)col - 1)) directions[numcandidates++] = 1;
  if (inside(row + 1, col) && isclosed((byte)row + 1, col)) directions[numcandidates++] = 2;
  if (inside(row, col + 1) && isclosed(row, (byte)col + 1)) directions[numcandidates++] = 3;
  return numcandidates;
}

void initmaze() {
  byte row, col;
  for (row = 0; row < HEIGHT; row++) {
    for (col = 0; col < WIDTH; col++) {
      maze[row][col] = 0;
    }
  }
}
byte nexttovisited (byte row, byte col) {
  return (goleft (row, col) && getvisited(row, col - 1)) ||
         (goright (row, col) && getvisited(row, col + 1)) ||
         (goup (row, col) && getvisited(row - 1, col)) ||
         (godown (row, col) && getvisited(row + 1, col));
}

byte isclosed (byte row, byte col)
{
  return (!goleft(row, col) &&
          !goright(row, col) &&
          !goup(row, col) &&
          !godown(row, col));
}

void setvisited(byte row, byte col, byte b) {
  maze[row][col] = (maze[row][col] & 3) | (b << 2);
}

byte getvisited(byte row, byte col) {
  return maze[row][col] >> 2;
}

void clearvisited() {
  byte row, col;
  for (row = 0; row < HEIGHT; row++) {
    for (col = 0; col < WIDTH; col++) {
      setvisited(row, col, 0);
    }
  }
}

void generatemaze() {
  byte directions[4] = {0};
  byte row = 0;
  byte col = 0;
  int visited = 1;
  byte choice;
  byte numcandidates = 0;
  int totalCells = WIDTH * HEIGHT;
  while (visited < totalCells) {
    numcandidates = candidates(row, col, directions);
    if (numcandidates != 0) {
      choice = directions[random(numcandidates)];
      switch (choice) {
        case 0:
          placeup(row, col, 1);
          row--;
          break;
        case 1:
          placeleft(row, col, 1);
          col--;
          break;
        case 2:
          placedown(row, col, 1);
          row++;
          break;
        case 3:
          placeright(row, col, 1);
          col++;
          break;
      }
      setvisited(row, col, choice);
      visited++;
    }
    else {
      switch (getvisited(row, col)) {
        case 0:
          row++;
          break;
        case 1:
          col++;
          break;
        case 2:
          row--;
          break;
        case 3:
          col--;
          break;
      }
    }
  }
}

void setup()
{
  byte i;
  GD.begin();
  GD.cmd_memwrite(0, 8 * 4);
  static const PROGMEM prog_uchar left_up[] = {
    0b10000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,

    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,

    0b11111111,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,

    0b11111111,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
  };
  GD.copy(left_up, 8 * 4);

  for (i = 0; i < 5; i++) {
    GD.BitmapHandle(i);
    GD.BitmapSource(i * 8); // start
    GD.BitmapSize(NEAREST, BORDER, BORDER, 8, 8);
    GD.BitmapLayout(L1, 1, 8);
  }

  initmaze();
  generatemaze();
  drawmaze();
}

void drawmaze() {
  GD.Clear();
  GD.Begin(BITMAPS);
  byte c;
  byte row, col;

  for (row = 0; row < HEIGHT; row++) {
    GD.ColorRGB(55 + 200 / HEIGHT * row, 0, 0);

    for (col = 0; col < WIDTH; col++) {

      c = 3 - maze[row][col] & 3;
      GD.Vertex2ii(LEFT + col * 8, TOP + row * 8, c);
    }
    GD.Vertex2ii(LEFT + WIDTH * 8, TOP + row * 8, 1);
  }
  for (col = 0; col < WIDTH; col++) {
    GD.Vertex2ii(LEFT + col * 8, TOP + HEIGHT * 8, 2);
  }
  GD.Vertex2ii(LEFT + WIDTH * 8, TOP + HEIGHT * 8, 0);
  GD.ColorRGB(0xff0000);
  GD.Vertex2ii(LEFT + visitedcol * 8 + 4, TOP + visitedrow * 8 + 4, 0);
  GD.swap();
}

void loop()
{
  GD.get_inputs();
  int x, y, z;
  GD.get_accel(x, y, z);
  if ((visitedcol > 0) && (x < 0) && goleft(visitedrow, visitedcol)) {
    delay(100);
    visitedcol--;
  }
  if ((visitedcol < WIDTH) && (x > 0) && goright(visitedrow, visitedcol)) {
    delay(100);
    visitedcol++;
  }
  if ((visitedrow > 0) && (y < 0) && goup(visitedrow, visitedcol)) {
    delay(100);
    visitedrow--;
  }
  if ((visitedrow < HEIGHT) && (y > 0) && godown(visitedrow, visitedcol)) {
    delay(100);
    visitedrow++;
  }
  if (GD.inputs.x != -32768) {
    initmaze();
    generatemaze();
    visitedrow = 0;
    visitedcol = 0;
  }
  drawmaze();
}
