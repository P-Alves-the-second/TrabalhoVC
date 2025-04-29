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


// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

// AULA
int vc_rgb_to_gray(IVC* srcdst);
int vc_rgb_to_hsv(IVC* src, IVC* dst);
int vc_scale_gray_to_color_pallete(IVC* src, IVC* dst);
int vc_gray_to_binary(IVC* src, int threshold);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernelSize);
int vc_gray_to_binary_nibblack(IVC* src, IVC* dst, int kernelSize);
int vc_gray_to_binary_threshold(IVC* src, IVC* dst, int treshold);
int vc_gray_to_binary_minmax(IVC* src, IVC* dst, int min, int max);
int vc_draw_blob(IVC* dst, OVC* blobs, int* nLabels);
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernel);
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nLabels);
OVC* vc_blob_AreaPerimeter(IVC* src, OVC* blobs, int* nLabels);
OVC* vc_blob_Centroid(IVC* src, OVC* blobs, int* nLabels);
OVC* vc_blob_BoundingBox(IVC* src, OVC* blobs, int* nLabels);
IVC* vc_gray_histogram_show(IVC* src);