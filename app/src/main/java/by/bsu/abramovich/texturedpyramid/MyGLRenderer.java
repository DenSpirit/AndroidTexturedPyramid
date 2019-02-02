package by.bsu.abramovich.texturedpyramid;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.opengl.Matrix;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class MyGLRenderer implements GLSurfaceView.Renderer, AutoCloseable {

    Context context;
    private Pyramid pyramid;
    private final float[] mMVPMatrix = new float[16];
    private final float[] mProjectionMatrix = new float[16];
    private final float[] mViewMatrix = new float[16];
    private final float[] mModelMatrix = new float[16];

    final int textureObjectsID[] = new int[8];

    private PyramidNative pyramidNative;
    private float valueX;
    private float valueY;
    private float valueZ;
    private float scale;
    private float ratio;


    public MyGLRenderer(Context context) {
        this.context = context;
        scale = 1;
    }


    public static int loadShader(int type, String shaderCode){
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);
        return shader;
    }


    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        GLES20.glClearColor(0.9f,1f,0.9f,1f);

//        pyramid = new Pyramid();
        pyramidNative = new PyramidNative();

        GLES20.glEnable(GLES20.GL_DEPTH_TEST);

        //load textures
        int textureID[] = new int[]{R.drawable.t1,R.drawable.t2,R.drawable.t3,R.drawable.t4,
                R.drawable.t5,R.drawable.t5 };


        GLES20.glGenTextures(6,textureObjectsID,0);

        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled=false;

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);

        for(int i=0;i<6;i++){
            final Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(),textureID[i],options);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textureObjectsID[i]);

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MIN_FILTER,GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MAG_FILTER,GLES20.GL_LINEAR);

            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D,0,bitmap,0);
            bitmap.recycle();
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,0); //Unbind
        }
        this.setTextures();
    }

    //frustumM(float[] m, int offset, float left, float right, float bottom, float top, float near, float far)
//multiplyMM(float[] result, int resultOffset, float[] lhs, int lhsOffset, float[] rhs, int rhsOffset)
//orthoM(float[] m, int mOffset, float left, float right, float bottom, float top, float near, float far)
//perspectiveM(float[] m, int offset, float fovy, float aspect, float zNear, float zFar)
//rotateM(float[] m, int mOffset, float a, float x, float y, float z)
//scaleM(float[] m, int mOffset, float x, float y, float z)
//setLookAtM(float[] rm, int rmOffset, float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ,
//                    float upX, float upY, float upZ)

    @Override
     public void onSurfaceChanged(GL10 gl10, int w, int h) {
        GLES20.glViewport(0,0,w,h);

        this.ratio = w > h?(float)w/h:(float)h/w;
        setTransform();
    }

    public void onAction(float valueX,float valueY,float valueZ,float scale){
        this.valueX = valueX;
        this.valueY = valueY;
        this.valueZ = valueZ;
        this.scale = scale;
        setTransform();
    }

    private void setTransform() {
        pyramidNative.setTransform(valueX, valueY, valueZ, scale, ratio);
    }

    private void setTextures() {
        pyramidNative.setTextures(this.textureObjectsID);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        pyramidNative.draw();
    }

    @Override
    public void close() throws Exception {
        pyramidNative.close();
    }
}
