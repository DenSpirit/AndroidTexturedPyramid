//
// Created by kurai on 01.02.19.
//

#include <jni.h>
#include "native-lib.h"
#include <GLES2/gl2.h>
#include <cstring>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <android/log.h>

const float pyramidCoords[] = {
        0.0,-0.8,0.0,0.5,1,
        0.8,0.8,0.8,0.95,0,
        -0.8,0.8,0.8,0.05,0,

        0.0,-0.8,0.0,0.5,1,
        -0.8,0.8,0.8,0.95,0,
        -0.8,0.8,-0.8,0.05,0,

        0.0,-0.8,0.0,0.5,1,
        -0.8,0.8,-0.8,0.95,0,
        0.8,0.8,-0.8,0.05,0,

        0.0,-0.8,0.0,0.5,1,
        0.8,0.8,-0.8,0.95,0,
        0.8,0.8,0.8,0.05,0,

        0.8,0.8,0.8,1,1,
        -0.8,0.8,-0.8,0,0,
        -0.8,0.8,0.8,0.0,1,

        0.8,0.8,0.8,1,1,
        0.8,0.8,-0.8,1,0,
        -0.8,0.8,-0.8,0,0
};
const float vectorToLight[] ={2, 0, 0.5};
const float lengthTL = sqrt(vectorToLight[0]*vectorToLight[0]+vectorToLight[1]*vectorToLight[1]+vectorToLight[2]*vectorToLight[2]);

const char vertexShaderCode[] =
    "attribute vec4 aPosition;"
    "attribute vec2 aTexCoord;"
    "uniform mat4 uMVPMatrix;"
    "varying vec2 vTexCoord;"
    "void main() {"
    "  vTexCoord = aTexCoord;"
    "  gl_Position = uMVPMatrix*aPosition;"
    "}";

const char fragmentShaderCode[] =
   "precision mediump float;"
   "uniform float uCos;"
   "uniform sampler2D uTextureUnit;"
   "varying vec2 vTexCoord;"
   "void main() {"
   "  gl_FragColor = texture2D(uTextureUnit,vTexCoord)*uCos;"
   "}";

void check() {
    auto err = glGetError();
    if (err != GL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, "tag", "error = %d", err);
        throw err;
    }
}

GLuint loadShader(GLuint type, const char code[]) {
    auto shader = glCreateShader(type);
    GLint len = strlen(code);
    GLint lengths[1] { len };
    glShaderSource(shader, 1, &code, lengths);
    glCompileShader(shader);
    check();
    return shader;
}

class Pyramid {
    const int POSITION_COUNT = 3; //number of position coordinates
    const int TEXTURE_COORD_COUNT = 2;
    const int COORDINATES_PER_VERTEX = POSITION_COUNT+TEXTURE_COORD_COUNT;
    const int TRIANGLES_COUNT = 6;

