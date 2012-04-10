package org.openpk.palmapi;

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

public class VisionOverlay extends View{
    private int x = 0;

    public VisionOverlay(Context c, AttributeSet attr) {
        super(c, attr);        
    }

    public void DrawResult(int r) {
        x = r;
        postInvalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        Paint p = new Paint();
        p.setTextSize (50);
        canvas.drawText("Hello", 100, x+50, p);
    }

}
