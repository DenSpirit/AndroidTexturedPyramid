package by.bsu.abramovich.texturedpyramid;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class MyGLSurfaceView extends GLSurfaceView {
    private final MyGLRenderer mRenderer;

    public MyGLSurfaceView(Context context) {
        super(context);
        setEGLContextClientVersion(2);
        mRenderer = new MyGLRenderer(context);
        setRenderer(mRenderer);
    }

    public MyGLRenderer getRenderer(){
        return mRenderer;
    }
}
