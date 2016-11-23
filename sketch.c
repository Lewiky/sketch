#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "display.h"


typedef struct state {
    int x;
    int y;
    int opDX;
    int opDY;
    int opDT;
    int opPEN;
    display *d;
    FILE *in;
    int colour;
} State;

typedef unsigned char byte;

// TODO: upgrade the run function, adding functions to support it.

void drawLine(State *st){
    printf("moving from %d,%d to %d,%d\n",st->x,st->y,(st->x + st->opDX),(st->y + st->opDY));
    if(st->colour == 0x000000FF){
        line(st->d, st->x, st->y, (st->x + st->opDX),(st->y + st->opDY));
    }
    else{
        cline(st->d, st->x, st->y, (st->x + st->opDX),(st->y + st->opDY),st->colour);
    }
    st->x = st->x + st->opDX;
    st->y = st->y + st->opDY;
}
//update the opcode for a recieved DY/DX opcodes
void makeMove(State *st, int operand){
    st->opDY = operand;
    if(st->opPEN == 3){
            drawLine(st);
    }
    else{
        st->y = st->y + st->opDY;
        st->x = st->x + st->opDX;
    }
    st->opDY = 0;
    st->opDX = 0;
    return;
}

//update the opcode for a recieved DT opcode
void DT(State *st, int operand){
    st->opDT = operand;
    pause(st->d, st->opDT);
    return;
}

//update the opcode for a recieved PEN opcode
void PEN(State *st, int operand){
    if(st->opPEN == 3){
        st->opPEN = 0;
        printf("pen down\n");
        return;
    }
    else{
        st->opPEN = operand;
        return;
    }
}

//if opcode is 3, launch extension mechanism
void extension(State *st, int operand, byte b){
    int extendedOpcode = b  & 0x0F;
    int operandNum = (b >> 4) & 0x03;
    if (operandNum == 3) {operandNum = 4;}
    printf("operand num = %i \n", operandNum);
    int xtraOperand = 0;
    if(operandNum > 0){
        for(int i =0; i < operandNum; i++){
            byte oper = fgetc(st->in);
            xtraOperand = (xtraOperand << 8) | oper;
        }
    }
    int maxnum = (pow(2,(8*operandNum))/2)-1;
    if(xtraOperand >maxnum){
        xtraOperand = xtraOperand - (maxnum+1)*2;
    }
    printf("extra operand = %i\n",xtraOperand);
    if(extendedOpcode == 3){
        PEN(st,operand);
    }
    else if(extendedOpcode == 4){
        clear(st->d);
    }
    else if(extendedOpcode ==5){
        printf("Waiting for key press.\n");
        key(st->d);
    }
    else if(extendedOpcode == 2){
        DT(st, xtraOperand);
    }
    else if(extendedOpcode == 0){
        st->opDX = xtraOperand;
    }
    else if(extendedOpcode == 1){
        makeMove(st, xtraOperand);
    }
    else if(extendedOpcode == 6){
        st->colour = xtraOperand;
    }
}

//unpack the bytes from binary file into respective opcodes and operands
void unpack(byte b, int i, display *d, State *st){
    int opcode[16];
    int operand[16];
    opcode[i] = b >> 6 & 0xFF;
    operand[i] = b & 0x3F;
    if(operand[i] > 31 && (opcode[i] == 0 || opcode[i] == 1 )){
        operand[i] = operand[i] - 64;
    }
    printf("%i \t %i \n", opcode[i], operand[i]);
    st->d = d;
    if(opcode [i] == 0){
        st->opDX = operand[i];
    }
    else if(opcode[i] == 1){
        makeMove(st, operand[i]);
    }
    else if(opcode[i] == 2){
        DT(st ,operand[i]);
    }
    else{
        extension(st, operand[i], b);
    }
}
//reads individual bytes from a binary file and prints them out in order
void interpret(FILE *in, display *d){
    byte b = fgetc(in);
    int i = 0;
    State state = {0,0,0,0,0,0,d,in,0x000000FF};
    State *st = &state;
    while (! feof(in)) {
        printf("%02x\t", b);
        unpack(b, i, d, st);
        b = fgetc(in);
        i++;
    }
}
// Read sketch instructions from the given file.  If test is NULL, display the
// result in a graphics window, else check the graphics calls made.
void run(char *filename, char *test[]) {
    FILE *in = fopen(filename, "rb");
    if (in == NULL) {
        fprintf(stderr, "Can't open %s\n", filename);
        exit(1);
    }
    display *d = newDisplay(filename, 200, 200, test);
    interpret(in, d);
    end(d);
    fclose(in);
}

