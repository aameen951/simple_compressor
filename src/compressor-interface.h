
#define COMPRESS_DEF(name) void name(DataView input, DataBuffer *output)
#define DECOMPRESS_DEF(name) void name(DataView input, DataBuffer *output)

typedef COMPRESS_DEF(compress_fn);
typedef COMPRESS_DEF(decompress_fn);

