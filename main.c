
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION_MAJOR
	#define VERSION_MAJOR 1
#endif

#ifndef VERSION_MINOR
	#define VERSION_MINOR 0
#endif

typedef struct {
	#define IPX_SIGNATURE 0x495058
	uint64_t signature;
	
	uint64_t version_major;
	uint64_t version_minor;
	
	uint64_t width;
	uint64_t height;
	uint64_t bpp; // 24 or 32 bits
} ipx_header_t;

#pragma pack(push, 1)
	typedef struct {
		uint16_t magic;
		uint32_t file_size;
		uint16_t reserved_1, reserved_2;
		uint32_t offset;
	} bmp_header_t;
	
	typedef struct {
		uint32_t size;
		int32_t width, height;
		uint16_t planes, bpp;
		uint32_t compression_type, image_bytes;
		int64_t pixels_per_meter_x, pixels_per_meter_y;
		uint32_t colour_count, important_colours;
	} bmp_info_header_t;
#pragma pack(pop)

int main(int argc, char* argv[]) {
	printf("Microsoft BMP to Inobulles IPX file converter v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
	
	if (argc < 2) {
		printf("ERROR You must specify which BMP file you want to convert\n");
		return 1;
	}
	
	for (int i = 1; i < argc; i++) {
		const char* argument = argv[i]; // parse arguments here
	}
	
	// load bmp
	
	printf("Opening BMP files (%s) ...\n", argv[1]);
	FILE* file = fopen(argv[1], "rb");
	
	if (!file) {
		printf("ERROR Failed to open BMP file\n");
		return 1;
		
	} else {
		printf("Opened BMP file, reading ...\n");
	}
	
	fseek(file, 0L, SEEK_END);
	uint32_t bytes = ftell(file);
	rewind(file);
	
	char* original_data = (char*) malloc(bytes);
	fread(original_data, 1, bytes, file);
	fclose(file);
	
	char* data = original_data;
	bmp_header_t header = *((bmp_header_t*) data);
	data += sizeof(bmp_header_t);
	
	if (header.magic != 0x4D42) {
		printf("ERROR File is not BMP\n");
		free(original_data);
		return 1;
	}
	
	bmp_info_header_t info_header = *((bmp_info_header_t*) data);
	data = original_data + header.offset;
	
	uint8_t* data8 = (uint8_t*) data;
	char* final_data = (char*) malloc(info_header.image_bytes);
	
	uint32_t components = info_header.bpp >> 3;
	uint32_t pitch = info_header.width * components;
	
	for (uint32_t i = 0; i < info_header.height; i++) { // swizzle components and flip vertically
		uint8_t* dest = final_data + i * pitch;
		uint8_t* src = data8 + (info_header.height - i - 1) * pitch;
		
		if (components == 4) for (uint32_t j = 0; j < pitch; j += components) {
			dest[j + 0] = src[j + 2];
			dest[j + 1] = src[j + 1];
			dest[j + 2] = src[j + 0];
			dest[j + 3] = src[j + 3];
			
		} else if (components == 3) for (uint32_t j = 0; j < pitch; j += components) {
			dest[j + 0] = src[j + 2];
			dest[j + 1] = src[j + 1];
			dest[j + 2] = src[j + 0];
		}
	}
	
	free(original_data);
	
	// write to ipx
	
	const char* ipx_path = argc > 2 ? argv[2] : "output.ipx";
	printf("BMP file read, writing to IPX file (%s) ...\n", ipx_path);
	
	FILE* ipx_file = fopen(ipx_path, "wb");
	if (!ipx_file) {
		printf("ERROR Failed to open IPX file\n");
		free(final_data);
		return 1;
	}
	
	ipx_header_t ipx;
	memset(&ipx, 0, sizeof(ipx));
	ipx.signature = IPX_SIGNATURE;
	
	ipx.version_major = VERSION_MAJOR;
	ipx.version_minor = VERSION_MINOR;
	
	ipx.width  = info_header.width;
	ipx.height = info_header.height;
	
	ipx.bpp = info_header.bpp;
	fwrite(&ipx, 1, sizeof(ipx), ipx_file);
	
	fwrite(final_data, 1, info_header.image_bytes, ipx_file);
	printf("Finished converting successfully\n");
	
	free(final_data);
	fclose(ipx_file);
	
	return 0;
}
