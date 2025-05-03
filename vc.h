typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=2; Cinzentos [1,256]; RGB [1,256]
	int bytesperline;		// width * channels
} IVC;

typedef struct {
	int x, y, width, height; // Bounding box
	int area;				 // Area
	int xc, yc;				 // Centro de massa
	int perimeter;			 // Perimetro
	int label;				 // Etiqueta
} OVC;

IVC* freeImage(IVC* image);
IVC* newImage(int width, int height, int channels, int levels);

int rgbToGray(IVC* src, IVC* dst);

