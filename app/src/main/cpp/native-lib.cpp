#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#define TAG "csz"
#define LOG(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define argb(a,r,g,b) (((a) & 0xff)<<24 | ((b) & 0xff)<<16 | ((g) & 0xff)<<8 |r)

typedef struct GifBean{
    int current_frame;
    int total_frame;
    //每帧的延迟时间数组
    int *delays;
}GifBean;

/**
 * 绘制一帧
 * @param pType
 * @param pBean
 * @param info
 * @param pixels
 */
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    SavedImage savedImage =gifFileType->SavedImages[gifBean->current_frame];
    int* px = static_cast<int *>(pixels);
    int pointPixel;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    GifByteType gifByteType;//每个像素的数据
    ColorMapObject * colorMapObject = frameInfo.ColorMap;
    if (colorMapObject == NULL){
        colorMapObject=gifFileType->SColorMap;
    }
    px = reinterpret_cast<int *>((char *) px + info.stride * frameInfo.Top);
    int *line;
    for(int y=frameInfo.Top;y<frameInfo.Top+frameInfo.Height;y++){
        line = px;
        for(int x=frameInfo.Left;x<frameInfo.Left+frameInfo.Width;x++){
            pointPixel =(y-frameInfo.Top)*frameInfo.Width+(x-frameInfo.Left);
            gifByteType = savedImage.RasterBits[pointPixel];
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255,gifColorType.Red,gifColorType.Green,gifColorType.Blue);
        }
        px = reinterpret_cast<int *>((char *) px + info.stride);
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_csz_gif_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}



extern "C"
JNIEXPORT jlong JNICALL
Java_com_csz_gif_GifLoader_load(JNIEnv *env, jobject thiz, jstring path) {
    const char* pathIndex = env->GetStringUTFChars(path,0);
    int err;
    GifFileType *gifFileType = DGifOpenFileName(pathIndex,&err);
    DGifSlurp(gifFileType);
    GifBean *gifBean = static_cast<GifBean *>(malloc(sizeof(GifBean)));
    memset(gifBean,0, sizeof(GifBean));
    gifBean->delays= static_cast<int *>(malloc(sizeof(int) * gifFileType->ImageCount));
    memset(gifBean->delays,0,sizeof(int) * gifFileType->ImageCount);

    gifFileType->UserData = gifBean;
    gifBean->current_frame = 0;
    gifBean->total_frame = gifFileType->ImageCount;

    ExtensionBlock* ext;
    for (int i = 0; i < gifBean->total_frame; ++i) {
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j<frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE){
                ext = &frame.ExtensionBlocks[j];
                int frame_delay = 10 * (ext->Bytes[1]|(ext->Bytes[2]<<8));
                LOG("frame_delay : %d",frame_delay);
                gifBean->delays[i]=frame_delay;
                break;
            }
        }
    }
    return (jlong)gifFileType;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_csz_gif_GifLoader_getWidth(JNIEnv *env, jobject thiz, jlong gif) {
    GifFileType * gifFileType = reinterpret_cast<GifFileType *>(gif);
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_csz_gif_GifLoader_getHeight(JNIEnv *env, jobject thiz, jlong gif) {
    GifFileType * gifFileType = reinterpret_cast<GifFileType *>(gif);
    return gifFileType->SHeight;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_csz_gif_GifLoader_updateFrame(JNIEnv *env, jobject thiz, jlong gif, jobject bitmap) {
    GifFileType *gifFileType= reinterpret_cast<GifFileType *>(gif);
    GifBean* gifBean= static_cast<GifBean *>(gifFileType->UserData);

    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env,bitmap,&info);
    AndroidBitmap_lockPixels(env,bitmap,&pixels);
    drawFrame(gifFileType,gifBean,info,pixels);

    int delay = gifBean->delays[gifBean->current_frame];
    gifBean->current_frame += 1;
    if(gifBean->current_frame == gifBean->total_frame){
        gifBean->current_frame = 0;
    }
    AndroidBitmap_unlockPixels(env,bitmap);
    return delay;
}