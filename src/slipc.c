#include "slipc.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void slipc_encoder_init(struct slipc_encoder *self, struct slipc_writer writer,
                        struct slipc_reader reader, bool send_start) {
  assert(self);
  assert(writer.write);
  assert(reader.read);

  self->writer = writer;
  self->reader = reader;
  self->send_start = send_start;
}

bool slipc_encode_byte(struct slipc_writer *writer, uint8_t byte) {
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

  if (stuffbyte) {
    if (writer->write(writer->ctx, &byte, 1) != SLIPC_WRITER_OK) {
      return false;
    }
    byte = stuffbyte;
  }

  if (writer->write(writer->ctx, &byte, 1) != SLIPC_WRITER_OK) {
    return false;
  }

  return true;
}

bool slipc_encode_transfer(struct slipc_encoder *self) {
  assert(self);

  uint8_t buf[SLIPC_MAX_PACKET_SIZE];

  if (self->send_start) {
    if (slipc_encode_byte(&self->writer, SLIPC_END) == false) {
      return false;
    }
  }

  while (1) {
    size_t len = sizeof(buf);
    switch (self->reader.read(self->reader.ctx, buf, &len)) {
    case SLIPC_READER_OK:
      for (size_t i = 0; i < len; i += 1) {
        if (slipc_encode_byte(&self->writer, buf[i]) == false) {
          return false;
        }
      }
      continue;
    case SLIPC_READER_EOF: {
      uint8_t end = SLIPC_END;
      if (self->writer.write(self->writer.ctx, &end, 1) != SLIPC_WRITER_OK) {
        return false;
      }
      return true;
    }
    case SLIPC_READER_ERROR:
      return false;
    }
  }
}

bool slipc_encode_packet(struct slipc_writer *writer, uint8_t const *buf,
                         size_t len, bool send_start) {
  assert(writer);
  assert(buf);

  if (send_start) {
    if (slipc_encode_byte(writer, SLIPC_END) == false) {
      return false;
    }
  }

  for (size_t i = 0; i < len; i += 1) {
    if (slipc_encode_byte(writer, buf[i]) == false) {
      return false;
    }
  }
  uint8_t end = SLIPC_END;
  if (writer->write(writer->ctx, &end, 1) != SLIPC_WRITER_OK) {
    return false;
  }
  return true;
}

void slip_decoder_init(struct slipc_decoder *self) { self->prev = SLIPC_END; }

enum slipc_decode_result slipc_decode_byte(struct slipc_decoder *decoder,
                                           struct slipc_writer *writer,
                                           uint8_t byte) {
  switch (byte) {
  case SLIPC_END:
    decoder->prev = byte;
    return SLIPC_DECODE_EOF;
  case SLIPC_ESC:
    decoder->prev = byte;
    return SLIPC_DECODE_Ok;
  }

  if (decoder->prev == SLIPC_ESC) {
    decoder->prev = byte;
    switch (byte) {
    case SLIPC_ESC_END:
      byte = SLIPC_END;
      break;
    case SLIPC_ESC_ESC:
      byte = SLIPC_ESC;
      break;
    }
  }

  if (writer->write(writer->ctx, &byte, 1) != SLIPC_WRITER_OK) {
    return SLIPC_DECODE_ERROR;
  }

  return SLIPC_DECODE_Ok;
}
