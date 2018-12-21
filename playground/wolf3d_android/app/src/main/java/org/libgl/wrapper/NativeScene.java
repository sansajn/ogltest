package org.libgl.wrapper;

import android.view.MotionEvent;

public class NativeScene {

	// volaj pri vytvoreni egl kontextu (napr. GLSurfaceView.Renderer.onSurfaceCreated())
	public static native void create(int width, int  height);

	/* volaj pri uvolneni egl kontextu (napr. GLSurfaceView.surfaceDestroyed())
	\note GLSurfaceView.Renderer bezi vo vlastnom vlakne, preto je pri volani z GLSurfaceView
	nutne pouzit Runnable a funkciu queueEvent(). V dobe vykonania kodu s runnable uz bude
	pravdepodobne egl kontext uvolneny systemom, ale aspon v kniznici moze prebehnut destruktor na
	ostatne alokovane zdroje. */
	public static native void destroy();

	// volaj pri zmene rozmerou surface-u (napr. GLSurfaceView.Renderer.onSurfaceChanged())
	public static native void reshape(int width, int  height);

	// volaj pri potrebe vykreslit scenu (napr. GLSurfaceView.Renderer.onDrawFrame())
	public static native void display();

	// volaj pri dotyku displey-a (napr. GLSurfaceView.onTouchEvent())
	public static void touch(MotionEvent event) {
		if (event.getActionMasked() == MotionEvent.ACTION_MOVE) {
			for (int idx = 0; idx < event.getPointerCount(); ++idx)
				touch((int)event.getX(idx), (int)event.getY(idx), event.getPointerId(idx), ACTION_MOVE);
			return;  // we are done
		}

		int pointerIdx, nativeAction;

		switch (event.getActionMasked()) {
			case MotionEvent.ACTION_DOWN:
				pointerIdx = 0;
				nativeAction = ACTION_DOWN;
				break;
			case MotionEvent.ACTION_UP:
				pointerIdx = 0;
				nativeAction = ACTION_UP;
				break;
			case MotionEvent.ACTION_POINTER_DOWN:
				pointerIdx = (event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				nativeAction = ACTION_DOWN;
				break;
			case MotionEvent.ACTION_POINTER_UP:
				pointerIdx = (event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				nativeAction = ACTION_UP;
				break;
			case MotionEvent.ACTION_CANCEL:
				pointerIdx = (event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				nativeAction = ACTION_CANCEL;
				break;
			default: return;  // ignore other actions
		}

		touch((int)event.getX(pointerIdx), (int)event.getY(pointerIdx), event.getPointerId(pointerIdx), nativeAction);
	}

	private static native void touch(int x, int y, int pointerId, int action);

	private static int ACTION_DOWN = 0;
	private static int ACTION_UP = 1;
	private static int ACTION_MOVE = 2;
	private static int ACTION_CANCEL = 3;

	static {
		System.loadLibrary("w3dclone");  // loads libw3dclone.so
	}
}
