#include "spu_emu.h"

/*

10 bits for immediate value limits it for +511 and -512
byte mask limit: [0xFF...0] 
half mask limit: [0x1FF...0] [0xFE00...0xFFFF]
word mask limit: [0x1FF...0] [0xFFFFFE00...0xFFFFFFFF]
*/


