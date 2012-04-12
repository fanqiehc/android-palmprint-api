package teaonly.projects.palmapi;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class OverlayView extends View{
    private int x = 0;

    public OverlayView(Context c, AttributeSet attr) {
        super(c, attr);        
    }

    public void DrawResult() {
    }

    @Override
    protected void onDraw(Canvas canvas) {
    }

}
