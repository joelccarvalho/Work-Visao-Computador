#define _CRT_SECURE_NO_WARNINGS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "vc.h"
#include <math.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}

	t = tok;

	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if(c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}

		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

// Converter de RGB para Gray
int vc_rgb_to_gray(IVC *src, IVC *dst) {
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char *) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    float  rf, gf, bf;

    // Verificação de erros
    if((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return  0;
    if((src->width != dst->width) || (src->height != dst->height)) return  0;
    if((src->channels != 3) || (dst->channels != 1)) return 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            rf = (float) datasrc[pos_src];
            gf = (float) datasrc[pos_src + 1];
            bf = (float) datasrc[pos_src + 2];

            datadst[pos_dst] = (unsigned char) ((rf * 0.299) + (gf * 0.587) + (bf * 0.114));

        }
    }

    return 1;

}

// Conversão de RGB para HSV
int vc_rgb_to_hsv(IVC *src)
{
    unsigned char *data = (unsigned char *)src->data;
    int width = src->width;
    int height = src->height;
    int channels = src->channels;
    float r, g, b, hue, saturation, value;
    float rgb_max, rgb_min;
    int i, size;

    // Verificação de erros
    if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
    if (channels != 3) return 0;

    size = width * height * channels;

    for (i = 0; i < size; i = i + channels)
    {
        r = (float)data[i];
        g = (float)data[i + 1];
        b = (float)data[i + 2];

        // Calcula valores máximo e mínimo dos canais de cor R, G e B
        rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
        rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

        // Value toma valores entre [0,255]
        value = rgb_max;
        if (value == 0.0f)
        {
            hue = 0.0f;
            saturation = 0.0f;
        }
        else
        {
            // Saturation toma valores entre [0,255]
            saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

            if (saturation == 0.0f)
            {
                hue = 0.0f;
            }
            else
            {
                // Hue toma valores entre [0,360]
                if ((rgb_max == r) && (g >= b))
                {
                    hue = 60.0f * (g - b) / (rgb_max - rgb_min);
                }
                else if ((rgb_max == r) && (b > g))
                {
                    hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
                }
                else if (rgb_max == g)
                {
                    hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
                }
                else /* rgb_max == b*/
                {
                    hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
                }
            }
        }

        // Atribui valores entre [0,255]
        data[i]     = (unsigned char) (hue / 360.0f * 255.0f);
        data[i + 1] = (unsigned char) (saturation);
        data[i + 2] = (unsigned char) (value);
    }

    return 1;
}

int paint_center(IVC *src, IVC *dst, int xc, int yc, int kernel){
    unsigned char *data = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int channels = src->channels;
    int bytesperline= src->width * src->channels;
    long int posk;
    int offset = kernel / 2;
    int x, y, kx, ky;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if (channels != 3) return 0;

    // Percorrer img e respetivos vizinhos
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
			for (kx = -offset; kx <= offset; ++kx) {
				for (ky = -offset; ky <= offset; ++ky) {
					// Para não percorer pixeis fora da img
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) {
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						// Se encontrar o centro, pintá-lo de vermelho
						if (x == xc && y == yc)
			   			{
			       			data[posk]     = 255;
			   				data[posk + 1] = 0;
				            data[posk + 2] = 0;
                        }
              		}
              	}
			}
        }
    }

	return 1;
}

int draw_box(IVC *src, IVC *dst, int posx, int posy, int w, int h, int kernel, char *color){
    unsigned char *data = (unsigned char *)dst->data;
    int channels = src->channels;
    int bytesperline= src->width * src->channels;
    long int posk;
    int offset = kernel / 2;
    int x, y, kx, ky;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if (channels != 3) return 0;

    // Percorrer peças e respetivos vizinhos
    for (y = posy; y < (posy+h); ++y) {
        for (x = posx; x < (posx+w); ++x) {
            for (kx = -offset; kx <= offset; ++kx) {
                for (ky = -offset; ky <= offset; ++ky) {
                    // Para não percorer pixeis fora das peças
                    if ((y + ky >= posy) && (y + ky < (posy+h)) && (x + kx >= posx) && (x + kx < (posx+w))) {
                        posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (x <= posx+offset || y <= posy+offset || x > (posx-offset+w) || y > (posy-offset+h))
                        {

							if (strcmp(color, "Vermelho") == 0)
							{
								data[posk]     = 255;
								data[posk + 1] = 0;
								data[posk + 2] = 0;
							}
							else if (strcmp(color, "Preto") == 0)
							{
								data[posk]     = 0;
								data[posk + 1] = 0;
								data[posk + 2] = 0;
							}
							else if (strcmp(color, "Big Blind") == 0)
							{
								data[posk]     = 91;
								data[posk + 1] = 84;
								data[posk + 2] = 53;
							}
							else if (strcmp(color, "Small Blind") == 0)
							{
								data[posk]     = 50;
								data[posk + 1] = 0;
								data[posk + 2] = 85;
							}
							else if (strcmp(color, "Azul") == 0)
							{
								data[posk]     = 5;
								data[posk + 1] = 0;
								data[posk + 2] = 100;
							}
							else if (strcmp(color, "Ciano") == 0)
							{
								data[posk]     = 3;
								data[posk + 1] = 58;
								data[posk + 2] = 66;
							}
							else if (strcmp(color, "Branco") == 0)
							{
								data[posk]     = 255;
								data[posk + 1] = 255;
								data[posk + 2] = 255;
							}
                        }
                    }
                }
            }
        }
    }
}

