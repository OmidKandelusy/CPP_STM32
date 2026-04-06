// including the required header files:
#include "blink.hpp"

#define PIN_NUMBER 5

int main(void) {

    // creating a blink object:
    Blink Blink(GPIOA, PIN_NUMBER);

    // initialize the object:
    Blink.init();

    // a simple blink:
    Blink.blink();

    // pattern blink with pattern 0
    Blink.pattern(PATTERN_0);
    Blink.pattern(PATTERN_1);
    Blink.pattern(PATTERN_2);


    return 0;
}
