package org.example;

import android.app.Activity;
import android.os.Bundle;
import org.libgl.view.GLSurfaceViewImpl;

public class MainActivity extends Activity {

	@Override protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		_view = new GLSurfaceViewImpl(getApplication());
		setContentView(_view);
	}

	@Override protected void onPause() {
		super.onPause();
		_view.onPause();
	}

	@Override protected void onResume() {
		super.onResume();
		_view.onResume();
	}

	GLSurfaceViewImpl _view;
}
