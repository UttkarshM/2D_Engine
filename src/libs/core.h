#pragma once

#include <cmath>
#include <stdio.h>
#include <cstddef>
#include <cstring>
#include <malloc.h>
#include <stdio.h>
#include <sys/stat.h>


#define ROOT_DIR "C:/Users/uttka/Desktop/god/"

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#endif

#define ArraySize(x) (sizeof(x)/sizeof((x)[0]))

#define b8 char
#define BIT(x) 1<<(x)
#define KB(x) ((unsigned long long) 1024*x)
#define MB(x) ((unsigned long long) 1024*KB(x))
#define GB(x) ((unsigned long long) 1024*MB(x))

//for wav files
constexpr int NUM_CHANNELS = 2;
constexpr int SAMPLE_RATE = 44100;



//logging
enum TextColor
{  
  TEXT_COLOR_BLACK,
  TEXT_COLOR_RED,
  TEXT_COLOR_GREEN,
  TEXT_COLOR_YELLOW,
  TEXT_COLOR_BLUE,
  TEXT_COLOR_MAGENTA,
  TEXT_COLOR_CYAN,
  TEXT_COLOR_WHITE,
  TEXT_COLOR_BRIGHT_BLACK,
  TEXT_COLOR_BRIGHT_RED,
  TEXT_COLOR_BRIGHT_GREEN,
  TEXT_COLOR_BRIGHT_YELLOW,
  TEXT_COLOR_BRIGHT_BLUE,
  TEXT_COLOR_BRIGHT_MAGENTA,
  TEXT_COLOR_BRIGHT_CYAN,
  TEXT_COLOR_BRIGHT_WHITE,
  TEXT_COLOR_COUNT
};


template <typename ...Args>
void _log(char* prefix, char* msg, TextColor textColor, Args... args)
{
  static char* TextColorTable[TEXT_COLOR_COUNT] = 
  {    
    "\x1b[30m", // TEXT_COLOR_BLACK
    "\x1b[31m", // TEXT_COLOR_RED
    "\x1b[32m", // TEXT_COLOR_GREEN
    "\x1b[33m", // TEXT_COLOR_YELLOW
    "\x1b[34m", // TEXT_COLOR_BLUE
    "\x1b[35m", // TEXT_COLOR_MAGENTA
    "\x1b[36m", // TEXT_COLOR_CYAN
    "\x1b[37m", // TEXT_COLOR_WHITE
    "\x1b[90m", // TEXT_COLOR_BRIGHT_BLACK
    "\x1b[91m", // TEXT_COLOR_BRIGHT_RED
    "\x1b[92m", // TEXT_COLOR_BRIGHT_GREEN
    "\x1b[93m", // TEXT_COLOR_BRIGHT_YELLOW
    "\x1b[94m", // TEXT_COLOR_BRIGHT_BLUE
    "\x1b[95m", // TEXT_COLOR_BRIGHT_MAGENTA
    "\x1b[96m", // TEXT_COLOR_BRIGHT_CYAN
    "\x1b[97m", // TEXT_COLOR_BRIGHT_WHITE
  };

  char formatBuffer[8192] = {};
  sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);

  char textBuffer[8912] = {};
  sprintf(textBuffer, formatBuffer, args...);

  puts(textBuffer);
}


#define EN_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define EN_WARN(msg, ...) _log("WARN: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define EN_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);

