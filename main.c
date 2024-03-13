#include <stdio.h>
#include <stdlib.h>




struct Color {
    unsigned char r, g, b;
} typedef Color;

void PrintColor( Color color ) {
    printf( "{ %x, %x, %x }\n", color.r, color.g, color.b );
}

struct Image {
    int
        width, height,
        paletteSize; // Amount of different colors
    Color* palette;
    unsigned char* pixels; // Points to the bytes representing color pointers; len = width * height
} typedef Image;

Image GenerateImage( int width, int height, Color color ) {
    Image img;
    img.width = width;
    img.height = height;

    // We're just doing one because we're generating a single color image
    img.palette = (Color*)malloc( sizeof(Color) );
    img.paletteSize = 1;
    *img.palette = color;

    // calloc sets all allocated memory to zero pointing to the first color.
    img.pixels = (unsigned char*)calloc( width * height, sizeof(unsigned char) );

    return img;
}

Color FetchColor( Image img, int x, int y ) {
    if ( x < 0 || x > img.width || y < 0 || y > img.height ) {
        printf(
            "WARNING: Trying to fetch pixel ( %d, %d ) out of bounds ( %d, %d )!\n",
            x, y, img.width, img.height
        );
        return (Color){0};
    }
    
    return img.palette[img.pixels[x + y * img.width]];
}


int ExportImage( Image img, unsigned char* fileName ) {
    FILE* fp = fopen( fileName, "wb" );
    unsigned char thbop[] = "thbop";

    // Metadata
    if (fwrite(thbop, sizeof(unsigned char), sizeof(thbop)-1, fp) != sizeof(thbop)-1 ||
        fwrite(&img.width,  sizeof(int), 1, fp) != 1 ||
        fwrite(&img.height, sizeof(int), 1, fp) != 1 )        { fclose(fp); return 1; }

    // Colors
    if ( fwrite(img.palette, sizeof(Color), img.paletteSize, fp) != img.paletteSize ) { fclose(fp); return 2; }

    // Pixels
    if ( fwrite(img.pixels, sizeof(unsigned char), img.width * img.height, fp) != img.width * img.height ) { fclose(fp); return 3; }

    fclose(fp);
    return 0;
}

void UnloadImage( Image img ) {
    free( img.palette );
    free( img.pixels );
}


int main() {
    Image img = GenerateImage( 100, 20, (Color){ 0xFF, 0, 0 } );

    printf( "Error Code: %d\n", ExportImage( img, "test.ti" ) );

    UnloadImage(img);

    return 0;
}