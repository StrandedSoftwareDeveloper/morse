#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>

#define DIT_TIME 60*1000 //0.06 seconds, measured in us
#define DAH_TIME DIT_TIME*3
#define INTRA_CHAR_SPACE DIT_TIME
#define INTER_CHAR_SPACE DIT_TIME*3
#define WORD_SPACE DIT_TIME*5 //7 - 2 because of the surrounding spaces

const char charOrder[] = {
    'k',
    'm',
    'u',
    'r',
    'e',
    's',
    'n',
    'a',
    'p',
    't',
    'l',
    'w',
    'i',
    '.',
    'j',
    'z',
    '=',
    'f',
    'o',
    'y',
    ',',
    'v',
    'g',
    '5',
    '/',
    'q',
    '9',
    '2',
    'h',
    '3',
    '8',
    'b',
    '?',
    '4',
    '7',
    'c',
    '1',
    'd',
    '6',
    '0',
    'x',
};

const char *morseTable[] = {
    "", //0x00
    "", //0x01
    "", //0x02
    "", //0x03
    "", //0x04
    "", //0x05
    "", //0x06
    "", //0x07
    "", //0x08
    "", //0x09
    ".-.-", //0x0A, LF
    "", //0x0B
    "", //0x0C
    "", //0x0D
    "", //0x0E
    "", //0x0F

    "", //0x10
    "", //0x11
    "", //0x12
    "", //0x13
    "", //0x14
    "", //0x15
    "", //0x16
    "", //0x17
    "", //0x18
    "", //0x19
    "", //0x1A
    "", //0x1B
    "", //0x1C
    "", //0x1D
    "", //0x1E
    "", //0x1F

    "/", //0x20, SPACE
    "-.-.--", //0x21, !
    ".-..-.", //0x22, "
    "", //0x23, #
    "", //0x24, $
    "", //0x25, %
    ".-...", //0x26, &
    ".----.", //0x27, '
    "-.--.", //0x28, (
    "-.--.-", //0x29, )
    "", //0x2A, *
    ".-.-.", //0x2B, +
    "--..--", //0x2C, ,
    "-....-", //0x2D, -
    ".-.-.-", //0x2E, .
    "-..-.", //0x2F, /

    "-----", //0x30, 0
    ".----", //0x31, 1
    "..---", //0x32, 2
    "...--", //0x33, 3
    "....-", //0x34, 4
    ".....", //0x35, 5
    "-....", //0x36, 6
    "--...", //0x37, 7
    "---..", //0x38, 8
    "----.", //0x39, 9
    "---...", //0x3A, :
    "", //0x3B, ;
    "", //0x3C, <
    "-...-", //0x3D, =
    "", //0x3E, >
    "..--..", //0x3F, ?

    ".--.-.", //0x40, @
    ".-", //0x41, A
    "-...", //0x42, B
    "-.-.", //0x43, C
    "-..", //0x44, D
    ".", //0x45, E
    "..-.", //0x46, F
    "--.", //0x47, G
    "....", //0x48, H
    "..", //0x49, I
    ".---", //0x4A, J
    "-.-", //0x4B, K
    ".-..", //0x4C, L
    "--", //0x4D, M
    "-.", //0x4E, N
    "---", //0x4F, O

    ".--.", //0x50, P
    "--.-", //0x51, Q
    ".-.", //0x52, R
    "...", //0x53, S
    "-", //0x54, T
    "..-", //0x55, U
    "...-", //0x56, V
    ".--", //0x57, W
    "-..-", //0x58, X
    "-.--", //0x59, Y
    "--..", //0x5A, Z
    "", //0x5B, [
    "", //0x5C, \.
    "", //0x5D, ]
    "", //0x5E, ^
    "", //0x5F, _

    "", //0x60, `
    ".-", //0x61, a
    "-...", //0x62, b
    "-.-.", //0x63, c
    "-..", //0x64, d
    ".", //0x65, e
    "..-.", //0x66, f
    "--.", //0x67, g
    "....", //0x68, h
    "..", //0x69, i
    ".---", //0x6A, j
    "-.-", //0x6B, k
    ".-..", //0x6C, l
    "--", //0x6D, m
    "-.", //0x6E, n
    "---", //0x6F, o

    ".--.", //0x70, p
    "--.-", //0x71, q
    ".-.", //0x72, r
    "...", //0x73, s
    "-", //0x74, t
    "..-", //0x75, u
    "...-", //0x76, v
    ".--", //0x77, w
    "-..-", //0x78, x
    "-.--", //0x79, y
    "--..", //0x7A, z
    "", //0x7B, {
    "", //0x7C, |
    "", //0x7D, }
    "", //0x7E, ~
    "", //0x7F
};

