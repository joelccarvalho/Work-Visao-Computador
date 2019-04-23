#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vc.h"

int countImperfect(IVC *image[], int *colors, int xc, int yc, int kernel, int cPecaDefeituosas) {
    int pecaDefeituosas = 0;
    IVC *teste;

    teste = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    memcpy(teste->data, image[0]->data, image[0]->width * image[0]->height * image[0]->channels);
    vc_rgb_to_hsv(teste);

    pecaDefeituosas = count_imperfect(teste, xc, yc, kernel, colors);
    vc_write_image("teste.ppm", teste);

    if(pecaDefeituosas > 100) {
        cPecaDefeituosas++;
    }

    //vc_write_image("teste.ppm", teste);
    //system("open teste.ppm"); // Output

    //printf("n pixeis: %d\n", pecaDefeituosas);

    return cPecaDefeituosas;
}

// Abrir imagem em RGB, converter para HSV e gravar em novo ficheiro
void showDetailsByColor(IVC *image[], IVC *imageHSV, char *color, OVC *nObjetos, int numero, IVC *imageCentroPintado, int *colors) {
    int numeroCor = 0, c = 0, cPecaDefeituosas = 0;

    vc_rgb_to_gray(imageHSV, image[2]);
    vc_binary_open(image[2], image[3], 3);
    vc_binary_close(image[3], image[2], 5);

    OVC *nObjetosCor = vc_binary_blob_labelling(image[2], image[3], &numeroCor);
    vc_binary_blob_info(image[3], nObjetosCor, numeroCor);
    vc_write_image("imgHSV.ppm", image[2]);

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf("|%-11s|%-10s|%-10s|%-10s|%-11s|%-11s|%-10s|%-10s|%-10s|%-10s|\n", "PEÇA", "ETIQUETA", "X", "Y", "ÁREA", "PERÍMETRO", "CENTRO X", "CENTRO Y", "LARGURA", "ALTURA");
    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf("|COR: %s\n", color);
    printf("---------------------------------------------------------------------------------------------------------------\n");

    for (int k = 0; k < numeroCor; ++k) {
        for (int j = 0; j < numero; ++j) {
            if ((nObjetosCor[k].xc >= nObjetos[j].x && nObjetosCor[k].xc <= nObjetos[j].x + nObjetos[j].width) &&
            (nObjetosCor[k].yc >= nObjetos[j].y && nObjetosCor[k].yc <= nObjetos[j].y + nObjetos[j].height))
            {
                if (nObjetosCor[k].area > 200 && nObjetos[j].area > 200) {
                    cPecaDefeituosas = countImperfect(image, colors, nObjetos[j].xc, nObjetos[j].yc, 10, cPecaDefeituosas);
                    c++;
                    printf("|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|\n", c, nObjetos[j].label, nObjetos[j].x, nObjetos[j].y, nObjetos[j].area, nObjetos[j].perimeter, nObjetos[j].xc, nObjetos[j].yc, nObjetos[j].width, nObjetos[j].height);
                    paint_center(image[0], imageCentroPintado, nObjetos[j].xc, nObjetos[j].yc, 10);
                    draw_box(image[0], imageCentroPintado, nObjetos[j].x, nObjetos[j].y, nObjetos[j].width, nObjetos[j].height, 5);
                }
            }
        }
    }

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf("|TOTAL DE DEFEITUOSAS: %d\n", cPecaDefeituosas);
    printf("|TOTAL DE %s: %d\n", color, (c-cPecaDefeituosas));
    printf("---------------------------------------------------------------------------------------------------------------\n");
}


