#include "mystd.h"

#include "compressor-interface.h"
#include "RLE.cpp"
#include "LZ.cpp"

#include <stdio.h>

void usage()
{
  printf("\n");
  printf("  Usage: simple_compressor compress   <raw-file>    <output-file>\n");
  printf("         simple_compressor decompress <output-file> <raw-file>\n");
  printf("\n");
}
struct ReadFileResult
{
  bool succeeded;
  DataView data;
};

ReadFileResult read_input_file(char *file_path){
  auto file = fopen(file_path, "rb");
  if(file)
  {
    fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    fseek(file, 0, SEEK_SET);
    auto buffer = (u8 *)malloc(size);
    fread(buffer, size, 1, file);
    fclose(file);
    return {true, {buffer, (um)size}};
  }
  return {false};
}
bool write_output_file(char *file_path, DataView data){
  auto file = fopen(file_path, "wb");
  if(file)
  {
    fwrite(data.data, data.count, 1, file);
    fclose(file);
    return true;
  }
  return false;
}

bool data_equal(DataView a, DataView b)
{
  if(a.count != b.count)return false;
  for(um i=0; i<a.count; i++)
  {
    if(a.data[i] != b.data[i])return false;
  }
  return true;
}


struct CompressorDesc
{
  char *name;
  compress_fn *compress;
  decompress_fn *decompress;
};

CompressorDesc compressors[] = {
  {"RLE", rle_compress, rle_decompress},
  {"LZ", lz_compress, lz_decompress},
};

int main(int argc, char **argv)
{
  if(argc == 4)
  {
    char *command = argv[1];
    char *input_file_path = argv[2];
    char *output_file_path = argv[3];

    auto compressor = compressors[1];

    if(strcmpi(command, "compress") == 0)
    {
      auto validate_output = true;

      auto read_result = read_input_file(input_file_path);
      if(read_result.succeeded)
      {
        auto input_data = read_result.data;
        DataBuffer compressed_data = {};
        compressor.compress(input_data, &compressed_data);

        if(validate_output)
        {
          DataBuffer decompressed_data = {};
          compressor.decompress(compressed_data.to_data_view(), &decompressed_data);
          if(!data_equal(input_data, decompressed_data.to_data_view()))
          {
            printf("\nError: We didn't get the same data after decompressing the compressed output!\n\n");
          }
        }

        write_output_file(output_file_path, compressed_data.to_data_view());

        printf("\n");
        printf("Original size: %llu\n", input_data.count);
        printf("Compressed size: %llu\n", compressed_data.count);
        printf("Compression percentage: %%%.2f\n", 100.0 * compressed_data.count/input_data.count);
        printf("\n");
      }
      else
      {
        printf("\nError: failed to load input file '%s'\n", input_file_path);
      }
    }
    else if(strcmpi(command, "decompress") == 0)
    {

      auto read_result = read_input_file(input_file_path);
      if(read_result.succeeded)
      {
        auto input_data = read_result.data;
        DataBuffer uncompressed_data = {};
        compressor.decompress(input_data, &uncompressed_data);
        write_output_file(output_file_path, uncompressed_data.to_data_view());
      }
      else
      {
        printf("\nError: failed to load input file '%s'\n", input_file_path);
      }
    }
    else 
    {
      usage();
    }
  }
  else
  {
    usage();
  }
}
