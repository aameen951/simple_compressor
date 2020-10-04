#define MYSTD_IMPLEMENTATION
#include "mystd.h"

#include "file.h"

#include "compressor-interface.h"
#include "RLE.cpp"
#include "LZ.cpp"

#define VERIFY_COMPRESSION 1

void usage()
{
  printf("\n");
  printf("  Usage: simple_compressor compress   <raw-file>    <output-file>\n");
  printf("         simple_compressor decompress <output-file> <raw-file>\n");
  printf("\n");
}

struct CompressorDesc
{
  char *name;
  compress_fn *compress;
  decompress_fn *decompress;
};

#define COMPRESSIO_RLE 0
#define COMPRESSIO_LZ 1
CompressorDesc compressors[] = {
  {"RLE", rle_compress, rle_decompress},
  {"LZ", lz_compress, lz_decompress},
};

enum AppCommand
{
  AppCommand_Unknown,
  AppCommand_Compress,
  AppCommand_Decompress,
};

int main(int argc, char **argv)
{
  if(argc != 4)
  {
    printf("\nError: incorrect number of arguments.\n");
    usage();
    return 1;
  }

  char *command_str = argv[1];
  char *input_file_path = argv[2];
  char *output_file_path = argv[3];

  AppCommand command = AppCommand_Unknown;
  if(strcmpi(command_str, "compress") == 0)
  {
    command = AppCommand_Compress;
  }
  else if(strcmpi(command_str, "decompress") == 0)
  {
    command = AppCommand_Decompress;
  }
  else
  {
    printf("\nError: unknown command '%s'.\n", command_str);
    usage();
    return 2;
  }

  auto read_result = read_input_file(input_file_path);
  if(!read_result.succeeded)
  {
    printf("\nError: failed to load input file '%s'\n", input_file_path);
    return 3;
  }
  auto input_data = read_result.data;

  auto compressor = compressors[COMPRESSIO_LZ];

  switch(command)
  {
    case AppCommand_Compress:{

      DataBuffer compressed_data = {};
      compressor.compress(input_data, &compressed_data);

      if(VERIFY_COMPRESSION)
      {
        DataBuffer decompressed_data = {};
        auto succeeded = compressor.decompress(compressed_data.to_data_view(), &decompressed_data);
        if(!succeeded || !data_equal(input_data, decompressed_data.to_data_view()))
        {
          printf("\nError: We didn't get the same data after decompressing the compressed output!\n\n");
          return 5;
        }
        data_buffer_free(&decompressed_data);
      }

      if(!write_output_file(output_file_path, compressed_data.to_data_view()))
      {
        printf("\nError: Failed to write the compressed file to '%s'\n\n", output_file_path);
        return 4;
      }

      printf("\n");
      printf("Original size: %llu\n", input_data.count);
      printf("Compressed size: %llu\n", compressed_data.count);
      printf("Compression percentage: %%%.2f\n", 100.0 * compressed_data.count/input_data.count);
      printf("\n");

      data_buffer_free(&compressed_data);
      
    }break;
    case AppCommand_Decompress:{
      
      DataBuffer uncompressed_data = {};
      if(!compressor.decompress(input_data, &uncompressed_data))
      {
        printf("\nError: the file is corrupted. Could not decompress\n\n");
        return 5;
      }


      if(!write_output_file(output_file_path, uncompressed_data.to_data_view()))
      {
        printf("\nError: Failed to write the decompressed file to '%s'\n\n", output_file_path);
        return 4;
      }

      data_buffer_free(&uncompressed_data);

    }break;
    default:
    {
      printf("\nInternal Error: unhandled command '%d'\n", command);
      return 999;
    }break;
  }
  read_file_result_free(&input_data);
  return 0;
}
