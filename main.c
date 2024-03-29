#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


struct Color {
    unsigned char r, g, b;
} typedef Color;

void PrintColor( Color color ) {
    printf( "{ %x, %x, %x }\n", color.r, color.g, color.b );
}
COLORREF ToWinColor( Color color ) {
    return RGB( color.r, color.g, color.b );
}

struct Image {
    unsigned short
        width, height;
    unsigned char paletteSize; // Amount of different colors
    Color* palette;
    unsigned char* pixels; // Points to the bytes representing color pointers; len = width * height
} typedef Image;

Image GenerateColorImage( unsigned short width, unsigned short height, Color color ) {
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

double frac( double v ) {
    return v - floor(v);
}

double lerp( double a, double b, double v ) {
    return ( b - a ) * v + a;
}

double WhiteNoise( int seed, int x, int y ) {
    // Generates white noise at a given location
    return frac( sin( x * 2458.234 + y * 2348.8985 ) * 3432.2543 * seed );
}



Color RandomColor( int seed, int x, int y ) {
    return (Color){
        floor(WhiteNoise( seed,   x, y ) * 255),
        floor(WhiteNoise( seed+1, x, y ) * 255),
        floor(WhiteNoise( seed+2, x, y ) * 255)
    };
}

int RandomInt( int seed, int x, int y, int mn, int mx ) {
    return floor(WhiteNoise(seed, x, y) * (mx-mn) + mn );
}

Image GenerateNoiseImage( unsigned short width, unsigned short height, unsigned char paletteSize ) {
    Image img;
    img.width = width;
    img.height = height;

    img.palette = (Color*)malloc( sizeof(Color) * paletteSize );
    img.paletteSize = paletteSize;
    for (unsigned char i = 0; i < paletteSize; i++)
        img.palette[i] = RandomColor( 32, i, i+83 );
    

    // calloc sets all allocated memory to zero pointing to the first color.
    img.pixels = (unsigned char*)calloc( width * height, sizeof(unsigned char) );
    for ( int i = 0; i < img.width*img.height; i++ )
        img.pixels[i] = (unsigned char)RandomInt(390, i, i+257, 0, paletteSize);

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
        fwrite(&img.width,  sizeof(unsigned short), 1, fp) != 1 ||
        fwrite(&img.height, sizeof(unsigned short), 1, fp) != 1 ) { fclose(fp); return 1; }

    // Colors
    if (fputc(img.paletteSize, fp) == EOF ||
        fwrite(img.palette, sizeof(Color), img.paletteSize, fp) != img.paletteSize)
        { fclose(fp); return 2; }

    // Pixels
    if ( fwrite(img.pixels, sizeof(unsigned char), img.width * img.height, fp) != img.width * img.height ) { fclose(fp); return 3; }

    fclose(fp);
    return 0;
}

long int TIGetFileSize( FILE* fp ) {
    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size;
}



Image ImportImage( unsigned char* fileName ) {
    FILE* fp = fopen( fileName, "rb" );
    if (fp == NULL) {
        printf( "ERROR: Could not open \"%s\"!", fileName );
        return (Image){0};
    }

    Image img;
    long int size = TIGetFileSize(fp);
    fseek(fp, 5, SEEK_SET); // Ignore "thbop" header

    if ( fread(&img.width,  sizeof(unsigned short), 1, fp) != 1 ) { fclose(fp); return (Image){0}; }
    if ( fread(&img.height, sizeof(unsigned short), 1, fp) != 1 ) { fclose(fp); return (Image){0}; }

    if ( fread(&img.paletteSize, sizeof(unsigned char), 1, fp) != 1 ) { fclose(fp); return (Image){0}; }

    img.palette = (Color*)malloc( img.paletteSize * sizeof(Color) );
    if ( fread(img.palette, sizeof(Color), img.paletteSize, fp) != img.paletteSize ) { fclose(fp); return (Image){0}; }

    img.pixels = (unsigned char*)calloc( img.width * img.height, sizeof(unsigned char) );
    if ( fread(img.pixels, sizeof(unsigned char), img.width * img.height, fp) != img.width * img.height ) { fclose(fp); return (Image){0}; }

    

    fclose(fp);
    return img;
}

void UnloadImage( Image img ) {
    if ( img.palette != NULL ) free( img.palette );
    if ( img.pixels  != NULL ) free( img.pixels  );
}



// int main() {
    // Image img = GenerateImage( 100, 50, (Color){ 0xFF, 0x56, 0x45 } );

    // printf( "Error Code: %d\n", ExportImage( img, "test.ti" ) );
    // UnloadImage(img);
    
//     Image img = ImportImage("test.ti");
//     printf( "Error Code: %d\n", ExportImage( img, "test2.ti" ) );

//     UnloadImage(img);

//     return 0;
// }

Image img;

LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            for ( unsigned short j = 0; j < img.height; j++ ) {
                for ( unsigned short i = 0; i < img.width; i++ ) {
                    SetPixel(hdc, i, j, ToWinColor(FetchColor(img, i, j)));
                }
            }

            EndPaint(hWnd, &ps);
            return 0;
        } case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } default:
            return DefWindowProc( hWnd, msg, wParam, lParam );
    }
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
    // Load image
    // img = GenerateNoiseImage( 500, 500, 100 );
    // ExportImage(img, "image.ti");
    img = ImportImage("t.ti");


    // Window class registration
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(2);
    wc.lpszClassName = "ThbopImageViewer";

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window registration failed!", "Error", MB_ICONEXCLAMATION);
        return 0;
    }

    // Window creation
    HWND hWnd = CreateWindowEx(
        0, "ThbopImageViewer", "Thbop Image Viewer",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        img.width, img.height, NULL, NULL, hInstance, NULL
    );

    if ( hWnd == NULL ) {
        MessageBox( NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION );
        return 0;
    }

    // Show the window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Message loop
    MSG msg;
    while ( GetMessage(&msg, NULL, 0, 0) ) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnloadImage(img);
    return (int)msg.wParam;
}