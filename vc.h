//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
    int area;					// �rea
    int xc, yc;					// Centro-de-massa
    int perimeter;				// Per�metro
    int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);
int vc_rgb_to_gray(IVC *src, IVC *dst);
int *vc_hsv_segmentation(IVC *srcdst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_rgb_to_hsv(IVC *src);
int vc_gray_negative(IVC *srcdst);
int vc_binary_open(IVC *src, IVC *dst, int kernel);
int vc_binary_close(IVC *src, IVC *dst, int kernel);
int vc_binary_dilate(IVC *src, IVC *dst, int kernel);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);
int paint_center(IVC *src, IVC *dst, int xc, int yc, int kernel);
int draw_box(IVC *src, IVC *dst, int posx, int posy, int w, int h, int kernel);
int count_imperfect(IVC *src, int xc, int yc, int kernel, int *colors);

// BLOBS
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);