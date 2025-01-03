// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "tesseract/baseapi.h"
#include "leptonica/allheaders.h"

tesseract::TessBaseAPI* api;

struct Pix
{
    l_uint32 w; //width in pixels
    l_uint32 h; //height in pixels
    l_uint32 d; //depth in bits
    l_uint32 spp; //samples per pixel
    l_uint32 wpl; //32-bit words/line
    l_uint32 refcount; //reference count (1 if no clones)
    l_int32 xres; //image res (ppi) in x direction (use 0 if unknown)
    l_int32 yres; //image res (ppi) in y direction (use 0 if unknown)
    l_int32 informat; //input file format, IFF_* - 0 for unknown (if not from file)
    l_int32 special; // special instructions for I/O, etc
    char* text; //text string associated with pix
    struct PixColormap* colormap; //colormap (may be null)
    l_uint32* data; //the image data
};

Pix MyPix; //used to pass image to ocr

//we do not use any initialization in dllmain because current Tesseract is mostly thread safe. So we can use it in dynamic classes (multiple instances).
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void tesseractApiCreate(); //we create api for every object
void tesseractApiCreate() {
    api = new tesseract::TessBaseAPI();
}

extern "C" __declspec(dllexport) void tesseractApiDispose(); //we dispose api when object is disposed
void tesseractApiDispose() {
    api->End();
    delete api;
}

extern "C" __declspec(dllexport) bool tesseractApiInit(char* DataPath, char* language); //we can init and reinit same object
bool tesseractApiInit(char* dataPath, char* language) {

    if (api->Init(dataPath, language) == 0) return true; else return false;
}

//use data to initialize image
extern "C" __declspec(dllexport) void tesseractApiSetImageData(unsigned int* Data, int width, int height, int bits, int verticalResolution, int horizontalResolution);
void tesseractApiSetImageData(unsigned int* Data, int width, int height, int bits, int verticalResolution, int horizontalResolution){
    //verticalResolution & horizontalResolution are optional
    MyPix.data = Data;
    MyPix.w = width;
    MyPix.h = height;
    MyPix.d = bits;
    MyPix.spp = bits/8; //sample per pixel (4 bytes for 32 bit image)
    MyPix.wpl = width; //same as width?
    MyPix.refcount = 1; //always 1 when no extra handles created (we don not create them)
    MyPix.xres = horizontalResolution;
    MyPix.yres = verticalResolution;
    api->SetImage(&MyPix);
}

//use Pix to initialize image
extern "C" __declspec(dllexport) void tesseractApiSetImagePix(Pix* image);
void tesseractApiSetImagePix(Pix* image) {
    api->SetImage(image);
}

//get UTF8 result
extern "C" __declspec(dllexport) char* tesseractApiGetUTF8();
char* tesseractApiGetUTF8() {
    return api->GetUTF8Text();
}

//get HOCR result (html based)
extern "C" __declspec(dllexport) char* tesseractApiGetHOCR(int page);
char* tesseractApiGetHOCR(int page) {
    return api->GetHOCRText(page);
}

//read Pix from file
extern "C" __declspec(dllexport) Pix* tesseractApiPixRead(char* filePath);
Pix* tesseractApiPixRead(char* filePath) {
    return pixRead(filePath);
}