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
int grayToBinaryTreshold(IVC* src, IVC* dst, int threshold)
{
	int pos = 0;
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;
			int posDst = y * dst->bytesperline + x * dst->channels;
			if (src->data[pos] < threshold)
			{
				dst->data[posDst] = 255;
				dst->data[posDst + 1] = 255;
				dst->data[posDst + 2] = 255;
			}
			else
			{
				dst->data[posDst] = 0;
				dst->data[posDst + 1] = 0;
				dst->data[posDst + 2] = 0;
			}
		}
	}
}
OVC* binaryBlobLabelling(IVC* src, IVC* dst, int* newNLabels)
{
	int label = 1;
	int maxLabels = src->width * src->height / 4; // Estimativa conservadora
	int* labelTable = (int*)malloc(maxLabels * sizeof(int));
	if (labelTable == NULL) {
		printf("Erro ao alocar memória para a tabela de rótulos.\n");
		return NULL;
	}
	memset(labelTable, 0, maxLabels * sizeof(int));
	int min;
	int tmpLabel;
	int posA, posB, posC, posD, pos;
	int size = src->width * src->height * src->channels;

	// Zera a imagem de destino para evitar sobreposição de rótulos
	memset(dst->data, 0, size);

	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			if(y > 50 && y < src->height-50 && x > 50 && x < src->width-50)
			{
				pos = y * src->bytesperline + x * src->channels;
				posA = (y - 1) * src->bytesperline + (x - 1) * src->channels;
				posB = (y - 1) * src->bytesperline + x * src->channels;
				posC = (y - 1) * src->bytesperline + (x + 1) * src->channels;
				posD = y * src->bytesperline + (x - 1) * src->channels;

				if (src->data[pos] != 0)
				{
					// Verifica limites para evitar acesso inválido
					if (y == 0 || x == 0 || x == src->width - 1) posA = posB = posC = posD = -1;

					min = 255;
					if (posA >= 0 && dst->data[posA] != 0 && dst->data[posA] < min) min = dst->data[posA];
					if (posB >= 0 && dst->data[posB] != 0 && dst->data[posB] < min) min = dst->data[posB];
					if (posC >= 0 && dst->data[posC] != 0 && dst->data[posC] < min) min = dst->data[posC];
					if (posD >= 0 && dst->data[posD] != 0 && dst->data[posD] < min) min = dst->data[posD];

					// Novo blob detectado
					if (min == 255)
					{
						dst->data[pos] = label;
						labelTable[label] = label;
						label++;
					}
					else // Propaga o rótulo mínimo
					{
						dst->data[pos] = min;
						// Mescla os rótulos se necessário
						for (int i = 1; i < label; i++)
						{
							if (labelTable[i] == dst->data[posA] || labelTable[i] == dst->data[posB] || labelTable[i] == dst->data[posC] || labelTable[i] == dst->data[posD])
							{
								labelTable[i] = min;
							}
						}
					}
				}
			}
		}
	}

	// Remove rótulos repetidos (compactação)
	int* uniqueLabels = (int*)malloc(label * sizeof(int));
	int newLabelCount = 0;
	for (int i = 1; i < label; i++)
	{
		int currentLabel = labelTable[i];
		int isUnique = 1;
		for (int j = 0; j < newLabelCount; j++)
		{
			if (uniqueLabels[j] == currentLabel)
			{
				isUnique = 0;
				break;
			}
		}
		if (isUnique)
		{
			uniqueLabels[newLabelCount++] = currentLabel;
		}
	}
	*newNLabels = newLabelCount;

	// Aloca memória para os blobs identificados
	OVC* newBlobs = (OVC*)calloc(*newNLabels, sizeof(OVC));
	if (newBlobs != NULL)
	{
		for (int i = 0; i < *newNLabels; i++)
		{
			newBlobs[i].label = uniqueLabels[i];
		}
	}
	else
	{
		free(labelTable);
		free(uniqueLabels);
		return NULL;
	}

	free(labelTable);
	free(uniqueLabels);
	return newBlobs;
}

