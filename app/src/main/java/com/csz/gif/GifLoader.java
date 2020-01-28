package com.csz.gif;

import android.annotation.SuppressLint;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Message;
import android.widget.ImageView;

import java.lang.ref.WeakReference;
import java.util.HashSet;
import java.util.Iterator;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class GifLoader {

    static HashSet<GifLoader> sLoaderSet = new HashSet<>();

    private static final int UPDATE = 0x01;

    private long mAddress;
    private String mPath;

    private Bitmap mBitmap;
    private WeakReference<ImageView> mImageViewWeakReference;

    @SuppressLint("HandlerLeak")
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            if (msg.what == UPDATE) {
                ImageView imageView = mImageViewWeakReference.get();
                if (imageView != null) {
                    int update = update(mBitmap);
                    imageView.setImageBitmap(mBitmap);
                    sendEmptyMessageDelayed(UPDATE, update);
                }
            }
        }
    };

    private GifLoader() {
    }

    public static GifLoader create(String path) {
        GifLoader loader = new GifLoader(path);
        if (sLoaderSet.contains(loader)) {
            Iterator<GifLoader> iterator = sLoaderSet.iterator();
            while (iterator.hasNext()) {
                GifLoader next = iterator.next();
                if (next.equals(loader)) {
                    loader = null;
                    return next;
                }
            }
        }
        sLoaderSet.add(loader);
        return loader;
    }

    private GifLoader(String path) {
        if (mPath == path) return;
        mPath = path;

    }

    public void into(ImageView imageView) {
        if (mImageViewWeakReference == null) {
            mImageViewWeakReference = new WeakReference<>(imageView);
        } else if (imageView == mImageViewWeakReference.get()) return;

        mAddress = load(mPath);
        int width = getWidth();
        int height = getHeight();
        mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        int interval = update(mBitmap);
        imageView.setImageBitmap(mBitmap);
        mHandler.sendEmptyMessageDelayed(UPDATE, interval);
    }

    public int getWidth() {
        return getWidth(mAddress);
    }

    public int getHeight() {
        return getHeight(mAddress);
    }

    public int update(Bitmap bitmap) {
        return updateFrame(mAddress, bitmap);
    }

    public void release(){
        mImageViewWeakReference.clear();
        sLoaderSet.remove(this);
    }

    @Override
    public boolean equals(@Nullable Object obj) {
        boolean assignableFrom = GifLoader.class.isAssignableFrom(obj.getClass());
        if (assignableFrom){
            GifLoader loader = (GifLoader) obj;
            return this.mPath.equals(loader.mPath);
        }
        return assignableFrom;
    }

    @Override
    public int hashCode() {
        return mPath.hashCode();
    }

    private native long load(String path);

    private native int getWidth(long gif);

    private native int getHeight(long gif);

    private native int updateFrame(long gif, Bitmap bitmap);

}
