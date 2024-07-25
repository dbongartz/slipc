/* SLIPC (Serial Line Internet Protocol in C) encoder and decoder.
 *
 * This library provides functions to encode and decode data using the SLIP
 * protocol according to RFC 1055: www.ietf.org/rfc/rfc1055.txt
 *
 * It supports a writer and reader interface to allow for easy integration with
 * different I/O systems.
 *
 * LICENSE: This library is released under the MIT License.
 */
#include "slipc.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \brief Write the END byte to the writer.
 *
 * \param writer Writer structure
 *
 * \retval SLIPC_WRITER_OK Operation successful
 * \retval SLIPC_WRITER_EOF Can't write more data
 * \retval SLIPC_WRITER_ERROR Error occurred
 */
static slipc_writer_result_t slipc_write_end_byte(slipc_writer_t *writer);

/**
 * \brief Skip to the start byte in the reader.
 *
 * \param reader Reader structure
 *
 * \retval SLIPC_DECODER_MORE Start byte found
 * \retval SLIPC_DECODER_NOT_FOUND Start byte not found
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
static slipc_decoder_result_t slipc_skip_to_start(slipc_reader_t *reader);

/**
 * \brief Decode a packet without the start byte.
 *
 * \param writer Writer to write the decoded data
 * \param buf Buffer containing the data to decode
 * \param len Length of the buffer
 *
 * \retval SLIPC_DECODER_EOF End of packet
 * \retval SLIPC_DECODER_MORE More data to decode
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
static slipc_decoder_result_t
slipc_decode_without_startbyte(slipc_writer_t *writer, uint8_t const *buf,
                               size_t len);

slipc_encoder_result_t slipc_encode_byte(slipc_writer_t *writer, uint8_t byte) {
  assert(writer);

  uint8_t stuffbyte = 0;
  switch (byte) {
  case SLIPC_END:
    byte = SLIPC_ESC;
    stuffbyte = SLIPC_ESC_END;
    break;
  case SLIPC_ESC:
    byte = SLIPC_ESC;
    stuffbyte = SLIPC_ESC_ESC;
    break;
  }

  if (stuffbyte != 0) {
    if (writer->write(writer->user_ctx, &byte, 1) != SLIPC_WRITER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }
    byte = stuffbyte;
  }

  if (writer->write(writer->user_ctx, &byte, 1) != SLIPC_WRITER_OK) {
    return SLIPC_ENCODER_IO_ERROR;
  }

  return SLIPC_ENCODER_OK;
}

void slipc_encoder_init(slipc_encoder_t *self, bool startbyte) {
  assert(self);
  *self = slipc_encoder_new(startbyte);
}

slipc_encoder_t slipc_encoder_new(bool startbyte) {
  slipc_encoder_t self = {
      .startbyte = startbyte,
  };
  return self;
}

slipc_encoder_result_t slipc_encoder_transfer(slipc_encoder_t *self,
                                              slipc_reader_t *reader,
                                              slipc_writer_t *writer) {
  assert(self);
  assert(writer && writer->write);
  assert(reader && reader->read);

  uint8_t byte;

  if (self->startbyte) {
    if (slipc_write_end_byte(writer) != SLIPC_WRITER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }
  }

  while (1) {
    size_t len = 1;
    slipc_reader_result_t res = reader->read(reader->user_ctx, &byte, &len);

    if (len > 1) {
      return SLIPC_ENCODER_IO_ERROR;
    }

    if (len != 1 || res == SLIPC_READER_ERROR) {
      return SLIPC_ENCODER_IO_ERROR;
    }

    if (slipc_encode_byte(writer, byte) != SLIPC_ENCODER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }

    if (res == SLIPC_READER_EOF) {
      if (slipc_write_end_byte(writer) != SLIPC_WRITER_OK) {
        return SLIPC_ENCODER_IO_ERROR;
      }
      return SLIPC_ENCODER_OK;
    }
  }
}

slipc_encoder_result_t slipc_encode_packet(slipc_writer_t *writer,
                                           uint8_t const *buf, size_t len,
                                           bool startbyte) {
  assert(writer && writer->write);
  assert(buf);

  if (startbyte) {
    if (slipc_write_end_byte(writer) != SLIPC_WRITER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }
  }

  for (size_t i = 0; i < len; i += 1) {
    if (slipc_encode_byte(writer, buf[i]) != SLIPC_ENCODER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }
  }

  if (slipc_write_end_byte(writer) != SLIPC_WRITER_OK) {
    return SLIPC_ENCODER_IO_ERROR;
  }
  return SLIPC_ENCODER_OK;
}

void slipc_decoder_init(slipc_decoder_t *self, bool startbyte) {
  assert(self);
  *self = slipc_decoder_new(startbyte);
}

slipc_decoder_t slipc_decoder_new(bool startbyte) {
  slipc_decoder_t self = {
      .startbyte = startbyte,
      .prev = SLIPC_END,
  };
  return self;
}

slipc_decoder_result_t slipc_decode_byte(slipc_decoder_t *self,
                                         slipc_writer_t *writer, uint8_t byte) {

  assert(self);
  assert(writer && writer->write);

  switch (byte) {
  case SLIPC_END:
    self->prev = byte;
    return SLIPC_DECODER_EOF;
  case SLIPC_ESC:
    self->prev = byte;
    return SLIPC_DECODER_MORE;
  }

  if (self->prev == SLIPC_ESC) {
    self->prev = byte;
    switch (byte) {
    case SLIPC_ESC_END:
      byte = SLIPC_END;
      break;
    case SLIPC_ESC_ESC:
      byte = SLIPC_ESC;
      break;
    }
  }

  if (writer->write(writer->user_ctx, &byte, 1) != SLIPC_WRITER_OK) {
    return SLIPC_DECODER_IO_ERROR;
  }

  return SLIPC_DECODER_MORE;
}

// TODO: implement results
slipc_decoder_result_t slipc_decoder_transfer(slipc_decoder_t *self,
                                              slipc_reader_t *reader,
                                              slipc_writer_t *writer) {
  assert(self);
  assert(reader && reader->read);
  assert(writer && writer->write);

  uint8_t byte;

  if (self->startbyte) {
    if (slipc_skip_to_start(reader) != SLIPC_DECODER_MORE) {
      return SLIPC_DECODER_NOT_FOUND;
    }
  }

  while (1) {
    size_t len = 1;

    slipc_reader_result_t read_res =
        reader->read(reader->user_ctx, &byte, &len);

    if (len != 1 || read_res == SLIPC_READER_ERROR) {
      return SLIPC_DECODER_IO_ERROR;
    }

    slipc_decoder_result_t decode_res = slipc_decode_byte(self, writer, byte);
    if (decode_res == SLIPC_DECODER_EOF) {
      return SLIPC_DECODER_EOF; // done
    }

    if (!(decode_res == SLIPC_DECODER_MORE && read_res == SLIPC_READER_MORE)) {
      return SLIPC_DECODER_MORE;
    }
  }
}

slipc_decoder_result_t slipc_decode_packet(slipc_writer_t *writer,
                                           uint8_t const *buf, size_t len,
                                           bool startbyte) {
  assert(writer && writer->write);
  assert(buf);

  if (startbyte) {
    for (size_t i = 0; i < len; i += 1) {
      if (buf[i] == SLIPC_END) {
        return slipc_decode_without_startbyte(writer, buf + len, len);
      }
    }
  }

  return SLIPC_DECODER_NOT_FOUND;
}

static slipc_writer_result_t slipc_write_end_byte(slipc_writer_t *writer) {
  uint8_t const start = SLIPC_END;
  return writer->write(writer->user_ctx, &start, 1);
}

static slipc_decoder_result_t slipc_skip_to_start(slipc_reader_t *reader) {
  uint8_t byte;
  while (1) {
    size_t len = 1;

    slipc_reader_result_t res = reader->read(reader->user_ctx, &byte, &len);

    if (len > 1 || res == SLIPC_READER_ERROR) {
      return SLIPC_DECODER_IO_ERROR;
    }

    if (len == 0) {
      return SLIPC_DECODER_NOT_FOUND;
    }

    if (byte == SLIPC_END) {
      return SLIPC_DECODER_MORE;
    }
  }
}

static slipc_decoder_result_t
slipc_decode_without_startbyte(slipc_writer_t *writer, uint8_t const *buf,
                               size_t len) {
  slipc_decoder_t decoder = slipc_decoder_new(false);

  for (size_t i = 0; i < len; i += 1) {
    slipc_decoder_result_t res = slipc_decode_byte(&decoder, writer, buf[i]);
    if (res != SLIPC_DECODER_MORE) {
      return res;
    }
  }

  return SLIPC_DECODER_IO_ERROR;
}
