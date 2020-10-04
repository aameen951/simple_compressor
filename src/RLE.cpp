
COMPRESS_DEF(rle_compress)
{
  um MAX_RUN_LEN = 32767;
  um MIN_RUN_LEN = 4;
  sm last_copy_idx = -1;
  um last_copy_len = 0;
  for(um i=0; i < input.count; )
  {
    u8 c = input.data[i];
    u16 run_len = 1;
    // grap the following characters if they are the same as this character.
    while(i+run_len < input.count && run_len < MAX_RUN_LEN && c == input.data[i+run_len])run_len++;
    
    // If the run of repeated characters is above a minimum length
    if(run_len >= MIN_RUN_LEN)
    {
      // push the run length as 16bit Little Endian (the 16th bit is always zero).
      data_buffer_append(output, run_len);
      // push the repeated character.
      data_buffer_append(output, c);
      last_copy_idx = -1;
    }
    else
    {
      if(last_copy_idx == -1 || last_copy_len >= MAX_RUN_LEN)
      {
        // Initialize a data copy run.
        last_copy_idx = output->count;
        last_copy_len = 0;
        // push the initial run size as 16bit-LE (the 16th bit is always one).
        data_buffer_append(output, (u16)0x8000);
      }
      // ensure the run size doesn't exceed MAX_RUN_LEN
      run_len = MIN(run_len, MAX_RUN_LEN - last_copy_len);

      // append data to previous copy run.
      last_copy_len += run_len;
      for(um j=0; j<run_len; j++)data_buffer_append(output, input.data[i+j]);
      // overwrite the old run size with the new one as 16bit-LE (the 16th bit is always one).
      *(u16 *)(output->data+last_copy_idx) = 0x8000 + last_copy_len;
    }
    i += run_len;
  }
}

DECOMPRESS_DEF(rle_decompress)
{
  for(um i=0; i<input.count; )
  {
    // read 16bit-LE
    u16 run_len = *(u16 *)(input.data + i);
    i += 2;
    // If the 16th bit is set then it is a copy run
    if(run_len & 0x8000)
    {
      run_len = run_len & 0x7fff;
      data_buffer_append_memory(output, {input.data+i, run_len});
      i += run_len;
    }
    else // it is a repeated run.
    {
      u8 repeated_byte = input.data[i++];
      // write the repeated byte to output.
      data_buffer_ensure_room_for(output, run_len);
      for(um j=0; j<run_len; j++)
      {
        output->data[output->count++] = repeated_byte;
      }
    }
  }
}
