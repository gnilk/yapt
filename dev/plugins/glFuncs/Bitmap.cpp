#include <vector>
#include <fstream>

#include "picopng.h"
#include "lodepng.h"

#include "Bitmap.h"


//
// -- bitmap data for font
//
Bitmap::Bitmap(int w, int h, unsigned char *buffer) {
	this->width = w;
	this->height = h;
	this->buffer = (unsigned char *)malloc(sizeof(char) * w * h *4);
	for(int i=0;i<w*h;i++) {
		this->buffer[i*4+0] = buffer[i];
		this->buffer[i*4+1] = buffer[i];
		this->buffer[i*4+2] = buffer[i];
		this->buffer[i*4+3] = buffer[i];
	}
}
Bitmap::Bitmap(int w, int h) {
	this->width = w;
	this->height = h;
	this->buffer = (unsigned char *)malloc(sizeof(char) * w * h *4);

}

Bitmap::~Bitmap() {
	free(this->buffer);
}

Bitmap *Bitmap::FromAlpha(int w, int h, unsigned char *srcData) {
	Bitmap *dst = new Bitmap(w,h);
	for(int i=0;i<w*h;i++) {
		dst->buffer[i*4+0] = srcData[i];
		dst->buffer[i*4+1] = srcData[i];
		dst->buffer[i*4+2] = srcData[i];
		dst->buffer[i*4+3] = srcData[i];
	}
	return dst;
}

Bitmap *Bitmap::FromRGBA(int w, int h, unsigned char *srcData) {
	Bitmap *dst = new Bitmap(w,h);
	memcpy(dst->buffer, srcData, w*h*4);
	return dst;
}


unsigned char *Bitmap::Buffer(int x, int y) {
	return &buffer[(x+y*width)*4];
}

Bitmap *Bitmap::CopyToNew(int x, int y, int w, int h) {
	Bitmap *newBmp = new Bitmap(w,h);

	// Copy scanlines
	for(int row=0; row<h; row++) {
		memcpy(newBmp->Buffer(0, row), Buffer(x, y+row),w*4);
	}

	return newBmp;
}

static void LoadFile(std::vector<unsigned char>& buffer, const std::string& filename) //designed for loading files from hard disk in an std::vector
{
  std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

  //get filesize
  std::streamsize size = 0;
  if(file.seekg(0, std::ios::end).good()) size = file.tellg();
  if(file.seekg(0, std::ios::beg).good()) size -= file.tellg();


  //read contents of the file into the vector
  if(size > 0)
  {
    buffer.resize((size_t)size);
    file.read((char*)(&buffer[0]), size);
	//psys->GetLogger("BitmapFont")->Debug("Read %d bytes from image file '%s'", size, filename.c_str());
  } else {
	//psys->GetLogger("BitmapFont")->Debug("Invalid file size: %d", size);
  	buffer.clear();
  }
}


Bitmap *Bitmap::LoadPNGImage(std::string imagefile) {
	std::vector<unsigned char> buffer, image;
	::LoadFile(buffer, imagefile);
	unsigned long w, h;
	int error = DecodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());
	if (error != 0) {
		//psys->GetLogger("BitmapFont")->Error("Decode PNG file '%s' failed, error=%d", imagefile.c_str(), error);
		return NULL;
	}

	//psys->GetLogger("BitmapFont")->Debug("Decoded PNG ok, w=%d, h=%d",w,h);
	// allocate and create bitmap of original font
	//bitmap = new Bitmap(w,h, &image[0]);
	Bitmap *bitmap = Bitmap::FromRGBA(w,h,&image[0]);

	return bitmap;
}
