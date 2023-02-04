#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "ws2812.pio.h"
<<<<<<< assign-2/assign02.c
#include "morse_code.h"
#include <string.h>
>>>>>>> assign-2/assign02.c
#define IS_RGBW true        // Will use RGBW format
#define NUM_PIXELS 1        // There is 1 WS2812 device in the chain
#define WS2812_PIN 28       // The GPIO pin that the WS2812 connected to

// Declare the main assembly code entry point.
void main_asm();

//                          Functions for the pio(controlling the RGB)
=======
//                    SDK functions that are called from main_asm()
///////////////////////////////////////////////////////////////////////////////

//Get the current timestamp
uint32_t  asm_get_time(){
    return time_us_32();
}
// Initialise a GPIO pin � see SDK for detail on gpio_init()
void asm_gpio_init(uint pin) {
    gpio_init(pin);
}
// Set direction of a GPIO pin � see SDK for detail on gpio_set_dir()
void asm_gpio_set_dir(uint pin, bool out) {
    gpio_set_dir(pin, out);
}
// Get the value of a GPIO pin � see SDK for detail on gpio_get()
bool asm_gpio_get(uint pin) {
    return gpio_get(pin);
}
// Set the value of a GPIO pin � see SDK for detail on gpio_put()
void asm_gpio_put(uint pin, bool value) {
    gpio_put(pin, value);
}
// Enable falling-edge interrupt � see SDK for detail on gpio_set_irq_enabled()
void asm_gpio_set_irq_fall(uint pin) {
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true);
}
// Enable Rising-edge interrupt
void asm_gpio_set_irq_rise(uint pin){
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE, true);
}

void asm_watchdog_reset(){              //to reset the watchdog timer
    watchdog_update();
}
//////////////////////////////////////////////////////////////////////////////////////////


//                          C WRAPPER FOR USING THE RGB LED

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Wrapper function used to call the underlying PIO
 *        function that pushes the 32-bit RGB colour value
 *        out to the LED serially using the PIO0 block. The
 *        function does not return until all of the data has
 *        been written out.
 * 
 * @param pixel_grb The 32-bit colour value generated by urgb_u32()
 */

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
/**
 * @brief Function to generate an unsigned 32-bit composit GRB
 *        value by combining the individual 8-bit paramaters for
 *        red, green and blue together in the right order.
 * 
 * @param r     The 8-bit intensity value for the red component
 * @param g     The 8-bit intensity value for the green component
 * @param b     The 8-bit intensity value for the blue component
 * @return uint32_t Returns the resulting composit 32-bit RGB value
 */
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return  ((uint32_t) (r) << 8)  |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void light_green()                                          // Function which Turns the RGB LED green
{
    put_pixel(urgb_u32(0x00, 0x7F, 0x00));                  //GREEN  When there are 3 lives are left
}
void light_blue()                                          // Function which Turns the RGB LED blue
{
    put_pixel(urgb_u32(0x00, 0x00, 0x7F));                  //BLUE   When game isnt in progress
}
void light_yellow()                                          // Function which Turns the RGB LED yellow
{
    put_pixel(urgb_u32(0x7F, 0x7F, 0x00));                  //YELLOW When there are 2 lives are left
}
void light_orange()                                          // Function which Turns the RGB LED orange
{
    put_pixel(urgb_u32(0x7F, 0x40, 0x00));                  //ORANGE When there is 1 life left
}
void light_red()                                          // Function which Turns the RGB LED red
{
   put_pixel(urgb_u32(0x7F, 0x00, 0x00));                  //RED    to denote game over
}

void light_off(){
   put_pixel(urgb_u32(0x00, 0x00, 0x00));      //turn off the rgb
}

void life_to_color(int a){
    if(a==3){
        light_green();
    }
    else if(a==2){
        light_yellow();
    }
    else if(a==1){
        light_orange();
    }
    else if(a==0){
        light_red();
    }
}
///////////////////////////////////////////////////////////////////////////////

//                              C code to get the data from assembly
////////////////////////////////////////////////////////////////////////////////////
char input[20];
int ind = 0;
void add_input(int sample){
    if (ind < 20){
        if (sample == 0){
            input[ind] = '.';
            ind++;
        }
        else if (sample == 1){
            input[ind] = '_';
            ind++;
        }
        else if (sample == 2){
            input[ind] = 'S';
            ind++;
        }
        else if (sample == 3){
            input[ind] = '\0';
            ind++;
        }
    }
}
/*
char current_input[10];
int current_index = 0;
void add_input(char sample){
    if (current_index < 10){
        current_input[current_index] = sample;
        current_index++;
    }
    printf("%s\n",current_input);
}*/

/**
 * @brief compare each location of two charachter arrays
 * until the '\0' charachter is reached
 * 
 * @param arr_1 
 * @param arr_2 
 * @return int: 1 if values are equel, 0 otherwise 
 */
int char_cmp(char* arr_1, char* arr_2){
  char c_1, c_2;
  for (int i = 0; i < 5; i++){
    c_1 = arr_1[i];
    c_2 = arr_2[i];
    if (c_1 != c_2) return 0;
    if (c_1 == '\0') return 1;
  }
  return 0;
};

