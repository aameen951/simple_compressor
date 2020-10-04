#include <stdio.h>

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
void read_file_result_free(DataView *data){
  free(data->data);
  data->data = NULL;
  data->count = 0;
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

