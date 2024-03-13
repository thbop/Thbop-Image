#include <stdio.h>
#include <stdlib.h>

struct Color {
    unsigned char r, g, b;
} typedef Color;

void printColor( Color color ) {
    printf( "{ %x, %x, %x }\n", color.r, color.g, color.b );
}

struct Image {
    int width, height;
    Color* colors;
    unsigned char* pixels; // Points to the bytes representing color pointers; len = width * height
} typedef Image;






int main() {
    Color a = { 255, 56, 0 };
    printColor(a);
    
    return 0;
}