OVC* blobAreaPerimeter(IVC* src, OVC* blobs, int nLabels)
{
	int offset = 1;

	for (int i = 0; i < nLabels; i++)
	{
		int area = 0;
		int perimeter = 0;
		int label = blobs[i].label;
		for (int y = 0; y < src->height; y++)
		{
			for (int x = 0; x < src->width; x++)
			{
				int pos = y * src->bytesperline + x * src->channels;
				// Verificando se o pixel pertence ao blob atual
				if (src->data[pos] == label)
				{
					area++;
					// Verificando se é um pixel de borda
					bool isBorder = false;
					for (int ky = -offset; ky <= offset; ky++)
					{
						for (int kx = -offset; kx <= offset; kx++)
						{
							int nx = x + kx;
							int ny = y + ky;

							if (nx >= 0 && nx < src->width && ny >= 0 && ny < src->height)
							{
								int neighborPos = ny * src->bytesperline + nx * src->channels;
								if (src->data[neighborPos] != label)
								{
									isBorder = true;
								}
							}
						}
					}
					if (isBorder) perimeter++;
				}
			}
		}

		// Atualiza a área e perímetro do blob
		blobs[i].area = area;
		blobs[i].perimeter = perimeter;
	}
	return blobs;
}
OVC* blobCentroid(IVC* src, OVC* blobs, int* nLabels)
{
	int x = 0;
	int y = 0;
	int somX = 0;
	int somY = 0;
	int area = 0;
	for (int i = 0; i < *nLabels; i++)
	{
		area = blobs[i].area;
		for (int y = 0; y < src->height; y++)
		{
			for (int x = 0; x < src->width; x++)
			{
				int pos = y * src->bytesperline + x * src->channels;
				if (src->data[pos] == blobs[i].label)
				{
					somX += x;
					somY += y;
				}
			}
		}
		if (area != 0)blobs[i].xc = somX / area;
		if (area != 0)blobs[i].yc = somY / area;
		somX = 0;
		somY = 0;
	}
	return blobs;
}
OVC* blobBoundingBox(IVC* src, OVC* blobs, int* nLabels)
{
	for (int i = 0; i < *nLabels; i++)
	{
		int xMin = src->width;
		int xMax = 0;
		int yMin = src->height;
		int yMax = 0;

		for (int y = 0; y < src->height; y++)
		{
			for (int x = 0; x < src->width; x++)
			{
				int pos = y * src->bytesperline + x * src->channels;
				if (src->data[pos] == blobs[i].label)
				{
					if (x < xMin) xMin = x;
					if (x > xMax) xMax = x;
					if (y < yMin) yMin = y;
					if (y > yMax) yMax = y;
				}
			}
		}

		// Armazena os limites calculados no blob atual
		blobs[i].x = xMin;
		blobs[i].y = yMin;
		blobs[i].width = xMax - xMin;
		blobs[i].height = yMax - yMin;
	}
	return blobs;
}
void drawBoundingBoxAndCentroid(IVC* image, OVC* blobs, int nLabels) {
	// Cor para as bounding boxes (vermelho)
	unsigned char boxColor[3] = { 255, 0, 0 };
	// Cor para os centros de massa (verde)
	unsigned char centroidColor[3] = { 0, 255, 0 };

	for (int i = 0; i < nLabels; i++) {
		int xMin = blobs[i].x;
		int yMin = blobs[i].y;
		int width = blobs[i].width;
		int height = blobs[i].height;
		int xc = blobs[i].xc;
		int yc = blobs[i].yc;

		// Desenha o retângulo da bounding box
		for (int x = xMin; x <= xMin + width; x++) {
			int top = yMin * image->bytesperline + x * image->channels;
			int bottom = (yMin + height) * image->bytesperline + x * image->channels;

			image->data[top] = boxColor[0];
			image->data[top + 1] = boxColor[1];
			image->data[top + 2] = boxColor[2];

			image->data[bottom] = boxColor[0];
			image->data[bottom + 1] = boxColor[1];
			image->data[bottom + 2] = boxColor[2];
		}
		for (int y = yMin; y <= yMin + height; y++) {
			int left = y * image->bytesperline + xMin * image->channels;
			int right = y * image->bytesperline + (xMin + width) * image->channels;

			image->data[left] = boxColor[0];
			image->data[left + 1] = boxColor[1];
			image->data[left + 2] = boxColor[2];

			image->data[right] = boxColor[0];
			image->data[right + 1] = boxColor[1];
			image->data[right + 2] = boxColor[2];
		}

		// Desenha o ponto do centro de massa
		int pos = yc * image->bytesperline + xc * image->channels;
		image->data[pos] = centroidColor[0];
		image->data[pos + 1] = centroidColor[1];
		image->data[pos + 2] = centroidColor[2];
	}
}

