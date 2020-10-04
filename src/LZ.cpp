
struct FindPrefixResult
{
  um run_start;
  um run_len;
};
FindPrefixResult find_best_prefix_match(DataView data, um a_idx, um b_idx, um max_run_size)
{
  FindPrefixResult result = {};
  for(um j=a_idx; j<b_idx; j++)
  {
    um run_len = 0;
    while(b_idx+run_len < data.count && run_len < max_run_size)
    {
      if(data.data[j+run_len] != data.data[b_idx+run_len])
      {
        break;
      }
      run_len++;
    }
    if(run_len >= result.run_len)
    {
      result.run_len = run_len;
      result.run_start = j;
    }
  }
  return result;
}

COMPRESS_DEF(lz_compress)
{
  um MAX_RUN_SIZE = (1 << 16) - 1;
  sm MAX_LOOKBACK = (1 << 8) - 1;
  um MIN_RUN_LEN = 8;
  
  sm last_copy_idx = -1;
  sm last_copy_len = 0;
  for(sm i=0; i<input.count; )
  {
    um start = MAX(0, i-MAX_LOOKBACK);
    auto best_match = find_best_prefix_match(input, start, i, MAX_RUN_SIZE);

    if(best_match.run_len < MIN_RUN_LEN)
    {
      if(last_copy_idx == -1 || last_copy_len >= MAX_RUN_SIZE)
      {
        last_copy_idx = output->count+1;
        last_copy_len = 0;
        data_buffer_append(output, (u8)0x00);
        data_buffer_append(output, (u16)0x0000);
      }

      last_copy_len++;
      data_buffer_append(output, input.data[i]);
      *(u16 *)(output->data+last_copy_idx) = last_copy_len;
      i++;
    }
    else
    {
      data_buffer_append(output, (u8)(i-best_match.run_start));
      data_buffer_append(output, (u16)best_match.run_len);
      last_copy_idx = -1;
      i += best_match.run_len;
    }
  }
}

DECOMPRESS_DEF(lz_decompress)
{
  um i;
  for(i=0; i<input.count-3; )
  {
    u8 offset = input.data[i++];
    u16 length = *(u16 *)(input.data + i);
    i += 2;

    if(offset == 0)
    {
      if(i+length > input.count)return false;
      data_buffer_append_memory(output, {input.data+i, length});
      i += length;
    }
    else
    {
      if((sm)output->count - (sm)offset < 0)return false;
      data_buffer_ensure_room_for(output, length);
      for(um j=0; j<length; j++)
      {
        output->data[output->count+j] = output->data[output->count-offset+j];
      }
      output->count += length;
    }
  }
  if(i != input.count)return false;
  return true;
}
