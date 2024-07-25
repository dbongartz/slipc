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
#include "slipc_io.h"
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
static slipc_io_writer_result_t slipc_write_end_byte(slipc_io_writer_t *writer);

/**
 * \brief Skip to the start byte in the reader.
 *
 * \param reader Reader structure
 *
 * \retval SLIPC_DECODER_MORE Start byte found
 * \retval SLIPC_DECODER_NOT_FOUND Start byte not found
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
static slipc_decoder_result_t slipc_skip_to_start(slipc_io_reader_t *reader);

slipc_encoder_result_t slipc_encode_byte(slipc_io_writer_t *writer,
                                         uint8_t byte) {
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
    size_t len = 1;
    if (writer->write(writer->user_ctx, &byte, &len) != SLIPC_IO_WRITER_OK ||
        len != 1) {
      return SLIPC_ENCODER_IO_ERROR;
    }
    byte = stuffbyte;
  }

  size_t len = 1;
  if (writer->write(writer->user_ctx, &byte, &len) != SLIPC_IO_WRITER_OK ||
      len != 1) {
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
                                              slipc_io_reader_t *reader,
                                              slipc_io_writer_t *writer) {
  assert(self);
  assert(writer && writer->write);
  assert(reader && reader->read);

  uint8_t byte;

  if (self->startbyte) {
    if (slipc_write_end_byte(writer) != SLIPC_IO_WRITER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }
  }

  while (1) {
    size_t len = 1;
    slipc_io_reader_result_t res = reader->read(reader->user_ctx, &byte, &len);
    if (len != 1 || res == SLIPC_IO_READER_ERROR) {
      return SLIPC_ENCODER_IO_ERROR;
    }

    if (slipc_encode_byte(writer, byte) != SLIPC_ENCODER_OK) {
      return SLIPC_ENCODER_IO_ERROR;
    }

    if (res == SLIPC_IO_READER_EOF) {
      if (slipc_write_end_byte(writer) != SLIPC_IO_WRITER_OK) {
        return SLIPC_ENCODER_IO_ERROR;
      }
      return SLIPC_ENCODER_OK;
    }
  }
}

slipc_encoder_result_t slipc_encode_packet(slipc_io_writer_t *writer,
                                           uint8_t const *buf, size_t len,
                                           bool startbyte) {
  assert(writer && writer->write);
  assert(buf);

  slipc_io_array_reader_t reader_ctx;
  slipc_io_reader_t reader =
      slipc_io_array_reader_create(&reader_ctx, buf, len);

  slipc_encoder_t encoder = slipc_encoder_new(startbyte);
  return slipc_encoder_transfer(&encoder, &reader, writer);
}

void slipc_decoder_init(slipc_decoder_t *self, bool startbyte) {
  assert(self);
  *self = slipc_decoder_new(startbyte);
}

slipc_decoder_t slipc_decoder_new(bool startbyte) {
  slipc_decoder_t self = {
      .startbyte = startbyte,
      .prev = SLIPC_END,
      .malformed = false,
  };
  return self;
}

bool slipc_decoder_is_malformed(slipc_decoder_t *self) {
  assert(self);
  return self->malformed;
}

static slipc_io_writer_result_t slipc_write_byte(slipc_io_writer_t *writer,
                                                 uint8_t byte) {
  size_t len = 1;
  return writer->write(writer->user_ctx, &byte, &len);
}

slipc_decoder_result_t slipc_decode_byte(slipc_decoder_t *self,
                                         slipc_io_writer_t *writer,
                                         uint8_t byte) {
  assert(self);
  assert(writer && writer->write);

  uint8_t prev = self->prev;
  self->prev = byte;

  uint8_t byte_to_write = byte;

  if (byte == SLIPC_END) {
    return SLIPC_DECODER_EOF;
  }

  if (prev == SLIPC_ESC) {
    switch (byte) {
    case SLIPC_ESC_END:
      byte_to_write = SLIPC_END;
      break;
    case SLIPC_ESC_ESC:
      byte_to_write = SLIPC_ESC;
      break;
    default:
      // Malformed packet, but let's just keep those bytes in the output.
      self->malformed = true;
      if (slipc_write_byte(writer, byte) != SLIPC_IO_WRITER_OK) {
        return SLIPC_DECODER_IO_ERROR;
      }
      return SLIPC_DECODER_MORE;
    }
  }

  if (byte == SLIPC_ESC) {
    return SLIPC_DECODER_MORE;
  }

  if (slipc_write_byte(writer, byte_to_write) != SLIPC_IO_WRITER_OK) {
    return SLIPC_DECODER_IO_ERROR;
  }
  return SLIPC_DECODER_MORE;
}

// TODO: implement results
slipc_decoder_result_t slipc_decoder_transfer(slipc_decoder_t *self,
                                              slipc_io_reader_t *reader,
                                              slipc_io_writer_t *writer) {
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

    slipc_io_reader_result_t read_res =
        reader->read(reader->user_ctx, &byte, &len);

    if (len == 0 && read_res == SLIPC_IO_READER_EOF) {
      return SLIPC_DECODER_NOT_FOUND;
    }

    if (len != 1 || read_res == SLIPC_IO_READER_ERROR) {
      return SLIPC_DECODER_IO_ERROR;
    }

    slipc_decoder_result_t decode_res = slipc_decode_byte(self, writer, byte);
    if (decode_res == SLIPC_DECODER_EOF) {
      return SLIPC_DECODER_EOF;
    }

    if (!(decode_res == SLIPC_DECODER_MORE &&
          read_res == SLIPC_IO_READER_MORE)) {
      return SLIPC_DECODER_MORE;
    }
  }
}

slipc_decoder_result_t slipc_decoder_decode_packet(slipc_decoder_t *self,
                                                   slipc_io_writer_t *writer,
                                                   uint8_t const *buf,
                                                   size_t len) {
  assert(self);
  assert(writer && writer->write);
  assert(buf);

  slipc_io_array_reader_t reader_ctx;
  slipc_io_reader_t reader =
      slipc_io_array_reader_create(&reader_ctx, buf, len);
  return slipc_decoder_transfer(self, &reader, writer);
}

static slipc_io_writer_result_t
slipc_write_end_byte(slipc_io_writer_t *writer) {
  uint8_t const start = SLIPC_END;
  size_t len = 1;
  return writer->write(writer->user_ctx, &start, &len);
}

static slipc_decoder_result_t slipc_skip_to_start(slipc_io_reader_t *reader) {
  uint8_t byte;
  while (1) {
    size_t len = 1;

    slipc_io_reader_result_t res = reader->read(reader->user_ctx, &byte, &len);

    if (len > 1 || res == SLIPC_IO_READER_ERROR) {
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