int binaryDilate(IVC* src, IVC* dst, int kernel)
{
	int ky, kx, posDst;
	int offset = (kernel - 1) / 2;
	bool flag = false;

	// Initialize the destination image to all 0s (background)
	for (int y = 0; y < dst->height; y++)
	{
		for (int x = 0; x < dst->width; x++)
		{
			posDst = y * dst->bytesperline + x * dst->channels;
			dst->data[posDst] = 255;
			dst->data[posDst + 1] = 255;
			dst->data[posDst + 2] = 255;
		}
	}

	// Perform binary dilation
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			int posSrc = y * src->bytesperline + x * src->channels;
			posDst = y * dst->bytesperline + x * dst->channels;

			if (src->data[posSrc] == 0)  // Only consider non-zero pixels for dilation
			{
				for (ky = -offset; ky <= offset; ky++)
				{
					for (kx = -offset; kx <= offset; kx++)
					{
						// Check bounds to avoid accessing outside the image
						if (((y + ky) >= 0) && ((y + ky) < src->height) && ((x + kx) >= 0) && ((x + kx) < src->width))
						{
							int neighborPos = (y + ky) * src->bytesperline + (x + kx) * src->channels;
							// If any neighboring pixel is 255, set the destination pixel to 255
							if (src->data[neighborPos] == 255)
							{
								flag = true;

							}

						}
					}
				}
				if (flag == true)
				{
					dst->data[posDst] = 255;
					dst->data[posDst + 1] = 255;
					dst->data[posDst + 2] = 255;
				}
				else {
					dst->data[posDst] = 0;
					dst->data[posDst + 1] = 0;
					dst->data[posDst + 2] = 0;
				}
				flag = false;
			}
		}
	}

	return 0;  // Indicate successful completion
}
int binaryErode(IVC* src, IVC* dst, int kernel)
{
	int ky, kx, posDst;
	int offset = (kernel - 1) / 2;

	// Initialize the destination image to all 255s (foreground)
	for (int y = 0; y < dst->height; y++)
	{
		for (int x = 0; x < dst->width; x++)
		{
			posDst = y * dst->bytesperline + x * dst->channels;
			dst->data[posDst] = 0;
			dst->data[posDst + 1] = 0;
			dst->data[posDst + 2] = 0;
		}
	}

	// Perform binary erosion
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			int posSrc = y * src->bytesperline + x * src->channels;
			posDst = y * dst->bytesperline + x * dst->channels;

			if (src->data[posSrc] == 255)  // Only consider foreground pixels for erosion
			{
				bool erosionFlag = true;  // Assume erosion is true initially

				// Iterate over the kernel region
				for (ky = -offset; ky <= offset; ky++)
				{
					for (kx = -offset; kx <= offset; kx++)
					{
						// Check bounds to avoid accessing outside the image
						if (((y + ky) >= 0) && ((y + ky) < src->height) && ((x + kx) >= 0) && ((x + kx) < src->width))
						{
							int neighborPos = (y + ky) * src->bytesperline + (x + kx) * src->channels;
							// If any neighboring pixel is 0, erosion is false
							if (src->data[neighborPos] == 0)
							{
								erosionFlag = false;
							}
						}
					}
				}

				// If the flag is still true, all neighbors were foreground, so set the output pixel to 255
				if (erosionFlag)
				{
					dst->data[posDst] = 255;
					dst->data[posDst + 1] = 255;
					dst->data[posDst + 2] = 255;
				}
				else
				{
					dst->data[posDst] = 0;
					dst->data[posDst + 1] = 0;
					dst->data[posDst + 2] = 0;
				}
			}
		}
	}

	return 0;  // Indicate successful completion
}
double distanceBetweenPoints(int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	return sqrt(dx * dx + dy * dy);
}
// Função para copiar blobs
OVC* copyBlobs(OVC* srcBlobs, int nLabels) {
	if (srcBlobs == NULL || nLabels <= 0) {
		return NULL;
	}

	// Aloca memória para a nova lista de blobs
	OVC* dstBlobs = (OVC*)malloc(nLabels * sizeof(OVC));
	if (dstBlobs == NULL) {
		printf("Erro ao alocar memória para a cópia dos blobs.\n");
		return NULL;
	}

	// Copia os dados
	memcpy(dstBlobs, srcBlobs, nLabels * sizeof(OVC));

	return dstBlobs;
}

