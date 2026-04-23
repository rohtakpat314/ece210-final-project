#include <Adafruit_NeoPixel.h>

// ============================================================================
// SNAKE GAME ON 8x8 NeoPixel Grid
// Controls: Left button = turn counter-clockwise, Right button = turn clockwise
// ============================================================================

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// Pin connected to the NeoPixel data line
#define PIN 4

// Total number of LEDs in the 8x8 grid (8 rows × 8 columns)
#define NUM_LEDS 64

// Button pins for left/right rotation
#define BTN_LEFT 2
#define BTN_RIGHT 3

// Initialize the NeoPixel strip with 64 LEDs, GRB color order, 800 kHz frequency
Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// ============================================================================
// UTILITY FUNCTION: Convert (x, y) coordinates to linear LED index
// ============================================================================
// Example: xy(0, 0) = 0 (top-left), xy(7, 7) = 63 (bottom-right)
// Row-major order: each row has 8 LEDs
int xy(int x, int y) {
  return y * 8 + x;
}

// ============================================================================
// GAME STATE: SNAKE
// ============================================================================

// Maximum snake length (cannot exceed 64 LEDs)
const int MAX_SNAKE = 32;

// Arrays to store the (x, y) position of each snake segment
int snakeX[MAX_SNAKE];
int snakeY[MAX_SNAKE];

// Current snake length (how many segments are drawn)
int snakeLen = 3;

// Direction the snake is moving: 0=up, 1=right, 2=down, 3=left
int dir = 1;

// ============================================================================
// GAME STATE: FOOD
// ============================================================================

// Position of the food pellet that the snake eats
int foodX = 5;
int foodY = 5;

// ============================================================================
// GAME STATE: TIMING & INPUT
// ============================================================================

// Tick counter: incremented each loop iteration
// Used to slow down snake movement for playability (moves every 7 ticks)
int tick = 0;

// Previous button state for edge detection
// (we only turn on button press, not while held)
bool prevL = false;
bool prevR = false;

// ============================================================================
// RESET GAME STATE
// ============================================================================
// Called when snake dies (wall collision, self collision, or game start)
void resetGame() {
  // Start with a snake of length 3, facing right
  snakeLen = 3;
  dir = 1;

  // Position snake in the middle, horizontally centered
  // Head at (2, 4), middle at (1, 4), tail at (0, 4)
  snakeX[0] = 2; snakeY[0] = 4;
  snakeX[1] = 1; snakeY[1] = 4;
  snakeX[2] = 0; snakeY[2] = 4;

  // Spawn food at a random location on the 8×8 grid
  foodX = random(0, 8);
  foodY = random(0, 8);
}

// ============================================================================
// HARDWARE SETUP
// ============================================================================
void setup() {
  // Initialize the NeoPixel strip and turn all LEDs off
  strip.begin();
  strip.show();

  // Configure button pins as inputs with internal pull-up resistors
  // (buttons connect to ground, so HIGH = not pressed, LOW = pressed)
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // Seed the random number generator with noise from an analog pin
  // (ensures different food positions each time the game resets)
  randomSeed(analogRead(A5));

  // Initialize the game state
  resetGame();
}

// ============================================================================
// INPUT HANDLING
// ============================================================================
// Read button inputs and update snake direction
// Uses edge detection so snake only turns on button *press*, not hold
void input() {
  // Read current button state (inverted because buttons pull LOW when pressed)
  bool left = !digitalRead(BTN_LEFT);
  bool right = !digitalRead(BTN_RIGHT);

  // Turn left (counter-clockwise): direction decreases
  // Only on button press (transition from not pressed to pressed)
  if (left && !prevL) {
    dir--;
    if (dir < 0) dir = 3;  // Wrap around: 0-1 becomes 3
  }

  // Turn right (clockwise): direction increases
  // Only on button press (transition from not pressed to pressed)
  if (right && !prevR) {
    dir++;
    if (dir > 3) dir = 0;  // Wrap around: 4 becomes 0
  }

  // Update previous state for next iteration's edge detection
  prevL = left;
  prevR = right;
}

// ============================================================================
// GAME LOGIC: UPDATE SNAKE POSITION
// ============================================================================
// Called ~14 times per second (every 7 ticks at 40ms per tick)
void updateSnake() {

  // ========================================================================
  // STEP 1: Shift the snake body
  // ========================================================================
  // Each segment moves to where the segment in front of it was
  // Start from tail and move backwards to avoid overwriting
  for (int i = snakeLen - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // ========================================================================
  // STEP 2: Move the head in the current direction
  // ========================================================================
  if (dir == 0) snakeY[0]--;        // Up: decrease Y
  if (dir == 1) snakeX[0]++;        // Right: increase X
  if (dir == 2) snakeY[0]++;        // Down: increase Y
  if (dir == 3) snakeX[0]--;        // Left: decrease X

  // ========================================================================
  // STEP 3: Collision detection - walls
  // ========================================================================
  // If head goes outside the 8×8 grid, the snake dies
  if (snakeX[0] < 0 || snakeX[0] > 7 || snakeY[0] < 0 || snakeY[0] > 7) {
    resetGame();
    return;
  }

  // ========================================================================
  // STEP 4: Collision detection - self
  // ========================================================================
  // Check if head position matches any body segment (starting from index 1)
  for (int i = 1; i < snakeLen; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      // Head hit body, snake dies
      resetGame();
      return;
    }
  }

  // ========================================================================
  // STEP 5: Collision detection - food (eating)
  // ========================================================================
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    // Grow the snake by 1 segment (if not already at max length)
    if (snakeLen < MAX_SNAKE) snakeLen++;

    // Spawn new food at a random location
    foodX = random(0, 8);
    foodY = random(0, 8);
  }
}

// ============================================================================
// RENDERING: Draw the game state to the NeoPixel grid
// ============================================================================
void draw() {
  // Clear all LEDs (set to black / off)
  strip.clear();

  // ========================================================================
  // Draw food (red)
  // ========================================================================
  strip.setPixelColor(xy(foodX, foodY), strip.Color(150, 0, 0));

  // ========================================================================
  // Draw snake body (green)
  // ========================================================================
  // Loop through all segments of the snake and light them up
  for (int i = 0; i < snakeLen; i++) {
    strip.setPixelColor(xy(snakeX[i], snakeY[i]), strip.Color(0, 150, 0));
  }

  // Send the color data to the physical LED strip
  strip.show();
}

// ============================================================================
// MAIN GAME LOOP
// ============================================================================
void loop() {

  // Read button inputs and update direction if needed
  input();

  // Increment the tick counter
  tick++;

  // ========================================================================
  // MOVEMENT THROTTLING
  // ========================================================================
  // Snake only moves every 7 ticks (40ms × 7 ≈ 280ms per move)
  // This creates a playable speed (~3.5 moves per second)
  if (tick > 6) {
    tick = 0;
    updateSnake();
  }

  // Render the current game state to the NeoPixel grid
  draw();

  // Small delay to let the LEDs update and prevent excessive looping
  // Also helps reduce button debouncing issues
  delay(40);
}
