//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace angle
{

class CopyTexImageTest : public ANGLETest
{
  protected:
    CopyTexImageTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    GLuint createFramebuffer(GLenum format, GLenum type, GLfloat color[4]) const
    {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLuint texture = createTexture(format, type);
        glBindTexture(GL_TEXTURE_2D, texture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glClearColor(color[0], color[1], color[2], color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        return fbo;
    }

    GLuint createTexture(GLenum format, GLenum type) const
    {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, format, 16, 16, 0, format, type, nullptr);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return tex;
    }

    GLuint createTextureFromCopyTexImage(GLuint fbo, GLenum format) const
    {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0, 16, 16, 0);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return tex;
    }

    void copyTextureWithCopyTexSubImage(GLuint fbo,
                                        GLuint texture,
                                        GLint xoffset,
                                        GLint yoffset,
                                        GLint x,
                                        GLint y,
                                        GLsizei w,
                                        GLsizei h) const
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, x, y, w, h);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        constexpr char kVS[] =
            "precision highp float;\n"
            "attribute vec4 position;\n"
            "varying vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_Position = position;\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "    texcoord.y = 1.0 - texcoord.y;\n"
            "}\n";

        constexpr char kFS[] =
            "precision highp float;\n"
            "uniform sampler2D tex;\n"
            "varying vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2D(tex, texcoord);\n"
            "}\n";

        mTextureProgram = CompileProgram(kVS, kFS);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteProgram(mTextureProgram);

        ANGLETest::TearDown();
    }

    void verifyResults(GLuint texture, GLubyte data[4], GLint x, GLint y)
    {
        glViewport(0, 0, 16, 16);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Draw a quad with the target texture
        glUseProgram(mTextureProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(mTextureUniformLocation, 0);

        drawQuad(mTextureProgram, "position", 0.5f);

        // Expect that the rendered quad has the same color as the source texture
        EXPECT_PIXEL_NEAR(x, y, data[0], data[1], data[2], data[3], 1.0);
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;
};

TEST_P(CopyTexImageTest, RGBAToL)
{
    GLfloat color[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };

    GLuint fbo = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color);
    GLuint tex = createTextureFromCopyTexImage(fbo, GL_LUMINANCE);

    GLubyte expected[] = {
        64,
        64,
        64,
        255,
    };
    verifyResults(tex, expected, 0, 0);
}

TEST_P(CopyTexImageTest, RGBToL)
{
    GLfloat color[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };

    GLuint fbo = createFramebuffer(GL_RGB, GL_UNSIGNED_BYTE, color);
    GLuint tex = createTextureFromCopyTexImage(fbo, GL_LUMINANCE);

    GLubyte expected[] = {
        64,
        64,
        64,
        255,
    };
    verifyResults(tex, expected, 0, 0);
}

TEST_P(CopyTexImageTest, RGBAToLA)
{
    GLfloat color[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };

    GLuint fbo = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color);
    GLuint tex = createTextureFromCopyTexImage(fbo, GL_LUMINANCE_ALPHA);

    GLubyte expected[] = {
        64,
        64,
        64,
        127,
    };
    verifyResults(tex, expected, 0, 0);
}

TEST_P(CopyTexImageTest, RGBAToA)
{
    GLfloat color[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };

    GLuint fbo = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color);
    GLuint tex = createTextureFromCopyTexImage(fbo, GL_ALPHA);

    GLubyte expected[] = {
        0,
        0,
        0,
        127,
    };
    verifyResults(tex, expected, 0, 0);
}

TEST_P(CopyTexImageTest, SubImageRGBAToL)
{
    GLfloat color0[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };
    GLuint fbo0 = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color0);
    GLuint tex  = createTextureFromCopyTexImage(fbo0, GL_LUMINANCE);

    GLfloat color1[] = {
        0.5f,
        0.25f,
        1.0f,
        0.75f,
    };
    GLuint fbo1 = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color1);
    copyTextureWithCopyTexSubImage(fbo1, tex, 2, 4, 5, 6, 8, 8);

    GLubyte expected0[] = {
        64,
        64,
        64,
        255,
    };
    verifyResults(tex, expected0, 0, 0);

    GLubyte expected1[] = {
        127,
        127,
        127,
        255,
    };
    verifyResults(tex, expected1, 7, 7);
}

