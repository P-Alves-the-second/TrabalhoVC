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
	int counted;
	float value;
} OVC;

IVC* freeImage(IVC* image);
IVC* newImage(int width, int height, int channels, int levels);

OVC* binaryBlobLabelling(IVC* src, IVC* dst,int* newNLabels);
OVC* blobAreaPerimeter(IVC* src, OVC* blobs, int nLabels);
OVC* blobBoundingBox(IVC* src, OVC* blobs, int* nLabels);
OVC* blobCentroid(IVC* src, OVC* blobs, int* nLabels);
OVC* copyBlobs(OVC* srcBlobs, int nLabels);

OVC* detectCoinsByArea(IVC* src, OVC* blobs, int nLabels);
int grayToBinaryTreshold(IVC* src, IVC* dst, int threshold);
int binaryDilate(IVC* src, IVC* dst, int kernel);
int binaryErode(IVC* src, IVC* dst, int kernel);
void drawBoundingBoxAndCentroid(IVC* image, OVC* blobs, int nLabels);
void checkCoinCounted(OVC* blobs, OVC* newBlobs, int* nLabels, int* newNLabels);
double distanceBetweenPoints(int x1, int y1, int x2, int y2);



