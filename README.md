# MemoryBlink

![the project built on a breadboard](pictures/Memory%20Blink%20(breadboarded%20build).jpg)

<!-- 
## Firmware Versions
I plan on having two versions for the firmware: one with just a simple segment display intended for more of a personal use without support for multiplayer features; while the other will have a simple lcd screen that uses a small user interface controllable by the buttons to change basic settings, add players and so on. -->

MemoryBlink is a game modeled after the *then* popular [Simon Says](https://en.wikipedia.org/wiki/Simon_(game)) memory game. I've added a couple of extra features though.
Asides being a fun project in general, this is also a project to develop and showcase my skills in electrical design, mechanical design and writing firmware. 

## GAMEPLAY

-   At initial boot up, you will be prompted to configure the game mode. The last chosen game mode will be selected
-   After choosing a game mode, you'll be asked to choose the player (every player has their high scores for that game mode by the side)
-   Every player plays a full turn and must end before another player can play.
-   The game runs on until you lose or get to the end at 40 sequences.
-   Your level is updated on a screen after each level.


### Game modes

There are four different configurable toggles for the game mode. Each can be turned on of off therefore leading to a total of 2<sup>4</sup> (i.e. 16) possibilities of different kinds of game modes. The four settings are explained below

- forward/backward  
('forward' requires the player to enter the sequence in the same order, while 'backward' requires entering the sequence in the reverse order)  
The difficulty weight of this setting is 8

- append/refresh  
'append' adds one new position to the sequence while 'refresh' resets the sequence at every level  
The difficulty weight of this setting is 4

- diff/same  
'diff' uses different colors per pad while 'same' uses the same color for all pads  
The difficulty weight of this setting is 2

- sound/quiet  
'sound' enables the buzzer, 'quiet' disables it  
The difficulty weight of this setting is 1

The cumulative difficulty is the sum of all 4 of these settings. By default, the difficulty is 0.

### Timing 
The time between levels should be fixed at 4 seconds. \
Let the game change pace after every 10 levels, for 4 times. \
01-10: 500ms Flash / 125ms Delay | 5 seconds timeout \
11-20: 400ms Flash / 100ms Delay | 4 seconds timeout \
21-30: 300ms Flash / 75ms Delay | 3 seconds timeout \
31-40: 200ms Flash / 50ms Delay | 2 seconds timeout \

### Display
Memory Blink! \
Player x (Gxxxx)  
Score:xx (HS:xx)

<!-- I'm second guessing aybe sometime later in the future
#### With Segment Display

-   Before the game starts, all the lights should blink, the buzzer should sound, and segments should turn on to confirm everything works (more like a POST).
-   You configure the game mode by pressing the 4 buttons to toggle settings.
-   After that the high score for that game mode is displayed on the segment display and the games starts.
-   After each level, your score is updated on the segment display. -->
