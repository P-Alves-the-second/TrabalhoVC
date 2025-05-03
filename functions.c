#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>
#include <stdbool.h>

// Alocar memória para uma imagem
IVC* newImage(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 256)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return freeImage(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC* freeImage(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

int rgbToGray(IVC* src, IVC* dst)
{
	float red, green, blue, med;

	if (src->channels != 3 || dst->channels != 3) return -1;
	if (src->width != dst->width || src->height != dst->height) return -1;
	if (src == NULL) return -1;
	if (dst == NULL) return -1;
	
	for (int x = 0; x < src->width; x++)
	{
		for (int y = 0; y < src->height; y++)
		{
			int pos = x * src->channels + y * src->bytesperline;

			red = src->data[pos] * 0.299;
			green = src->data[pos + 1] * 0.587;
			blue = src->data[pos + 2] * 0.114;
			med = (red + blue + green);

			int posDst = x * dst->channels + y * dst->bytesperline;
			dst->data[posDst] = med;
			dst->data[posDst + 1] = med;
			dst->data[posDst + 2] = med;
		}
	}
	return 0;
}