#define EN_ASSERT(x, msg, ...)    \
{                                 \
  if(!(x))                        \
  {                               \
    EN_ERROR(msg, ##__VA_ARGS__); \
    DEBUG_BREAK();                \
    EN_ERROR("Assertion HIT!")    \
  }                               \
}
template<typename T, int N>
struct Array
{

static constexpr int maxElements = N;
  int count = 0;
  T elements[N];

  T& operator[](int idx)
  {
    EN_ASSERT(idx >= 0, "idx negative!");
    EN_ASSERT(idx < count, "Idx out of bounds!");
    return elements[idx];
  }

  int add(T element)
  {
    EN_ASSERT(count < maxElements, "Array Fuldl!");
    elements[count] = element;
    return count++;
  }

  void remove_idx_and_swap(int idx)
  {
    EN_ASSERT(idx >= 0, "idx negative!");
    EN_ASSERT(idx < count, "idx out of bounds!");
    elements[idx] = elements[--count];
  }

  void clear()
  {
    count = 0;
  }

  bool is_full()
  {
    return count == N;
  }
};

//Bump allocator

struct BumpAllocator {
  size_t capacity;
  size_t used;
  char *memory;
};

BumpAllocator make_bump_allocator(size_t size) {
  BumpAllocator ba = {};
  ba.memory = (char *)malloc(size);
  if (ba.memory) {
    ba.capacity = size;
    memset(ba.memory, 0, size);
  } else {
    EN_ERROR("Bump allocation failed");
  }
  ba.capacity = size;
  return ba;
}

char *bump_alloc(BumpAllocator *bumpAllocator, size_t size) {
  char *result = nullptr;

  size_t allignedSize =
      (size + 7) & ~7; // this makes sure the first 4 bits are 0
  if (bumpAllocator->used + allignedSize <= bumpAllocator->capacity) {
    result = bumpAllocator->memory + bumpAllocator->used;
    bumpAllocator->used += allignedSize;
  }
  return result;
}


long long get_timestamp(const char *file) {
  struct stat file_stat = {};
  stat(file, &file_stat);

  return file_stat.st_mtime;
}

bool file_exists(char *filePath) {
  auto file = fopen(filePath, "r");
  if (!file) {
    return false;
  }
  fclose(file);
  return true;
}

long get_file_size(char *filePath) {
  EN_ASSERT(filePath, "No filePath supplied");
  long fileSize = 0;
  auto file = fopen(filePath, "r");
  if (!file) {
    EN_ERROR("error in opening file", filePath);
    return 0;
  }
  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0,
        SEEK_SET); // sets the file streams position back to the beginning
  fclose(file);
  return fileSize;
}

char *read_file(char *filePath, int *fileSize, char *buffer) {
  EN_ASSERT(filePath, "file Path is empty");
  EN_ASSERT(fileSize, "file Size is empty");
  EN_ASSERT(buffer, "buffer not initialized");

  *fileSize = 0;
  FILE *file = fopen(filePath, "rb");
  if (!file) {
    EN_ERROR("failed to open the file", filePath);
    return nullptr;
  }
  // file size
  fseek(file, 0, SEEK_END);
  *fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  memset(buffer, 0, *filePath + 1);
  fread(buffer, sizeof(char), *fileSize, file);
  fclose(file);
  return buffer;
}

char *read_file(char *filePath, int *fileSize, BumpAllocator *bumpAllocator) {
  char *file = nullptr;
  long fileSize2 = get_file_size(filePath);

  if (fileSize2) {
    char *buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

    file = read_file(filePath, fileSize, buffer);
  }

  return file;
}

void write_file(char *filePath, int size, char *buffer) {
  EN_ASSERT(filePath, "file Path for writing is not given");
  EN_ASSERT(size, "size not provided for reading the file");
  EN_ASSERT(buffer, "buffer not provided");

  FILE *file = fopen(filePath, "w");

  if (!file) {
    EN_ERROR("error in opening the file");
  }

  fwrite(buffer, sizeof(char), size, file);
  fclose(file);
}

bool copy_file(char *fileName, char *outputName, char *buffer) {
  int fileSize = 0;
  char *data = read_file(fileName, &fileSize, buffer);

  FILE *outputFile = fopen(outputName, "wb");
  if (!outputFile) {
    EN_ERROR("error in opening the file to write the copy to");
    return false;
  }
  int result = fwrite(data, sizeof(char), fileSize, outputFile);
  if (!result) {
    EN_ERROR("error in writing the copy to the output file");
    return false;
  }
  fclose(outputFile);
  return true;
}

bool copy_file(char *fileName, char *outputName, BumpAllocator *bumpAllocator) {
  char *file = nullptr;
  long fileSize2 = get_file_size(fileName);

  if (fileSize2) {
    char *buffer = bump_alloc(
        bumpAllocator,
        fileSize2 +
            1); // 1 extra byte for the terminating character (null character)
    return copy_file(fileName, outputName, buffer);
  }
  return true;
}

//maths


int sign(int x)
{
  return (x >= 0)? 1 : -1;
}

float sign(float x)
{
  return (x >= 0.0f)? 1.0f : -1.0f;
}

int min(int a, int b)
{
  return (a < b)? a : b;
}

int max(int a, int b)
{
  return (a > b)? a : b;
}

long long max(long long a, long long b)
{
  if(a > b)
  {
    return a;
  }

  return b;
}

float max(float a, float b)
{
  if(a > b)
  {
    return a;
  }

  return b;
}

float min(float a, float b)
{
  if(a < b)
  {
    return a;
  }

  return b;
}

float approach(float current, float target, float increase)
{
  if(current < target)
  {
    return min(current + increase, target);
  }
  return max(current - increase, target);
}

float lerp(float a,float b,float t){
  return a + (b - a)*t;
}


struct IVec2{
  int x;
  int y;

