# MorseCode

The code implemented is a mix of assembly and C. The button captures events of GP21 are taken in from the GPIO interrupts written in assembly. The falling and rising mask of the GP21 is detected. The time elapsed between the fall event and the rise event is calculated using the `time_us_32()` function in the SDK.

Depending on the time taken, a DOT, DASH, SPACE, or END command is sent, denoted as 0, 1, 2, 3 in integers, and sent to the `addinput()` function written in C. This function takes the 0, 1, 2, 3 values and puts their respective notations (i.e., `'.', '', 'S', '\0'`) into the input array.

All the alphanumeric values are stored in the `letter_table`, and the decoded letters are stored in the `letters` array. Using the `rand()` function, a random number is generated, and modulo 36 is taken (as there are only 36 values to choose from). The alphanumeric value corresponding to that is displayed, and the input is compared with the Morse code equivalent of the alphanumeric value.

Levels 1 & 2 are implemented in the same function `level()`, and `switch_function()` is responsible for handling which level the game starts at.

## Design Decisions

- All the data taken in from the button captures is taken in from the GP21 button and stored in an array to simplify data storage in C.
- Instead of using the Morse code representations directly, 0, 1, 2, 3 are sent, as the data storage in an array was not consistent with what was expected.
- One function was used to handle the basic levels as it made the code a bit more readable.
- Global variables were used as local variables got overwritten when calling `main_asm()`.