    glm::mat4x4 mMVPMatrix;
    int textureObjectsID[8];

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;

public:
    Pyramid() {
        this->program = glCreateProgram();
        this->vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderCode);
        this->fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        glAttachShader(this->program, this->vertexShader);
        glAttachShader(this->program, this->fragmentShader);
        glLinkProgram(this->program);
    }
    void setTextures(int textureObjectsId[8]) {
        for (int i = 0; i < 8; i++) {
            this->textureObjectsID[i] = textureObjectsId[i];
        }
    }

    void setProjections(float valueX, float valueY, float valueZ, float scale, float ratio) {
        auto model = glm::mat4(1.0f);
        auto view = glm::lookAt(glm::vec3(0.f, 1.5f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
        auto projection = glm::perspective(glm::radians(45.f), ratio, 0.1f, 10.f);
        model = glm::rotate(model, glm::radians(valueX), glm::vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(valueY), glm::vec3(0.f, 1.f, 0.f));
        model = glm::rotate(model, glm::radians(valueZ), glm::vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, glm::vec3(scale / 5.f));
        auto mvp = view;
        mvp *= model;
        mvp *= projection;
        this->mMVPMatrix = mvp;
    }

    void draw() {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        auto mProgram = this->program;
        auto isProgram = glIsProgram(mProgram);
        glUseProgram(mProgram);

        auto mPositionHandle = glGetAttribLocation(mProgram, "aPosition");
        auto mTextCoordHandle = glGetAttribLocation(mProgram, "aTexCoord");

        auto mTextureUnit = glGetUniformLocation(mProgram, "uTextureUnit");
        auto mCosHandle = glGetUniformLocation(mProgram, "uCos");

        auto mMVPMatrixHandle = glGetUniformLocation(mProgram, "uMVPMatrix");

        glUniformMatrix4fv(mMVPMatrixHandle, 1, false, glm::value_ptr(mMVPMatrix));

        glVertexAttribPointer(mPositionHandle, POSITION_COUNT,
                                     GL_FLOAT, false, 4 * COORDINATES_PER_VERTEX, pyramidCoords);
        glEnableVertexAttribArray(mPositionHandle);

        glVertexAttribPointer(mTextCoordHandle,TEXTURE_COORD_COUNT,
                                     GL_FLOAT, false, 4 * COORDINATES_PER_VERTEX, pyramidCoords + POSITION_COUNT);
        glEnableVertexAttribArray(mTextCoordHandle);

        glActiveTexture(GL_TEXTURE0);

        for(int j=0;j<TRIANGLES_COUNT;j++){

            //Берем три точки A(x1,y1,z1), B(x2,y2,z2),C(x3,y3,z3)
            //Считаем вектор нормали N(nx,ny,nz)
            int i = j*COORDINATES_PER_VERTEX*3;

            float x1 = pyramidCoords[i];
            float y1 = pyramidCoords[i+1];
            float z1 = pyramidCoords[i+2];

            float x2 = pyramidCoords[i+COORDINATES_PER_VERTEX];
            float y2 = pyramidCoords[i+1+COORDINATES_PER_VERTEX];
            float z2 = pyramidCoords[i+2+COORDINATES_PER_VERTEX];

            float x3 = pyramidCoords[i+2*COORDINATES_PER_VERTEX];
            float y3 = pyramidCoords[i+1+2*COORDINATES_PER_VERTEX];
            float z3 = pyramidCoords[i+2+2*COORDINATES_PER_VERTEX];

            float nx =y1*(z2 - z3) + y2*(z3 - z1) + y3*(z1 - z2);
            float ny =z1*(x2 - x3) + z2*(x3 - x1) + z3*(x1 - x2);
            float nz =x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2);

            float lengthN = (float) sqrt(nx*nx+ny*ny+nz*nz);

            auto norm = glm::vec4(nx, ny, nz, 1.f);
            norm /= lengthN;

            norm = mMVPMatrix * norm;
            nx = norm[0];
            ny = norm[1];
            nz = norm[2];

            //Рассчитываем косинус угла между нормалью и вектором источника света
            float cosValue = (nx*vectorToLight[0]+ny*vectorToLight[1]+nz*vectorToLight[2])/lengthTL;
//            cosValue /= 2;

//            glUniform1f(mCosHandle, cosValue);
             glUniform1f(mCosHandle,1.f);

            glBindTexture(GL_TEXTURE_2D, this->textureObjectsID[j]);
            glUniform1i(mTextureUnit, 0);
            glDrawArrays(GL_TRIANGLES, j*3, 3); //3 vertices for triangle

        }
        glBindTexture(GL_TEXTURE_2D,0);

        glDisableVertexAttribArray(mPositionHandle);
        glDisableVertexAttribArray(mTextCoordHandle);
        check();
    }
};

extern "C" {
    JNIEXPORT jlong JNICALL Java_by_bsu_abramovich_texturedpyramid_PyramidNative_getNewPyramidObj(JNIEnv * env, jclass clazz) {
        return (long) (new Pyramid());
    }

    JNIEXPORT void JNICALL Java_by_bsu_abramovich_texturedpyramid_PyramidNative_releasePyramidObj(JNIEnv * env, jclass clazz, jlong pyramidCls) {
        delete (Pyramid*) pyramidCls;
    }

    JNIEXPORT void JNICALL Java_by_bsu_abramovich_texturedpyramid_PyramidNative_setPyramidTransform(JNIEnv * env, jclass clazz, jlong pyramidCls,
            jfloat floatX, jfloat floatY, jfloat floatZ, jfloat scale, jfloat ratio) {
        Pyramid* pyramid = (Pyramid*) pyramidCls;
        pyramid->setProjections(floatX, floatY, floatZ, scale, ratio);
    }

    JNIEXPORT void JNICALL Java_by_bsu_abramovich_texturedpyramid_PyramidNative_draw(JNIEnv * env, jclass clazz, jlong pyramidCls) {
        Pyramid* pyramid = (Pyramid*) pyramidCls;
        pyramid->draw();
    }

    JNIEXPORT void JNICALL
    Java_by_bsu_abramovich_texturedpyramid_PyramidNative_setPyramidTextures(JNIEnv *env, jclass type,
                                                                            jlong obj,
                                                                            jintArray textureIds_) {
        jint *textureIds = env->GetIntArrayElements(textureIds_, NULL);

        Pyramid* pyramid = (Pyramid*) obj;
        pyramid->setTextures(textureIds);

        env->ReleaseIntArrayElements(textureIds_, textureIds, 0);
    }
}
