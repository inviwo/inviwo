
/*  Definitions for bitflags  */
typedef unsigned int tgaFLAG;

#define TGA_DEFAULT            0x0000000000000000   /* In case we don't want any parameters */
#define TGA_FREE               0x0000000000000001   /* Bit flag 0 */
#define TGA_NO_PASS            0x0000000000000010   /* Bit flag 1 */
#define TGA_ALPHA              0x0000000000000100   /* Bit flag 2 */
#define TGA_LUMINANCE          0x0000000000001000   /* Bit flag 3 */
#define TGA_NO_MIPMAPS         0x0000000000010000   /* Bit flag 4 */
#define TGA_LOW_QUALITY        0x0000000000100000   /* Bit flag 5 */
#define TGA_COMPRESS           0x0000000001000000   /* Bit flag 6 */


/*
** GL_ARB_texture_compression
**
** Support:
**  GeForce
**  Radeon
**  ????   <- Any suggestions?
*/
#ifndef GL_ARB_texture_compression
#define GL_ARB_texture_compression 1

#define GL_COMPRESSED_ALPHA_ARB					 0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB				 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB		 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB				 0x84EC
#define GL_COMPRESSED_RGB_ARB				   	 0x84ED
#define GL_COMPRESSED_RGBA_ARB					 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB	    0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB	 0x86A0
#define GL_TEXTURE_COMPRESSED_ARB				 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB		 0x86A3

typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)(GLenum target, GLint level, 
													   GLenum internalFormat, GLsizei width,
													   GLsizei height, GLsizei depth,
													   GLint border, GLsizei imageSize,
													   const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)(GLenum target, GLint level,
													   GLenum internalFormat, GLsizei width,
													   GLsizei height, GLint border,
													   GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)(GLenum target, GLint level,
													   GLenum internalFormat, GLsizei width,
													   GLint border, GLsizei imageSize,
													   const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)(GLenum target, GLint level,
														  GLint xoffset, GLint yoffset,
														  GLint zoffset, GLsizei width,
														  GLsizei height, GLsizei depth,
														  GLenum format, GLsizei imageSize,
														  const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)(GLenum target, GLint level,
														  GLint xoffset, GLint yoffset,
														  GLsizei width, GLsizei height,
														  GLenum format, GLsizei imageSize,
														  const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)(GLenum target, GLint level,
														  GLint xoffset, GLsizei width,
														  GLenum format, GLsizei imageSize,
														  const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)(GLenum target, GLint lod, 
														GLvoid *img);
														

#endif /* GL_ARB_texture_compression */



typedef struct {
   unsigned char id_length;
   unsigned char colour_map_type;
   unsigned char image_type;

   // colourmap spec.  5 bytes
   short int     colour_map_first_entry;  // Ignore
   short int     colour_map_length;       // Usually 256
   unsigned char colour_map_entry_size;   // Usually 24-bit

   // image spec.  10 bytes
   short int     x_origin;  // Ignore
   short int     y_origin;  // Ignore
   short int     width;
   short int     height;
   unsigned char pixel_depth;       // Usually 24 or 32
   unsigned char image_descriptor;  // Ignore

   // Added for 'compeletness' :)
   int   components;
   int   bytes;

   GLenum tgaColourType;

} tgaHeader_t;


typedef struct {
   tgaHeader_t          info;
   unsigned char        *data;      /* Image data */
} image_t;


/* 'Public' functions */
void   tgaLoad        ( char *file_name, image_t *p, tgaFLAG mode );
GLuint tgaLoadAndBind ( char *file_name, tgaFLAG mode );

void tgaSetTexParams  ( unsigned int min_filter, unsigned int mag_filter, unsigned int application );

void tgaFree ( image_t *p );
