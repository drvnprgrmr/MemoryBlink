# MemoryBlink

_NOTES TO ME_

-   No wifi features
-   No mixing and matching of modes (10 simon, 20 reverse, e.t.c.)

-   The game is color agnostic (should use white LEDs). You can swap colors by physically replacing the pads.
-   There should be a physical switch to turn off the buzzer.

## Firmware Versions

I plan on having two versions for the firmware: one with just a simple segment display intended for more of a personal use without support for multiplayer features; while the other will have a simple lcd screen that uses a small user interface controllable by the buttons to change basic settings, add players and so on. Also, this is starting out with Arduino but I eventually want all my projects to be built using an ESP so I'll likely migrate it later on. This is just for prototyping and getting stuff to work.



### GAMEPLAY

There should be four different configurable toggles for the game mode.
- append/refresh ('append' adds one new position to the sequence while 'refresh' resets the sequence at every level)
- forward/backward ('forward' requires the player to enter the sequence in the same order, while 'backward' requires entering the sequence in the reverse order)
- sound/quiet ('sound' enables the buzzer, 'quiet' disables it)
- diff/same ('diff' uses different colors per pad while 'same' uses the same color for all pads)

The time between levels should be fixed at 4 seconds.
Let the game change pace after every 10 levels, for 4 times.
01-10: 500ms Flash / 125ms Delay | 5 seconds timeout
11-20: 400ms Flash / 100ms Delay | 4 seconds timeout
21-30: 300ms Flash / 75ms Delay | 3 seconds timeout
31-40: 200ms Flash / 50ms Delay | 2 seconds timeout
There'll be no more changes for subsequent levels

#### With screen

-   At initial boot up, you will be prompted to configure the game mode. The last chosen game mode will be selected
-   After choosing a game mode, you'll be asked to choose the player (every player has their high scores for that game mode by the side; also, at the end of the list there'll be an option to add an extra player up to 100 (might be extended to the max available based on memory after every other feature's been implemented/ or might just be a maximum of 4 players ;) ))
-   Every player plays a full turn and must end before another player can play.
-   The game runs on until you lose or get to the end at 100 steps per sequence (unlikely though :) ).
-   Your level is updated on a screen after each level.

#### With Segment Display

-   Before the game starts, all the lights should blink, the buzzer should sound, and segments should turn on to confirm everything works (more like a POST).
-   The firmware waits for you to long press one of the 4 buttons to choose 4 different game modes.
-   After that the high score for that game mode is displayed on the segment display and the games starts.
-   After each level, your score is updated on the segment display.

Button order for settings: up, down, left, right
