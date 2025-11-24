# MemoryBlink

_NOTES TO ME_

-   No wifi features
-   No mixing and matching of modes (10 simon, 20 reverse, e.t.c.)

## Firmware Versions
I plan on having two versions for the firmware: one with just a simple segment display intended for more of a personal use without support for multiplayer features; while the other will have a simple lcd screen that uses a small user interface controllable by the buttons to change basic settings, add players and so on. Also, this is starting out with Arduino but I eventually want all my projects to be built using an ESP so I'll likely migrate it later on. This is just for prototyping and getting stuff to work.

### With screen

#### GAMEPLAY

-   At initial boot up, there are two options: choose game mode, change settings
-   After choosing a game mode, you'll be asked to choose the player (every player has their high scores for that game mode by the side; also, at the end of the list there'll be an option to add an extra player up to 100 (might be extended to the max available based on memory after every other feature's been implemented))
-   Every player plays a full turn and must end before another player can play.
-   The game runs on until you lose or get to the end at 100 steps per sequence (unlikely though :) ).
-   Your level is updated on a screen after each level.
-   The game is by default color agnostic. You can swap colors by physically replacing the LEDs


#### SETTINGS

-   time increase [=fixed, decreasing to limit] (50ms increments)

##### Sound

-   play sounds
-   tones for each key (from predefined tone list)

##### Players

-   reset high score for player
-   add player

### With Segment Display

#### GAMEPLAY

-   Before the game starts, all the lights should blink, the buzzer should sound, and segments should turn on to confirm everything works (more like a POST).
- The firmware waits for you to long press one of the 4 buttons to choose 4 different game modes.
- After that the high score for that game mode is displayed on the segment display and the games starts.
- After each level, your score is updated on the segment display.
