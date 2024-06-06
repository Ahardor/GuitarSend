uint32_t compress() {
    esp_audio_err_t ret;
    void *enc_handle = NULL;
    int in_read = 0;
    int in_frame_size = 0;
    int out_frame_size = 0;
    int times = 0;


    esp_adpcm_enc_config_t config = ESP_ADPCM_ENC_CONFIG_DEFAULT();
    config.sample_rate = 20000;
    config.channel = 1;
    config.bit_per_sample = 16;

    ret = esp_adpcm_enc_open(&config, sizeof(esp_adpcm_enc_config_t), &enc_handle);

    if (ret != 0) {
        printf("Fail to create encoder handle.");
    }

    ret = esp_adpcm_enc_get_frame_size(enc_handle, &in_frame_size, &out_frame_size);

    temp_buf = (uint8_t*)calloc(1, out_frame_size);
    times = 4;
    in_frame_size *= times;
    out_frame_size *= times;
    Serial.println(in_frame_size);
    Serial.println(out_frame_size);
    // Encode process
    esp_audio_enc_in_frame_t in_frame = { 0 };
    esp_audio_enc_out_frame_t out_frame = { 0 };
    in_frame.buffer = input_buffer;
    in_frame.len = in_frame_size;
    out_frame.buffer = temp_buf;
    out_frame.len = out_frame_size;
    // while ((in_read = fread(inbuf, 1, in_frame_size, in_file)) > 0) {
    //     if (in_read < in_frame_size) {
    //         memset(inbuf + in_read, 0, in_frame_size - in_read);
    //     }
    ret = esp_adpcm_enc_process(enc_handle, &in_frame, &out_frame);
    if (ret != ESP_AUDIO_ERR_OK) {
        printf("audio encoder process failed.\n");
    }
        // fwrite(outbuf, 1, out_frame.encoded_bytes, out_file);
    // }
    if (enc_handle) {
        esp_adpcm_enc_close(enc_handle);
    }
    return out_frame.encoded_bytes;
}