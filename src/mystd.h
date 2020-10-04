#include <stdint.h>
#include <string.h>
#include <stdlib.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef size_t um;
typedef intptr_t sm;

#ifndef MAX
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a, b) ( (a) < (b) ? (a) : (b) )
#endif

struct DataView
{
  u8 *data;
  um count;
};
struct DataBuffer
{
  u8 *data;
  um size, count;

  DataView to_data_view(){
    return {data, count};
  }
};

#ifdef MYSTD_IMPLEMENTATION

void data_buffer_ensure_room_for(DataBuffer *buffer, um room_for)
{
  if(buffer->count+room_for > buffer->size)
  {
    um new_size = MAX(8, 2 * buffer->size);
    new_size = MAX(new_size, buffer->count+room_for);
    buffer->data = (u8 *)realloc(buffer->data, new_size);
    buffer->size = new_size;
  }
}
void data_buffer_append(DataBuffer *buffer, u8 byte)
{
  data_buffer_ensure_room_for(buffer, 1);
  buffer->data[buffer->count] = byte;
  buffer->count += 1;
}
void data_buffer_free(DataBuffer *buffer)
{
  free(buffer->data);
  buffer->data = NULL;
  buffer->count = 0;
  buffer->size = 0;
}
void data_buffer_append(DataBuffer *buffer, u16 two_bytes)
{
  data_buffer_ensure_room_for(buffer, 2);
  *(u16 *)(buffer->data + buffer->count) = two_bytes;
  buffer->count += 2;
}
void data_buffer_append_memory(DataBuffer *buffer, DataView memory)
{
  data_buffer_ensure_room_for(buffer, memory.count);
  memcpy(buffer->data+buffer->count, memory.data, memory.count);
  buffer->count += memory.count;
}
bool data_equal(DataView a, DataView b)
{
  if(a.count != b.count)return false;
  return memcmp(a.data, b.data, a.count) == 0;
}

#endif