// ----------------------------------------------------------------------------
// Nothing below this point needs to be changed.
// ----------------------------------------------------------------------------

// Forward declaration of test data.
char **lineTest, **squareTest, **boxTest, **oxoTest, **diagTest, **crossTest,
     **clearTest, **keyTest, **pausesTest, **fieldTest, **lawnTest;

void testSketches() {
    // Stage 1
    run("line.sketch", lineTest);
    run("square.sketch", squareTest);
    run("box.sketch", boxTest);
    run("oxo.sketch", oxoTest);

    // Stage 2
    run("diag.sketch", diagTest);
    run("cross.sketch", crossTest);

    // Stage 3
    run("clear.sketch", clearTest);
    run("key.sketch", keyTest);

    // Stage 4
    run("pauses.sketch", pausesTest);
    run("field.sketch", fieldTest);
    run("lawn.sketch", lawnTest);
}

int main(int n, char *args[n]) {
    if (n == 1) testSketches();
    else if (n == 2) run(args[1], NULL);
    else {
        fprintf(stderr, "Usage: sketch [file.sketch]");
        exit(1);
    }
}

// Each test is a series of calls, stored in a variable-length array of strings,
// with a NULL terminator.

// The calls that should be made for line.sketch.
char **lineTest = (char *[]) {
    "line(d,30,30,60,30)", NULL
};

// The calls that should be made for square.sketch.
char **squareTest = (char *[]) {
    "line(d,30,30,60,30)", "line(d,60,30,60,60)",
    "line(d,60,60,30,60)","line(d,30,60,30,30)", NULL
};

