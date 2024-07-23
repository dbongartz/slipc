#ifndef _SLIPC_H_
#define _SLIPC_H_

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SLIPC_MAX_PACKET_SIZE 256

enum slipc_char {
  SLIPC_END = 0xC0,
  SLIPC_ESC = 0xDB,
  SLIPC_ESC_END = 0xDC,
  SLIPC_ESC_ESC = 0xDD,
};

enum slipc_writer_error {
  SLIPC_WRITER_OK,
  SLIPC_WRITER_ERROR,
};

typedef enum slipc_writer_error (*slipc_write_cb)(void *ctx,
                                                  const uint8_t *data,
                                                  size_t len);

struct slipc_writer {
  void *ctx;
  slipc_write_cb write;
};

enum slipc_reader_error {
  SLIPC_READER_EOF,
  SLIPC_READER_OK,
  SLIPC_READER_ERROR,
};

typedef enum slipc_reader_error (*slipc_read_cb)(void *ctx, uint8_t *data,
                                                 size_t *len);

struct slipc_reader {
  void *ctx;
  slipc_read_cb read;
};

struct slipc_encoder {
  struct slipc_writer writer;
  struct slipc_reader reader;

  bool send_start;
};

void slipc_encoder_init(struct slipc_encoder *self, struct slipc_writer writer,
                        struct slipc_reader reader, bool send_start);

bool slipc_encode_byte(struct slipc_writer *writer, uint8_t byte);

bool slipc_encode_transfer(struct slipc_encoder *self);

bool slipc_encode_packet(struct slipc_writer *writer, uint8_t const *buf,
                         size_t len, bool send_start);

struct slipc_decoder {
  uint8_t prev;
};

enum slipc_decode_result {
  SLIPC_DECODE_Ok,
  SLIPC_DECODE_EOF,
  SLIPC_DECODE_ERROR,
};

void slip_decoder_init(struct slipc_decoder *self);

enum slipc_decode_result slipc_decode_byte(struct slipc_decoder *decoder,
                                           struct slipc_writer *writer,
                                           uint8_t byte);

#ifdef __cplusplus
}
#endif
#endif