int processImage(IVC *image[]) {
    OVC *nObjetos;
    IVC *imageHSVHelp, *imageCentroPintado;
    int numero, i, c = 0, j, k;
    int *colors;

    image[1] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    image[2] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    image[3] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);

    // Copiar conteúdo do imagem original para a posição 1
    memcpy(image[1]->data, image[0]->data, image[0]->width * image[0]->height * image[0]->channels);

    vc_rgb_to_hsv(image[1]);

    imageHSVHelp = vc_image_new(image[1]->width, image[1]->height, image[1]->channels, image[1]->levels);
    memcpy(imageHSVHelp->data, image[1]->data, image[1]->width * image[1]->height * image[1]->channels);

    vc_hsv_segmentation(image[1], 10, 45, 0, 40, 60, 90); // remover fundo e ver peças a branco
    vc_rgb_to_gray(image[1], image[2]);
    vc_gray_negative(image[2]);
    vc_binary_close(image[2], image[3], 3);

    nObjetos = vc_binary_blob_labelling(image[3], image[2], &numero);
    vc_binary_blob_info(image[2], nObjetos, numero);

    printf("-------------------------------------GLOBAL--------------------------------------------------------------------\n");
    printf("|%-11s|%-10s|%-10s|%-10s|%-11s|%-11s|%-10s|%-10s|%-10s|%-10s|\n", "PEÇA", "ETIQUETA", "X", "Y", "ÁREA", "PERÍMETRO", "CENTRO X", "CENTRO Y", "LARGURA", "ALTURA");
    printf("---------------------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < numero; i++) {
        if (nObjetos[i].area > 200) {
            c++;
            printf("|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|%-10d|\n", c, nObjetos[i].label, nObjetos[i].x, nObjetos[i].y, nObjetos[i].area, nObjetos[i].perimeter, nObjetos[i].xc, nObjetos[i].yc, nObjetos[i].width, nObjetos[i].height);
        }
    }
    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf("|TOTAL DE PEÇAS: %d\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t  |\n", c);
    printf("-------------------------------------FIM GLOBAL----------------------------------------------------------------\n\n\n");

    // Declarar array de imagens HSV a gerar
    IVC *imageHSV[c];

    // Criar imagens HSV para cada cor
    for (j = 0; j < c; j++)
    {
        // Guardar imagem HSV
        imageHSV[j] = vc_image_new(imageHSVHelp->width, imageHSVHelp->height, imageHSVHelp->channels, imageHSVHelp->levels);
        // Copiar conteúdo do imagem 1 para a imageHSV
        memcpy(imageHSV[j]->data, imageHSVHelp->data, imageHSVHelp->width * imageHSVHelp->height * imageHSVHelp->channels);
    }

    // Guardar imagem original para pintar o centro numa nova
    imageCentroPintado = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    memcpy(imageCentroPintado->data, image[0]->data, image[0]->width * image[0]->height * image[0]->channels);

    // Mostrar os detalhes por cada cor
    colors = vc_hsv_segmentation(imageHSV[0], 30, 50, 35, 55, 80, 100); // Big Blind
    showDetailsByColor(image, imageHSV[0], "Big Blind", nObjetos, numero, imageCentroPintado, colors);

    colors = vc_hsv_segmentation(imageHSV[1], 245, 255, 40, 50, 50, 70); // Small Blind
    showDetailsByColor(image, imageHSV[1], "Small Blind", nObjetos, numero, imageCentroPintado, colors);

    colors = vc_hsv_segmentation(imageHSV[2], 340, 360, 50, 60, 65, 80); // Vermelho
    showDetailsByColor(image, imageHSV[2], "Vermelho", nObjetos, numero, imageCentroPintado, colors);

    colors = vc_hsv_segmentation(imageHSV[3], 180, 218, 70, 100, 40, 70); // Azul
    showDetailsByColor(image, imageHSV[3], "Azul", nObjetos, numero, imageCentroPintado, colors);

    colors = vc_hsv_segmentation(imageHSV[4], 190, 210, 0, 20, 80, 95); // Branca
    showDetailsByColor(image, imageHSV[4], "Branco", nObjetos, numero, imageCentroPintado, colors);

    colors = vc_hsv_segmentation(imageHSV[5], 205, 220, 20, 80, 20, 45); // Preta
    showDetailsByColor(image, imageHSV[5], "Preto", nObjetos, numero, imageCentroPintado, colors);

    // Mostrar imagem com o centro pintado
    vc_write_image("centroPintado.ppm", imageCentroPintado);

    vc_image_free(imageCentroPintado);
    vc_image_free(image[0]);
    vc_image_free(image[1]);
    vc_image_free(image[2]);
    vc_image_free(image[3]);
    vc_image_free(imageHSVHelp);


    for (k = 0; k < c; k++)
    {
        vc_image_free(imageHSV[k]);
    }
}

// Processar imagens
int main(void)
{
    int n = 4;
    IVC *image[n];

    image[0] = vc_read_image("../Imagens/Imagem10.ppm");
    if (image[0] == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    processImage(image);

    system("open ../Imagens/Imagem10.ppm"); // Input
    system("open imgHSV.ppm"); // Output
    system("open centroPintado.ppm"); // Output

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}