/**
 * @brief Handler code for level 1 and 2, prints expected user input and calls main_asm() to take it.
 * checks if the user input returned by main_asm() is correct
 * 
 * @param print_morse 1 if morse code is to be printed (level 1), 0 if morse code is not to be printed (level 2)
 */
void charachter_challenge(int print_morse){
    //decide on and print expected letter
    int l = rand()% 26; // Generate random number between 0 and 25 tto represent number in the alphabet
    printf("Input: %c\n",(l + 'A')); // print the letter in the alphabet represented by the number
    if (print_morse){
        printf("Expected Morse: ");
        for(int i = 0; i < 5; i++){
            if (letter_table[l][i] == '\0') break;
            printf("%c", letter_table[l][i]);
            
        }
    }
    main_asm();
    if(char_cmp(current_input, letter_table[l])) printf("Sucsess"); //TODO 

}


void word_challenge(int show_morse){
  int l = rand()% NUM_WORDS; // Generate random number between 0 and 2 to represent word from list

  // print the expected word
  printf("Input: ");
  for(int i = 0; i<5; i++){
    if (word_table[l][i] == '\0') break;
    printf("%c", word_table[l][i]); 
  }

  printf("\n");
  // If on level 3, print out the required morse code
  if(show_morse){
    for(int i = 0; i<5; i++){
      if (word_table[l][i] == '\0') break;
      int c = word_table[l][i] - 'A';
      for(int j = 0; j < 5; j++){
        if (letter_table[c][j] == '\0') break;
        printf("%c", letter_table[c][j]); 
      }
      printf("S");
    }
  }

  // run main_asm()
  char buff[5];
  int word_pos = 0;
  int letter_pos = 0;
  for(int k = 0; k < 5; k++){
    int c = word_table[l][k] - 'A';
    while ((current_input[word_pos] != '\0')&&(current_input[word_pos] != 'S')){
      buff[letter_pos] = current_input[word_pos];
      word_pos++;
      letter_pos++;
    }
    buff[letter_pos] = '\0';
    if (!char_cmp(buff, letter_table[c])){
      printf("Fail\n");
      break;
    }

    if (current_input[word_pos] == '\0'){
      printf("Succses\n");
      break;
    }
    word_pos++;
    letter_pos = 0;



    
  }
  // for each entry in current_input seperated by a 'S' until '\0' charachter
  //    put into buffer array and delimate with '\0'
  // check if buffer matches the expected letter
}


  // run main_asm()
  char buff[5];
  int j = 0;
  for(int i = 0; i < 5; i++){
    int c = word_table[l][i] - 'A';
    while ((current_input[j] != '\0')&&(current_input[j] != 'S')){
      buff[j] = current_input[j];
      j++;
    }
    if (!char_cmp(buff, letter_table[c])){
      printf("Fail\n");
      break;
    }

    if (current_input[i] == '\0'){
      printf("Succses\n");
      break;
    }



    
  }
  // for each entry in current_input seperated by a 'S' until '\0' charachter
  //    put into buffer array and delimate with '\0'
  // check if buffer matches the expected letter
}

=======
}

//      FUNCTION FOR CLEARING THE INPUT ARRAY SO THAT IT CAN BE OVERWRITTEN
void clear_input(){
    ind=0;
    memset(input,0,20*sizeof(input[0]));
}
//////////////////////////////////////////////////////////////////
//                              Gameplay Part

int lives=3;
char a[4]={'.','.','.','\0'};

int match_input(){ //Function for matching the inputs

}

void level(){       //Function for Level-1(Match Characters with right sequence given)

}
void leve2(){       //Function for Level-2(Match Characters without the right sequence given)

}
void leve3(){       //Function for Level-3(Match Words with right sequence given)

}
void level4(){      //Function for Level-4(Match Words without the right sequence given)

}

void welcome(){     //Printing out the welcome message
light_blue();
printf("-------------------------------------------------\n");
printf("--                MORSE CODE GAME              --\n");
printf("--       USE GP-21 T0 INPUT THE SEQUENCE       --\n");
printf("--                 Level-1:'.'                 --\n");
printf("--                 Level-2:'..'                --\n");
printf("--                 Level-3:'...'               --\n");
printf("--                 Level-4:'....'              --\n");
printf("-------------------------------------------------\n");
}
>>>>>>> assign-2/assign02.c

// Main entry point of the application
int main() {
    stdio_init_all();                           // Initialise all basic IO

    // Initialise the PIO interface with the WS2812 code
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);

    // inittilize srand for pseudo-random intiger generation
    srand(time(NULL));
    //Printing out the Welcome banner
    welcome();
//    put_pixel(urgb_u32(0x00, 0x00, 0x00));      //turn off the rgb
    printf("ENTER THE MORSE CODE OF THE LEVEL YOU WANT:\n");
    watchdog_enable(8000000,1);
    clear_input();                              //Always use this before calling the function 
    main_asm();                                 //Jump into the ASM code
//    strcmp(input,level1);
    /*   TO DEMONSTRATE THAT STRCMP WORKS
    printf("%s\n",input);
    printf("%s\n",a);
    int result;
    result=strcmp(input,a);
    printf("%d",result);
    */
    return 0;                                   //Application return code
}