int count_imperfect(IVC *src, int xc, int yc, int kernel, int *colors) {
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int bytesperline= src->width * src->channels;
	long int posk;
	int offset = kernel / 2;
	int x, y, kx, ky;
	int h, s, v;
	int neighbors_account = 0;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Percorrer img e respetivos vizinhos
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			for (kx = -offset; kx <= offset; ++kx) {
				for (ky = -offset; ky <= offset; ++ky) {
					// Para não percorer pixeis fora da img
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) {
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						// Encontrar centro
						if (x == xc && y == yc) {

							// Converter
							h = ((float) data[posk]) / 255.0f * 360.0f;
							s = ((float) data[posk + 1]) / 255.0f * 100.0f;
							v = ((float) data[posk + 2]) / 255.0f * 100.0f;

							// Vizinhos são da mesma cor
							if ((h > *(colors + 0)) && (h <= *(colors + 1)) && (s >= *(colors + 2)) && (s <= *(colors + 3)) && (v >= *(colors + 4)) && (v <= *(colors + 5))) {
								data[posk]     = 0;
								data[posk + 1] = 0;
								data[posk + 2] = 0;
							}
							else { // Vizinhos de cores diferentes
                                neighbors_account++;
								data[posk]     = 255;
								data[posk + 1] = 0;
								data[posk + 2] = 0;
							}
						}
					}
				}
			}
		}
	}

	return neighbors_account;
}

// hmin,hmax = [0, 360]; smin,smax = [0, 100]; vmin,vmax = [0, 100]
int *vc_hsv_segmentation(IVC *src, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int bytesperline_src = src->width * src->channels;
	int h, s, v; // h=[0, 360] s=[0, 100] v=[0, 100]
	long int pos_src;
	int static colors[6];

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Percorrer img
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			pos_src = y * bytesperline_src + x * channels;

			// Converter
			h = ((float) data[pos_src]) / 255.0f * 360.0f;
			s = ((float) data[pos_src + 1]) / 255.0f * 100.0f;
			v = ((float) data[pos_src + 2]) / 255.0f * 100.0f;

			// Peças a branco
			if ((h > hmin) && (h <= hmax) && (s >= smin) && (s <= smax) && (v >= vmin) && (v <= vmax)) {
				data[pos_src]     = 255;
				data[pos_src + 1] = 255;
				data[pos_src + 2] = 255;
			} else // Pintar a preto
			{
				data[pos_src]     = 0;
				data[pos_src + 1] = 0;
				data[pos_src + 2] = 0;
			}
		}
	}

    colors[0] = hmin; colors[1] = hmax; colors[2] = smin; colors[3] = smax; colors[4] = vmin; colors[5] = vmax;
	return colors;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel) {
	int ret = 1; // Correu tudo bem

	IVC *temp = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &=  vc_binary_erode(src, temp, kernel);  // -> ret = ret & função; se ret == 1 e função == 0, então na proxima função vai retornar 0 pq o ret já é 0
	ret &=  vc_binary_dilate(temp, dst, kernel);

	vc_image_free(temp);

	return ret;
}

int vc_gray_negative(IVC *srcdst){
	unsigned char *data = (unsigned char *) srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x,y;
	long int pos;

	//check errors
	if((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	//inverte a image Gray
	for(y = 0; y < height; y++){
		for( x = 0; x < width; x++){
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
		}
	}
}

int vc_binary_close(IVC *src, IVC *dst, int kernel) {
	int ret = 1;

	IVC *temp = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &=  vc_binary_dilate(src, temp, kernel);
	ret &=  vc_binary_erode(temp, dst, kernel);

	vc_image_free(temp);

	return ret;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel) {
	// kernel: é a quantidade de pixeis à volta do local que nos encontramos
	unsigned char *datasrc = (unsigned char *) src->data;
	unsigned char *datadst = (unsigned char *) dst->data;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;
	int width = src->width;
	int height = src->height;
	int x, y, kx, ky;
	long int pos;
	long int posk; // posição do vizinho
	int offset = kernel / 2;
	int temp;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Percorre a imagem VIZINHOS
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;
			temp = 0;
			for (kx = -offset; kx <= offset; ++kx) {
				for (ky = -offset; ky <= offset; ++ky) {
					// Para não percorer pixies fora da img
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (datasrc[posk] == 255) {
							temp = 255;
						}

					}
				}
			}

			// Se houver vizinhos com 255 então o centro passa a 255
			if (temp == 255)
				datadst[pos] = 255;
			else
				datadst[pos] = 0;
		}
	}

	return 1;
}

int vc_binary_erode(IVC *src, IVC *dst, int kernel) {
	// kernel: é a quantidade de pixeis à volta do local que nos encontramos
	unsigned char *datasrc = (unsigned char *) src->data;
	unsigned char *datadst = (unsigned char *) dst->data;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;
	int width = src->width;
	int height = src->height;
	int x, y, kx, ky;
	long int pos;
	long int posk; // posição do vizinho
	int offset = kernel / 2;
	int temp;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Percorre a imagem VIZINHOS
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;
			temp = 255;
			for (kx = -offset; kx <= offset; ++kx) {
				for (ky = -offset; ky <= offset; ++ky) {
					// Para não percorer pixies fora da img
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (datasrc[posk] == 0) {
							temp = 0;
						}

					}
				}
			}

			// Se houver vizinhos com 0 então o centro passa a 0
			if (temp == 0)
				datadst[pos] = 0;
			else
				datadst[pos] = 255;
		}
	}

	return 1;
}

OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