TEST_P(CopyTexImageTest, SubImageRGBAToLA)
{
    GLfloat color0[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };
    GLuint fbo0 = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color0);
    GLuint tex  = createTextureFromCopyTexImage(fbo0, GL_LUMINANCE_ALPHA);

    GLfloat color1[] = {
        0.5f,
        0.25f,
        1.0f,
        0.75f,
    };
    GLuint fbo1 = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color1);
    copyTextureWithCopyTexSubImage(fbo1, tex, 2, 4, 5, 6, 8, 8);

    GLubyte expected0[] = {
        64,
        64,
        64,
        127,
    };
    verifyResults(tex, expected0, 0, 0);

    GLubyte expected1[] = {
        127,
        127,
        127,
        192,
    };
    verifyResults(tex, expected1, 7, 7);
}

TEST_P(CopyTexImageTest, SubImageRGBToL)
{
    GLfloat color0[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };
    GLuint fbo0 = createFramebuffer(GL_RGB, GL_UNSIGNED_BYTE, color0);
    GLuint tex  = createTextureFromCopyTexImage(fbo0, GL_LUMINANCE);

    GLfloat color1[] = {
        0.5f,
        0.25f,
        1.0f,
        0.75f,
    };
    GLuint fbo1 = createFramebuffer(GL_RGB, GL_UNSIGNED_BYTE, color1);
    copyTextureWithCopyTexSubImage(fbo1, tex, 2, 4, 5, 6, 8, 8);

    GLubyte expected0[] = {
        64,
        64,
        64,
        255,
    };
    verifyResults(tex, expected0, 0, 0);

    GLubyte expected1[] = {
        127,
        127,
        127,
        255,
    };
    verifyResults(tex, expected1, 7, 7);
}

// Read default framebuffer with glCopyTexImage2D().
TEST_P(CopyTexImageTest, DefaultFramebuffer)
{
    // Seems to be a bug in Mesa with the GLX back end: cannot read framebuffer until we draw to it.
    // glCopyTexImage2D() below will fail without this clear.
    glClear(GL_COLOR_BUFFER_BIT);

    const GLint w = getWindowWidth(), h = getWindowHeight();
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, w, h, 0);
    EXPECT_GL_NO_ERROR();
}

// Read default framebuffer with glCopyTexSubImage2D().
TEST_P(CopyTexImageTest, SubDefaultFramebuffer)
{
    // Seems to be a bug in Mesa with the GLX back end: cannot read framebuffer until we draw to it.
    // glCopyTexSubImage2D() below will fail without this clear.
    glClear(GL_COLOR_BUFFER_BIT);

    const GLint w = getWindowWidth(), h = getWindowHeight();
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    EXPECT_GL_NO_ERROR();
}

// specialization of CopyTexImageTest is added so that some tests can be explicitly run with an ES3
// context
class CopyTexImageTestES3 : public CopyTexImageTest
{};

//  The test verifies that glCopyTexSubImage2D generates a GL_INVALID_OPERATION error
//  when the read buffer is GL_NONE.
//  Reference: GLES 3.0.4, Section 3.8.5 Alternate Texture Image Specification Commands
TEST_P(CopyTexImageTestES3, ReadBufferIsNone)
{
    GLfloat color[] = {
        0.25f,
        1.0f,
        0.75f,
        0.5f,
    };

    GLuint fbo = createFramebuffer(GL_RGBA, GL_UNSIGNED_BYTE, color);
    GLuint tex = createTextureFromCopyTexImage(fbo, GL_RGBA);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glReadBuffer(GL_NONE);

    EXPECT_GL_NO_ERROR();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 4, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(CopyTexImageTest,
                       ES2_D3D9(),
                       ES2_D3D11(EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE),
                       ES2_D3D11(EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE),
                       ES2_OPENGL(),
                       ES2_OPENGL(3, 3),
                       ES2_OPENGLES(),
                       ES2_VULKAN());

ANGLE_INSTANTIATE_TEST(CopyTexImageTestES3, ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES());
}  // namespace angle
