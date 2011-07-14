#include "s2tc_compressor.h"

#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>

void fetch_2d_texel_rgb_dxt1(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel)
{
	// fetches a single texel (i,j) into pixdata (RGB)
	GLubyte *t = texel;
	const GLubyte *blksrc = (pixdata + ((srcRowStride + 3) / 4 * (j / 4) + (i / 4)) * 8);
	unsigned int c = blksrc[0] + 256*blksrc[1] + 65536*blksrc[2] + 16777216*blksrc[3];
	t[0] = (((c >> 11) & 0x1F) + ((c >> 27) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[1] = (((c >>  5) & 0x3F) + ((c >> 21) & 0x3F)) * (0.5f / 63.0f * 255.0f);
	t[2] = (((c      ) & 0x1F) + ((c >> 16) & 0x1F)) * (0.5f / 31.0f * 255.0f);
}

void fetch_2d_texel_rgba_dxt1(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel)
{
	// fetches a single texel (i,j) into pixdata (RGBA)
	GLubyte *t = texel;
	const GLubyte *blksrc = (pixdata + ((srcRowStride + 3) / 4 * (j / 4) + (i / 4)) * 8);
	unsigned int c = blksrc[0] + 256*blksrc[1] + 65536*blksrc[2] + 16777216*blksrc[3];
	t[0] = (((c >> 11) & 0x1F) + ((c >> 27) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[1] = (((c >>  5) & 0x3F) + ((c >> 21) & 0x3F)) * (0.5f / 63.0f * 255.0f);
	t[2] = (((c      ) & 0x1F) + ((c >> 16) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[3] = 255;
}

void fetch_2d_texel_rgba_dxt3(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel)
{
	// fetches a single texel (i,j) into pixdata (RGBA)
	GLubyte *t = texel;
	const GLubyte *blksrc = (pixdata + ((srcRowStride + 3) / 4 * (j / 4) + (i / 4)) * 16) + 8;
	unsigned int c = blksrc[0] + 256*blksrc[1] + 65536*blksrc[2] + 16777216*blksrc[3];
	t[0] = (((c >> 11) & 0x1F) + ((c >> 27) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[1] = (((c >>  5) & 0x3F) + ((c >> 21) & 0x3F)) * (0.5f / 63.0f * 255.0f);
	t[2] = (((c      ) & 0x1F) + ((c >> 16) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[3] = (
			  (blksrc[-8] & 0x0F)
			+ (blksrc[-8] >> 4)
			+ (blksrc[-7] & 0x0F)
			+ (blksrc[-7] >> 4)
			+ (blksrc[-6] & 0x0F)
			+ (blksrc[-6] >> 4)
			+ (blksrc[-5] & 0x0F)
			+ (blksrc[-5] >> 4)
	       ) * (0.125f / 15.0f * 255.0f);
}

void fetch_2d_texel_rgba_dxt5(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel)
{
	// fetches a single texel (i,j) into pixdata (RGBA)
	GLubyte *t = texel;
	const GLubyte *blksrc = (pixdata + ((srcRowStride + 3) / 4 * (j / 4) + (i / 4)) * 16) + 8;
	unsigned int c = blksrc[0] + 256*blksrc[1] + 65536*blksrc[2] + 16777216*blksrc[3];
	t[0] = (((c >> 11) & 0x1F) + ((c >> 27) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[1] = (((c >>  5) & 0x3F) + ((c >> 21) & 0x3F)) * (0.5f / 63.0f * 255.0f);
	t[2] = (((c      ) & 0x1F) + ((c >> 16) & 0x1F)) * (0.5f / 31.0f * 255.0f);
	t[3] = (0.5 * blksrc[-8] + 0.5 * blksrc[-7]);
}

void tx_compress_dxtn(GLint srccomps, GLint width, GLint height,
		      const GLubyte *srcPixData, GLenum destFormat,
		      GLubyte *dest, GLint dstRowStride)
{
	// compresses width*height pixels (RGB or RGBA depending on srccomps) at srcPixData (packed) to destformat (dest, dstRowStride)

	GLubyte *blkaddr = dest;
	GLint numxpixels, numypixels;
	GLint i, j;
	GLint dstRowDiff;
	unsigned char *rgba = malloc(width * height * 4);
	unsigned char *srcaddr;

	switch (destFormat) {
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			rgb565_image(rgba, srcPixData, width, height, srccomps, 0, 1);
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			rgb565_image(rgba, srcPixData, width, height, srccomps, 0, 15);
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			rgb565_image(rgba, srcPixData, width, height, srccomps, 0, 255);
			break;
		default:
			free(rgba);
			fprintf(stderr, "libdxtn: Bad dstFormat %d in tx_compress_dxtn\n", destFormat);
			return;
	}

	switch (destFormat) {
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			/* hmm we used to get called without dstRowStride... */
			dstRowDiff = dstRowStride >= (width * 2) ? dstRowStride - (((width + 3) & ~3) * 2) : 0;
			/*      fprintf(stderr, "dxt1 tex width %d tex height %d dstRowStride %d\n",
				width, height, dstRowStride); */
			for (j = 0; j < height; j += 4) {
				if (height > j + 3) numypixels = 4;
				else numypixels = height - j;
				srcaddr = rgba + j * width * 4;
				for (i = 0; i < width; i += 4) {
					if (width > i + 3) numxpixels = 4;
					else numxpixels = width - i;
					s2tc_encode_block(blkaddr, srcaddr, width, numxpixels, numypixels, DXT1, SRGB_MIXED, 0);
					srcaddr += 4 * numxpixels;
					blkaddr += 8;
				}
				blkaddr += dstRowDiff;
			}
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			dstRowDiff = dstRowStride >= (width * 4) ? dstRowStride - (((width + 3) & ~3) * 4) : 0;
			/*      fprintf(stderr, "dxt3 tex width %d tex height %d dstRowStride %d\n",
				width, height, dstRowStride); */
			for (j = 0; j < height; j += 4) {
				if (height > j + 3) numypixels = 4;
				else numypixels = height - j;
				srcaddr = rgba + j * width * 4;
				for (i = 0; i < width; i += 4) {
					if (width > i + 3) numxpixels = 4;
					else numxpixels = width - i;
					s2tc_encode_block(blkaddr, srcaddr, width, numxpixels, numypixels, DXT3, SRGB_MIXED, 0);
					srcaddr += 4 * numxpixels;
					blkaddr += 16;
				}
				blkaddr += dstRowDiff;
			}
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			dstRowDiff = dstRowStride >= (width * 4) ? dstRowStride - (((width + 3) & ~3) * 4) : 0;
			/*      fprintf(stderr, "dxt5 tex width %d tex height %d dstRowStride %d\n",
				width, height, dstRowStride); */
			for (j = 0; j < height; j += 4) {
				if (height > j + 3) numypixels = 4;
				else numypixels = height - j;
				srcaddr = rgba + j * width * 4;
				for (i = 0; i < width; i += 4) {
					if (width > i + 3) numxpixels = 4;
					else numxpixels = width - i;
					s2tc_encode_block(blkaddr, srcaddr, width, numxpixels, numypixels, DXT5, SRGB_MIXED, 0);
					srcaddr += 4 * numxpixels;
					blkaddr += 16;
				}
				blkaddr += dstRowDiff;
			}
			break;
	}

	free(rgba);
}
