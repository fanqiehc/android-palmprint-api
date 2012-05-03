package teaonly.projects.palmapi;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log; 
import android.util.Xml;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebView;

public class AnalyserActivity extends Activity {
    final private String TAG = "TEAONLY";
    private WebView webview;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);        
	    setContentView(R.layout.analyser);

	    //setup webView
	    webview = (WebView)findViewById(R.id.webview);
	    webview.getSettings().setJavaScriptEnabled(true);
	    webview.setVerticalScrollBarEnabled(false);
	    webview.addJavascriptInterface(this, "Helper");
	    webview.loadUrl("file:///android_asset/app.html");
    }
	
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause(){    	
        super.onPause();
    }

    @Override
    public void onResume(){    	
        super.onResume();
    }

    @Override
    public void onBackPressed() {
	    //Hook and do nothing
    }

    public void DebugPrint(String msg) {
	    Log.d(TAG, msg);
    }
}
