//   Copyright © 2021, Renjie Chen @ USTC

#pragma once

#include <cassert>
#include <vector>
#include <string>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)



class MyImage
{
private:
	std::vector<BYTE> pixels;
    int w, h, comp;

public:
    MyImage():w(0),h(0),comp(0) {}
    ~MyImage() { }

	MyImage(const std::string &filename, int ncomp=4):w(0), h(0),comp(0)
	{
		stbi_set_flip_vertically_on_load(true);
		BYTE *data = stbi_load(filename.data(), &w, &h, &comp, ncomp);

		if (data) {
			pixels = std::vector<BYTE>(data, data + w*h*comp);
			stbi_image_free(data);
		}
		else {
			fprintf(stderr, "failed to load image file %s!\n", filename.c_str());
            struct stat info;
            if ( stat(filename.c_str(), &info))
                fprintf(stderr, "file doesn't exist!\n");
		}
    }


    MyImage(BYTE* data, int ww, int hh, int pitch, int ncomp = 3) :w(ww), h(hh), comp(ncomp)
    {
		if (pitch == w*comp) pixels = std::vector<BYTE>(data, data + pitch*h);
		else {
			pixels.resize(w*comp*h);
			for (int i = 0; i < h; i++) std::copy_n(data + pitch*i, pitch, pixels.data() + i*w*comp);
		}
	}
	MyImage(int ww, int hh, std::vector<BYTE>&ans)
	{
		w = ww;
		h = hh;
		comp = 3;
		pixels = std::vector<BYTE>(ans);
	}

    static int alignment() { return 1; }  // OpenGL only supports 1,2,4,8, do not use 8, it is buggy

    inline bool empty() const { return pixels.empty(); }

	inline BYTE* data() { return pixels.data(); }
	inline const BYTE* data() const { return pixels.data(); }
	inline int width() const { return w; }
	inline int height() const { return h; }
	inline int dim() const { return comp; }
	inline int pitch() const { return w*comp; }

    MyImage rescale(int ww, int hh) const
    {
		std::vector<BYTE> data(ww*comp*hh);
		stbir_resize_uint8(pixels.data(), w, h, w*comp,
			data.data(), ww, hh, ww*comp, comp);
 
		return MyImage(data.data(), ww, hh, ww*comp, comp);   
	}


    MyImage resizeCanvas(int ww, int hh)
    {
		std::vector<BYTE> data(ww*comp*hh, 255);
		for (int i = 0; i < h; i++)
			std::copy_n(pixels.data() + i*w*comp, w*comp, data.data() + i*ww*comp);
 
		return MyImage(data.data(), ww, hh, ww*comp, comp);   
	}

	std::vector<std::vector<double>> to_energy() {
		std::vector<std::vector<double>>ans;
		for (int i = 0; i < h; i++)ans.push_back(std::vector<double>(w));
		for (int i = 0; i < h; i++)for (int j = 0; j < w; j++)for (int k = 0; k < comp; k++)ans[i][j] += pixels[comp * (i * w + j) + k];
		return ans;
	}
	void decrease_width(std::vector<int>& path, bool real=true) { // false for show line
		for (int i = size(path) - 1; i >= 0; i--) {
			int position = comp * (i * w + path[i]);
			if (!real)for (int j = comp - 1; j >= 0; j--) pixels[position + j] = 255;
			else {
				for (int j = comp - 1; j >= 0; j--) pixels.erase(pixels.begin() + position + j);
				if (i % h == 0)w -= 1;
			}
		}
	}
	void increase_width(std::vector<int>& path, bool real=true) { // false for show line
		for (int i = size(path) - 1; i >= 0; i--) {
			int position = comp * ((i % h) * w + path[i]);
			if (!real)for (int j = comp - 1; j >= 0; j--) pixels[position + j] = 255;
			else {
				for (int j = comp - 1; j >= 0; j--) pixels.insert(pixels.begin() + position, *(pixels.begin() + position + comp - 1));
				if (i % h == 0)w += 1;
			}
		}
	}
	void transpose() {
		std::vector<BYTE>ori_pixels(pixels);
		for (int i = 0; i < w; i++)for (int j = 0; j < h; j++)for (int k = 0; k < comp; k++)pixels[comp * (i * h + j) + k] = ori_pixels[comp * (j * w + i) + k];
		int i = w;
		w = h;
		h = i;
	}



    inline void write(const std::string &filename, bool vflip=true) const {
		if (filename.size() < 4 || !_strcmpi(filename.data() + filename.size() - 4, ".png")) {
			stbi_write_png(filename.data(), w, h, comp, pixels.data()+(vflip?w*comp*(h-1):0), w*comp*(vflip?-1:1));
		}
		else {
			fprintf(stderr, "only png file format(%s) is supported for writing!\n", filename.c_str());
		}
	}

	inline std::vector<BYTE> bits(int align=1) const
	{
        const int pitch = (w * comp + align - 1) / align*align;

		std::vector<BYTE> data(pitch*h);
		for(int i=0; i<h; i++)
			std::copy_n(pixels.data()+i*w*comp, w*comp, data.data()+i*pitch);

		return data;
	}
};