// The calls that should be made for box.sketch.
char **boxTest = (char *[]) {
    "line(d,30,30,32,30)", "pause(d,10)", "line(d,32,30,34,30)", "pause(d,10)",
    "line(d,34,30,36,30)", "pause(d,10)", "line(d,36,30,38,30)", "pause(d,10)",
    "line(d,38,30,40,30)", "pause(d,10)", "line(d,40,30,42,30)", "pause(d,10)",
    "line(d,42,30,44,30)", "pause(d,10)", "line(d,44,30,46,30)", "pause(d,10)",
    "line(d,46,30,48,30)", "pause(d,10)", "line(d,48,30,50,30)", "pause(d,10)",
    "line(d,50,30,52,30)", "pause(d,10)", "line(d,52,30,54,30)", "pause(d,10)",
    "line(d,54,30,56,30)", "pause(d,10)", "line(d,56,30,58,30)", "pause(d,10)",
    "line(d,58,30,60,30)", "pause(d,10)", "line(d,60,30,60,32)", "pause(d,10)",
    "line(d,60,32,60,34)", "pause(d,10)", "line(d,60,34,60,36)", "pause(d,10)",
    "line(d,60,36,60,38)", "pause(d,10)", "line(d,60,38,60,40)", "pause(d,10)",
    "line(d,60,40,60,42)", "pause(d,10)", "line(d,60,42,60,44)", "pause(d,10)",
    "line(d,60,44,60,46)", "pause(d,10)", "line(d,60,46,60,48)", "pause(d,10)",
    "line(d,60,48,60,50)", "pause(d,10)", "line(d,60,50,60,52)", "pause(d,10)",
    "line(d,60,52,60,54)", "pause(d,10)", "line(d,60,54,60,56)", "pause(d,10)",
    "line(d,60,56,60,58)", "pause(d,10)", "line(d,60,58,60,60)", "pause(d,10)",
    "line(d,60,60,58,60)", "pause(d,10)", "line(d,58,60,56,60)", "pause(d,10)",
    "line(d,56,60,54,60)", "pause(d,10)", "line(d,54,60,52,60)", "pause(d,10)",
    "line(d,52,60,50,60)", "pause(d,10)", "line(d,50,60,48,60)", "pause(d,10)",
    "line(d,48,60,46,60)", "pause(d,10)", "line(d,46,60,44,60)", "pause(d,10)",
    "line(d,44,60,42,60)", "pause(d,10)", "line(d,42,60,40,60)", "pause(d,10)",
    "line(d,40,60,38,60)", "pause(d,10)", "line(d,38,60,36,60)", "pause(d,10)",
    "line(d,36,60,34,60)", "pause(d,10)", "line(d,34,60,32,60)", "pause(d,10)",
    "line(d,32,60,30,60)", "pause(d,10)", "line(d,30,60,30,58)", "pause(d,10)",
    "line(d,30,58,30,56)", "pause(d,10)", "line(d,30,56,30,54)", "pause(d,10)",
    "line(d,30,54,30,52)", "pause(d,10)", "line(d,30,52,30,50)", "pause(d,10)",
    "line(d,30,50,30,48)", "pause(d,10)", "line(d,30,48,30,46)", "pause(d,10)",
    "line(d,30,46,30,44)", "pause(d,10)", "line(d,30,44,30,42)", "pause(d,10)",
    "line(d,30,42,30,40)", "pause(d,10)", "line(d,30,40,30,38)", "pause(d,10)",
    "line(d,30,38,30,36)", "pause(d,10)", "line(d,30,36,30,34)", "pause(d,10)",
    "line(d,30,34,30,32)", "pause(d,10)", "line(d,30,32,30,30)", "pause(d,10)",
    NULL
};

// The calls that should be made for oxo.sketch.
char **oxoTest = (char *[]) {
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "line(d,30,40,60,40)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "line(d,30,50,60,50)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "line(d,40,30,40,60)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "line(d,50,30,50,60)", NULL
};

// The calls that should be made for diag.sketch.
char **diagTest = (char *[]) {
    "line(d,30,30,60,60)", NULL
};

// The calls that should be made for cross.sketch.
char **crossTest = (char *[]) {
    "line(d,30,30,60,60)", "line(d,60,30,30,60)", NULL
};

// The calls that should be made for clear.sketch.
char **clearTest = (char *[]) {
    "line(d,30,40,60,40)", "line(d,30,50,60,50)", "line(d,40,30,40,60)",
    "line(d,50,30,50,60)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "clear(d)", "line(d,30,30,60,60)",
    "line(d,60,30,30,60)", NULL
};

// The calls that should be made for key.sketch.
char **keyTest = (char *[]) {
    "line(d,30,40,60,40)", "line(d,30,50,60,50)", "line(d,40,30,40,60)",
    "line(d,50,30,50,60)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)", "pause(d,63)",
    "pause(d,63)", "pause(d,63)", "key(d)", "clear(d)", "line(d,30,30,60,60)",
    "line(d,60,30,30,60)", NULL
};

// The calls that should be made for diag.sketch.
char **pausesTest = (char *[]) {
    "pause(d,0)", "pause(d,0)", "pause(d,127)", "pause(d,128)", "pause(d,300)",
    "pause(d,0)", "pause(d,71469)", NULL
};

// The calls that should be made for field.sketch.
char **fieldTest = (char *[]) {
    "line(d,30,30,170,30)", "line(d,170,30,170,170)",
    "line(d,170,170,30,170)", "line(d,30,170,30,30)", NULL
};

// The calls that should be made for field.sketch.
char **lawnTest = (char *[]) {
    "cline(d,30,30,170,30,16711935)", "cline(d,170,30,170,170,16711935)",
    "cline(d,170,170,30,170,16711935)", "cline(d,30,170,30,30,16711935)",
    NULL
};
