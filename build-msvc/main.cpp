//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on MultiTexture.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"

#include "shader_utils.h"
#include "system_utils.h"
#include "tga_utils.h"

extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}

class MultiTextureSample : public SampleApplication
{
public:
    MultiTextureSample(const char *displayTypeArg)
        : SampleApplication("MultiTexture", 1280, 720, 2, 0, GetDisplayTypeFromArg(displayTypeArg))
    {
    }

    GLuint loadTexture(const std::string &path, TGAImage &img)
    {
        if (!LoadTGAImageFromFile(path, &img))
        {
            return 0;
        }

        return LoadTextureFromTGAImage(img);
    }

    virtual bool initialize()
    {
        const std::string vs =
            R"(attribute vec4 a_position;
            attribute vec2 a_texCoord;
            uniform vec2 u_windowSize;
            varying vec2 v_texCoord;
            void main()
            {
                vec2 pos = a_position.xy / u_windowSize * 2.0 - 1.0;
                gl_Position = vec4(pos.x, -pos.y, 0, 1);
                v_texCoord = a_texCoord;
            })";

        const std::string fs =
            R"(precision mediump float;
            varying vec2 v_texCoord;
            uniform sampler2D s_baseMap;
            uniform sampler2D s_lightMap;
            void main()
            {
                vec4 baseColor;
                vec4 lightColor;

                baseColor = texture2D(s_baseMap, v_texCoord);
                lightColor = texture2D(s_lightMap, v_texCoord);
                gl_FragColor = baseColor;// * (lightColor + 0.5);
            })";

        mProgram = CompileProgram(vs, fs);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        mPositionLoc = glGetAttribLocation(mProgram, "a_position");
        mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

        mWindowSizeLoc = glGetUniformLocation(mProgram, "u_windowSize");

        // Get the sampler location
        mBaseMapLoc = glGetUniformLocation(mProgram, "s_baseMap");
        mLightMapLoc = glGetUniformLocation(mProgram, "s_lightMap");

        // Load the textures
        std::stringstream baseStr;
        baseStr << "basemap.tga";

        std::stringstream lightStr;
        lightStr << "lightmap.tga";

        mBaseMapTexID = loadTexture(baseStr.str(), mImgBase);
        TGAImage imgLight;
        mLightMapTexID = loadTexture(lightStr.str(), imgLight);
        if (mBaseMapTexID == 0 || mLightMapTexID == 0)
        {
            return false;
        }

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mBaseMapTexID);
        glDeleteTextures(1, &mLightMapTexID);
    }

    virtual void draw()
    {
        static GLfloat vertices[] =
        {
            0, 0, 0,                            0, 1,
            mImgBase.width, 0, 0,               1, 1,
            mImgBase.width, mImgBase.height, 0, 1, 0,
            0, mImgBase.height, 0,              0, 0,
        };
        static GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        // Set the viewport
        int wndWidth = getWindow()->getWidth();
        int wndHeight = getWindow()->getHeight();
        glViewport(0, 0, wndWidth, wndHeight);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        glUniform2f(mWindowSizeLoc, wndWidth, wndHeight);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices + 3);

        glEnableVertexAttribArray(mPositionLoc);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Bind the base map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mBaseMapTexID);

        // Set the base map sampler to texture unit to 0
        glUniform1i(mBaseMapLoc, 0);

        // Bind the light map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mLightMapTexID);

        // Set the light map sampler to texture unit 1
        glUniform1i(mLightMapLoc, 1);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    GLint mWindowSizeLoc;

    // Sampler locations
    GLint mBaseMapLoc;
    GLint mLightMapLoc;

    // Texture handle
    GLuint mBaseMapTexID;
    GLuint mLightMapTexID;

    TGAImage mImgBase;
};

int main(int argc, char **argv)
{
    const char *displayTypeArg = "";
    if (argc == 2)
        displayTypeArg = argv[1];
    MultiTextureSample app(displayTypeArg);
    return app.run();
}
