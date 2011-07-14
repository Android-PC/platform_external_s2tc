#ifndef LIBTXC_DXTN_H
#define LIBTXC_DXTN_H

#include <GL/gl.h>

#ifdef __cplusplus
extern "C"
{
#endif

void fetch_2d_texel_rgb_dxt1(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel);
void fetch_2d_texel_rgba_dxt1(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel);
void fetch_2d_texel_rgba_dxt3(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel);
void fetch_2d_texel_rgba_dxt5(GLint srcRowStride, const GLubyte *pixdata,
			     GLint i, GLint j, GLvoid *texel);
void tx_compress_dxtn(GLint srccomps, GLint width, GLint height,
		      const GLubyte *srcPixData, GLenum destFormat,
		      GLubyte *dest, GLint dstRowStride);

#ifdef __cplusplus
}
#endif

#endif
