#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C" {
#include "vc.h"
}


void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}


int main(void) {
	// Vídeo
	char videofile[20] = "video2.mp4";
	OVC* blobs = NULL;
	OVC* newBlobs = NULL;
	int nLabels = 0;
	int newNLabels = 0;
	float total = 0;
	int framer = 0;
	int modeas[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	float valores[] = { 0.01f, 0.02f, 0.05f, 0.10f, 0.20f, 0.50f, 1.00f, 2.00f };
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de vídeo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi deverá estar localizado no mesmo directório que o ficheiro de código fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
		return 1;
	}

	/* Número total de frames no vídeo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do vídeo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolução do vídeo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	cv::Mat frame;
	while (key != 'q') {
		/* Leitura de uma frame do vídeo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		framer++;
		//printf("Frame: %d\n", framer);
		IVC* image = newImage(video.width, video.height, 3, 256);
		IVC* gray = newImage(video.width, video.height, 3, 256);
		IVC* binary = newImage(video.width, video.height, 3, 256);
		IVC* dilateted = newImage(video.width, video.height, 3, 256);
		IVC* eroded = newImage(video.width, video.height, 3, 256);
		IVC* labeled = newImage(video.width, video.height, 3, 256);
		IVC* original = newImage(video.width, video.height, 3, 256);

		memcpy(image->data, frame.data, video.width* video.height * 3);
		memcpy(original->data, frame.data, video.width* video.height * 3);
		grayToBinaryTreshold(original, binary, 100);
		binaryDilate(binary, dilateted, 3);
		binaryErode(dilateted, eroded, 19);
		newBlobs = binaryBlobLabelling(eroded,labeled,&newNLabels);

		for(int i = 0;i < newNLabels;i++)
		{
			newBlobs[i].counted = 0;
		}
		newBlobs = blobAreaPerimeter(labeled, newBlobs, newNLabels);
		newBlobs = blobCentroid(labeled, newBlobs, &newNLabels);
		newBlobs = blobBoundingBox(labeled, newBlobs, &newNLabels);
		checkCoinCounted(blobs, newBlobs, &nLabels, &newNLabels);
		newBlobs = detectCoinsByArea(original, newBlobs, newNLabels);
		drawBoundingBoxAndCentroid(original, newBlobs, newNLabels);

		for(int i = 0;i < newNLabels;i++)
		{
			if (newBlobs[i].counted != 1 && newBlobs[i].value != 0)
			{
				total += newBlobs[i].value;
				newBlobs[i].counted = 1;
				printf("%f\n",newBlobs[i].value);
				if (newBlobs[i].value == 0.01f)
				{
					modeas[0]++;
				}
				else if (newBlobs[i].value == 0.02f)
				{
					modeas[1]++;
				}
				else if (newBlobs[i].value == 0.05f)
				{
					modeas[2]++;
				}
				else if (newBlobs[i].value == 0.10f)
				{
					modeas[3]++;
				}
				else if (newBlobs[i].value == 0.20f)
				{
					modeas[4]++;
				}
				else if (newBlobs[i].value == 0.50f)
				{
					modeas[5]++;
				}
				else if (newBlobs[i].value == 1.00f)
				{
					modeas[6]++;
				}
				else if (newBlobs[i].value == 2.00f)
				{
					modeas[7]++;
				}
			}
		}

		//printf("Total: %.2f\n", total);
		memcpy(frame.data, original->data, video.width* video.height * 3);
		freeImage(image);

		blobs = copyBlobs(newBlobs, newNLabels);
		free(newBlobs);
		nLabels = newNLabels;


		/* Exemplo de inserção texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);


		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	for(int i = 0; i < 8; i++){
		if (modeas[i] != 0)
		{
			printf("Moeda de %.2f: %d\n", valores[i], modeas[i]);
		}
	}
	printf("Total: %.2f\n", total);
	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	return 0;
}