void checkCoinCounted(OVC* blobs, OVC* newBlobs, int* nLabels, int* newNLabels) {
	if (*nLabels < 0) return;
	for (int i = 0; i < *nLabels; i++) {
		for (int j = 0; j < *newNLabels; j++) {
			if (blobs != NULL && newBlobs != NULL) {
				if (distanceBetweenPoints(blobs[i].xc, blobs[i].yc, newBlobs[j].xc, newBlobs[j].yc) < 50) {
					// Verifica se a moeda já foi contada em algum frame anterior
					if (blobs[i].counted == 1)
					{
						newBlobs[j].counted = 1;
						//printf("aaaaaaa");
					}
				}
			}
		}
	}
}

OVC* detectCoinsByArea(IVC* src, OVC* blobs, int nLabels) {
	float valor = 0;
	for (int i = 0; i < nLabels; i++) {
		int area = blobs[i].area;
		int perimeter = blobs[i].perimeter;
		if (blobs[i].counted == 0 && blobs[i].yc > 200 && blobs[i].yc < src->height - 200) {
			// Verificação combinada de área e perímetro
			if (area > 7000 && area < 8000 && perimeter > 400 && perimeter < 450) {
				blobs[i].value = 0.01f; // Moeda de 1 cent
			}
			else if (area >= 9000 && area < 10000 && perimeter >= 450 && perimeter < 500) {
				blobs[i].value = 0.02f; // Moeda de 2 cents
			}
			else if (area >= 12500 && area < 13500 && perimeter >= 500 && perimeter < 550) {
				blobs[i].value = 0.05f; // Moeda de 5 cents
			}
			else if (area >= 11000 && area < 12000 && perimeter >= 500 && perimeter < 550) {
				blobs[i].value = 0.10f; // Moeda de 10 cents
			}
			else if (area >= 14000 && area < 14500 && perimeter >= 550 && perimeter < 600) {
				blobs[i].value = 0.20f; // Moeda de 20 cents
			}
			else if (area >= 18000 && area < 19000 && perimeter >= 600 && perimeter < 650) {
				blobs[i].value = 0.50f; // Moeda de 50 cents
			}
			else if (area >= 15000 && area < 16000 && perimeter >= 600 && perimeter < 650) {
				blobs[i].value = 1.00f; // Moeda de 1 euro
			}
			else if (area >= 19000 && area < 20000 && perimeter >= 650 && perimeter < 700) {
				blobs[i].value = 2.00f; // Moeda de 2 euros
			}
		}
		if (blobs[i].counted == true)
		{
			src->data[blobs[i].yc * src->bytesperline + blobs[i].xc * src->channels] = 255;
			src->data[blobs[i].yc * src->bytesperline + blobs[i].xc * src->channels + 1] = 0;
			src->data[blobs[i].yc * src->bytesperline + blobs[i].xc * src->channels + 2] = 0;
		}
	}

	return blobs;
}