  IVec2 operator-(IVec2 other)
  {
    return {x - other.x, y - other.y};
  }
  IVec2& operator-=(int value)
  {
    x-=value;
    y-=value;
    return *this;
  }
  IVec2& operator+=(int value){
    x+=value;
    y+=value;
    return *this;
  }

  IVec2 operator/(int scalar){
    return {x/scalar, y/scalar};
  }
};


struct Vec2{
  float x;
  float y;

  Vec2 operator/(float scalar)
  {
    return {x / scalar, y / scalar};
  }

  Vec2 operator-(Vec2 other)
  {
    return {x - other.x, y - other.y};
  }
};

Vec2 vec_2(IVec2 v)
{
  return Vec2{(float)v.x, (float)v.y};
}

Vec2 lerp(Vec2 a, Vec2 b, float t){
  Vec2 result;
  result.x = lerp(a.x,b.x,t);
  result.y = lerp(a.y,b.y,t);
  return result;
}
IVec2 lerp(IVec2 a, IVec2 b,float t){
  IVec2 result;

  result.x = (int)floorf(lerp((float)a.x,(float)b.x,t));
  result.y = (int)floorf(lerp((float)a.y,(float)b.y,t));
  return result;
}

struct Vec4
{
  union
  {
    float values[4];
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
    
    struct
    {
      float r;
      float g;
      float b;
      float a;
    };
  };

  float& operator[](int idx)
  {
    return values[idx];
  }
};

struct Mat4
{
  union 
  {
    Vec4 values[4];
    struct
    {
      float ax;
      float bx;
      float cx;
      float dx;

      float ay;
      float by;
      float cy;
      float dy;

      float az;
      float bz;
      float cz;
      float dz;
      
      float aw;
      float bw;
      float cw;
      float dw;
    };
  };

  Vec4& operator[](int col)
  {
    return values[col];
  }
};

struct Rect{
  Vec2 pos;
  Vec2 size;
};

struct IRect{
  IVec2 pos;
  IVec2 size;
};


// checking if it comes under the rectangle(bounding box)
bool point_in_rect(Vec2 point, Rect rect)
{
  return(
    point.x >= rect.pos.x &&
    point.x <= rect.pos.x + rect.size.x &&
    point.y >= rect.pos.y &&
    point.y <= rect.pos.y + rect.pos.y
  );
}

bool point_in_rect(Vec2 point, IRect rect){
  return(
    point.x >= rect.pos.x &&
    point.x <= rect.pos.x + rect.size.x &&
    point.y >= rect.pos.y &&
    point.y <= rect.pos.y + rect.size.y
  );
}

bool rect_collision(IRect a, IRect b){

  return (
  a.pos.x + a.size.x >= b.pos.x &&
  a.pos.x <= b.pos.x + b.size.x &&
  a.pos.y + a.size.y >= b.pos.y &&
  a.pos.y < b.pos.y + b.size.y);

}



Mat4 orthographic_projection(float left, float right, float top, float bottom)
{
  Mat4 result = {};
  result.aw = -(right + left) / (right - left);
  result.bw = (top + bottom) / (top - bottom);
  result.cw = 0.0f; // Near Plane
  result[0][0] = 2.0f / (right - left);
  result[1][1] = 2.0f / (top - bottom); 
  result[2][2] = 1.0f / (1.0f - 0.0f); // Far and Near
  result[3][3] = 1.0f;

  return result;
}

//more references https://stackoverflow.com/questions/13660777/c-reading-the-data-part-of-a-wav-file
struct WAVHeader
{
  // Riff Chunk
	unsigned int riffChunkId;
	unsigned int riffChunkSize;
	unsigned int format;

  // Format Chunk
	unsigned int formatChunkId;
	unsigned int formatChunkSize;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;

  // Data Chunk
	unsigned char dataChunkId[4];
	unsigned int dataChunkSize;
};

struct WAVFile
{
	WAVHeader header;
	char dataBegin;
};

WAVFile* load_wav(char* path, BumpAllocator* bumpAllocator)
{
	int fileSize = 0;
	WAVFile* wavFile = (WAVFile*)read_file(path, &fileSize, bumpAllocator);
	if(!wavFile) 
  { 
    EN_ASSERT(0, "Failed to load Wave File: %s", path);
    return nullptr;
  }

	EN_ASSERT(wavFile->header.numChannels == NUM_CHANNELS, 
            "We only support 2 channels for now!");
	EN_ASSERT(wavFile->header.sampleRate == SAMPLE_RATE, 
            "We only support 44100 sample rate for now!");

	EN_ASSERT(memcmp(&wavFile->header.dataChunkId, "data", 4) == 0, 
						"WAV File not in propper format");

	return wavFile;
}





