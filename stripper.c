// based off https://github.com/alexfru/SmallerC/issues/18#issuecomment-321761305
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

void error(char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	vprintf(format, vl);
	va_end(vl);
	exit(EXIT_FAILURE);
}

FILE* Fopen(const char* filename, const char* mode)
{
	FILE* stream = fopen(filename, mode);
	if (!stream)
		error("Can't open/create file '%s'\n", filename);
	return stream;
}

void Fseek(FILE* stream, long offset, int whence)
{
	int r = fseek(stream, offset, whence);
	if (r)
		error("Can't seek a file\n");
}

void Fread(void* ptr, size_t size, FILE* stream)
{
	size_t r = fread(ptr, 1, size, stream);
	if (r != size)
		error("Can't read a file\n");
}

void* Malloc(size_t size)
{
	void* p = malloc(size);
	if (!p)
		error("Out of memory\n");
	return p;
}

#define C_ASSERT(expr) extern char CAssertExtern[(expr)?1:-1]

typedef struct
{
	uint8_t  Signature[2];
	uint16_t PartPage;
	uint16_t PageCnt;
	uint16_t ReloCnt;
	uint16_t HdrSize;
	uint16_t MinAlloc;
	uint16_t MaxAlloc;
	uint16_t InitSs;
	uint16_t InitSp;
	uint16_t ChkSum;
	uint16_t InitIp;
	uint16_t InitCs;
	uint16_t ReloOff;
	uint16_t OverlayNo;
	uint16_t FirstRelo[2];
} tDosExeHeader;

int main(int argc, char* argv[])
{
	FILE* f;
	FILE* fo;
	tDosExeHeader header;
	uint32_t fsize;
	uint32_t hsize;
	uint32_t tsize;
	uint32_t dsize;
	uint32_t bsize;
	uint32_t asize;
	uint8_t* code;
	uint8_t* data;
	uintptr_t code_aligned;
	uintptr_t data_aligned;
	uint16_t cs;
	uint16_t ss;

	if (argc < 3)
		error("Usage: ldr.exe <program.exe> <output.bin>\n");

	f = Fopen(argv[1], "rb");
	Fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	Fseek(f, 0, SEEK_SET);
	printf("File size: %u\n", fsize);
	Fread(&header, sizeof header, f);

	if (header.Signature[0] != 'M' || header.Signature[1] != 'Z')
		error("Not MZ .EXE header\n");

	//fsize = (header.PageCnt - 1) * UINT32_C(512) + header.PartPage;

	hsize = header.HdrSize * UINT32_C(16);
	printf("  MZ .EXE header size: %u\n", hsize);
	if (header.HdrSize != 2)
		error("header.HdrSize: %u (expected: 2)\n", header.HdrSize);
	if (header.ReloCnt != 0)
		error("header.ReloCnt: %u (expected: 0)\n", header.ReloCnt);
	if (header.InitCs != 0xFFFE)
		error("header.InitCs: 0x%X (expected: 0xFFFE)\n", header.InitCs);

	tsize = (uint16_t)(header.InitSs - header.InitCs) * UINT32_C(16) - hsize;
	printf("  .text (follows MZ .EXE header) size: %u\n", tsize);
	if (tsize > 65536 - hsize)
		error(".text is too big\n");

	dsize = fsize - hsize - tsize;
	printf("  .rodata + .data (follow .text) size: %u\n", dsize);
	if (dsize > 65536)
		error(".rodata + .data is too big\n");

	printf("Not stored in file:\n");
	bsize = header.InitSp - dsize;
	printf("  .bss + stack (follow .*data) size: %u\n", bsize);
	if (header.InitSp < dsize)
		error("header.InitSp is too small\n");

	printf("Memory DOS allocates after .*data for .bss + stack + etc:\n");
	asize = header.MinAlloc * UINT32_C(16);
	printf("  header.MinAlloc * 16: %u\n", asize);
	if (asize < bsize)
		error("header.MinAlloc is too small\n");

	printf("  header.MaxAlloc * 16: %u\n", header.MaxAlloc * UINT32_C(16));
	if (header.MaxAlloc < header.MinAlloc)
		error("header.MaxAlloc < header.MinAlloc\n");

	/*code = Malloc(tsize + 15);
	memset(code, 0xCC, tsize + 15); // fill with int3
	code_aligned = ((uintptr_t)code + 15) / 16 * 16;
	Fread((void*)code_aligned, tsize, f);

	data = Malloc(header.InitSp + asize + 15);
	memset(data, 0x80, header.InitSp + asize + 15); // fill with 0x80
	data_aligned = ((uintptr_t)data + 15) / 16 * 16;
	Fread((void*)data_aligned, dsize, f);*/

	fo = Fopen(argv[2], "wb");

	code = (uint8_t*)malloc(tsize + 15);
	Fread((void*)code, tsize, f);
	fwrite(code, tsize, 1, fo);
	free(code);

	data = (uint8_t*)malloc(header.InitSp + asize + 15);
	Fread((void*)data, dsize, f);
	fwrite(data, dsize, 1, fo);
	free(data);

	fclose(fo);

	fclose(f);

	uint32_t fixup = (header.InitCs * 16 + header.InitIp) & 0xFFFFF; /* 20 bits */
	uint32_t rem;

	uint32_t new_ip = fixup % 0x1000;
	uint32_t new_cs = (fixup - new_ip) / 0x1000;

	printf("entry CS:IP -> %.4X:%.4X (0x%.8X), SP: %.4X\r\n", 
		new_cs, 
		new_ip, 
		fixup, 
		header.InitSp);

	if ((new_ip != 0) || (new_cs != 0))
	{
		printf("needs a jmp!");
	}

    return 0;
}


