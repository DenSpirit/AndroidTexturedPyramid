package by.bsu.abramovich.texturedpyramid;

public class PyramidNative implements AutoCloseable {

    static {
        System.loadLibrary("native-lib");
    };

    private static native long getNewPyramidObj();
    private static native void releasePyramidObj(long obj);

    private static native void setPyramidTransform(long obj, float valueX,float valueY,float valueZ,float scale, float ratio);
    private static native void draw(long obj);
    private static native void setPyramidTextures(long obj, int[] textureIds);

    private long handler;

    PyramidNative() {
        this.handler = getNewPyramidObj();
    }

    public void close() {
        long handler = this.handler;
        this.handler = 0;
        releasePyramidObj(handler);
    }

    public void setTransform(float valueX, float valueY, float valueZ, float scale, float ratio) {
        setPyramidTransform(this.handler, valueX, valueY, valueZ, scale, ratio);
    }

    public void draw() {
        draw(this.handler);
    }

    public void setTextures(int[] textureObjectsID) {
        setPyramidTextures(this.handler, textureObjectsID);
    }
}