int clamp(int val, int min, int max) {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }
    return val;
}

void playMorse(const char *morse, ma_sound *dit, ma_sound *dah) {
    int len = strlen(morse);
    for (int i = 0; i<len; i++) {
        if (morse[i] == '.') {
            ma_sound_start(dit);
            usleep(DIT_TIME+INTRA_CHAR_SPACE);
        } else if (morse[i] == '-') {
            ma_sound_start(dah);
            usleep(DAH_TIME+INTRA_CHAR_SPACE);
        } else if (morse[i] == ' ') {
            usleep(INTER_CHAR_SPACE);
        } else if (morse[i] == '/') {
            usleep(WORD_SPACE);
        }
    }
}

void textToMorse(const char *text, char *outBuffer, int outBufferLen) {
    memset(outBuffer, 0, outBufferLen);

    int len = strlen(text);
    int buffer_index = 0;
    for (int i=0; i<len; i++) {
        for (int j=0; j<strlen(morseTable[text[i]]); j++) {
            outBuffer[buffer_index++] = morseTable[text[i]][j];
            if (buffer_index >= outBufferLen-1) {
                i = INT_MAX;
                break;
            }
        }
        outBuffer[buffer_index++] = ' ';
        if (buffer_index >= outBufferLen-1) {
            break;
        }
    }
    outBuffer[buffer_index] = '\0';
}

void playMorseText(const char *text, ma_sound *dit, ma_sound *dah) {
    char buffer[1024];
    textToMorse(text, buffer, sizeof(buffer));
    playMorse(buffer, dit, dah);
}

int main(int argc, char** argv) {
    srand(time(NULL));

    int numLearned = 2;
    if (argc > 1) {
        numLearned = strtol(argv[1], NULL, 0);
        numLearned = clamp(numLearned, 1, 41);
    }

    ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
        return -1;
    }

    ma_sound dit;
    result = ma_sound_init_from_file(&engine, "dit.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &dit);
    if (result != MA_SUCCESS) {
        return result;  // Failed to load sound.
    }

    ma_sound dah;
    result = ma_sound_init_from_file(&engine, "dah.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &dah);
    if (result != MA_SUCCESS) {
        return result;  // Failed to load sound.
    }

    initscr();
    printw("Press space to repeat the last letter, esc to quit\n");
    printw("The current letters are: 'k'");
    for (int i=1; i<numLearned; i++) {
        printw(", '%c'", charOrder[i]);
    }
    printw("\n");
    refresh();

    int numRight = 0;
    int numTotal = 0;
    char text[2];
    text[0] = charOrder[rand() % numLearned];
    text[1] = '\0';

    time_t startTime = time(NULL);
    while (true) {
        playMorseText(text, &dit, &dah);

        char c = getch();
        if (c == 27) {
            break;
        } else if (c == ' ') {
            continue;
        }

        printw(":%c\n", text[0]);
        refresh();

        numRight += (c == text[0]);
        numTotal++;

        text[0] = charOrder[rand() % numLearned];
        text[1] = '\0';

        usleep(INTER_CHAR_SPACE);
        if (getcury(stdscr) >= getmaxy(stdscr)-1) {
            clear();
            refresh();
        }
    }
    time_t endTime = time(NULL);

    endwin();
    ma_engine_uninit(&engine);

    long timeTaken = endTime-startTime;
    float cpm = (numTotal / (float)timeTaken) * 60.0f;
    //float wpm = cpm / 5.0f;
    printf("%d/%d correct, %.1f%%\nCompleted in %ld seconds, %.1f cpm\n", numRight, numTotal, (numRight/(float)numTotal) * 100.0f, timeTaken, cpm);

    return 0;
}