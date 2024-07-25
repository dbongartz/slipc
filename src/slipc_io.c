#include "slipc_io.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * \brief Read callback for array reader.
 */
static slipc_io_reader_result_t
slipc_array_reader_read(slipc_io_user_ctx_t user_ctx, uint8_t *buf, size_t *len) {
  slipc_io_array_reader_t *ctx = user_ctx.ctx;

  *len = ctx->len < *len ? ctx->len : *len;
  memcpy(buf, ctx->buf, *len);

  ctx->len -= *len;
  ctx->buf += *len;

  if (ctx->len == 0) {
    return SLIPC_IO_READER_EOF;
  }

  return SLIPC_IO_READER_MORE;
}

slipc_io_reader_t slipc_io_array_reader_create(slipc_io_array_reader_t *self,
                                         uint8_t const *buf, size_t len) {
  assert(self);
  assert(buf);
  self->buf = buf;
  self->len = len;

  slipc_io_reader_t reader = {
      .user_ctx = {self},
      .read = slipc_array_reader_read,
  };
  return reader;
}

/**
 * \brief Write callback for array writer.
 */
static slipc_io_writer_result_t slipc_array_writer_write(slipc_io_user_ctx_t user_ctx,
                                                      uint8_t const *buf,
                                                      size_t *len) {
  slipc_io_array_writer_t *ctx = user_ctx.ctx;

  *len = ctx->len < *len ? ctx->len : *len;
  memcpy(ctx->buf, buf, *len);

  ctx->len -= *len;
  ctx->buf += *len;

  if (ctx->len == 0) {
    return SLIPC_IO_WRITER_EOF;
  }

  return SLIPC_IO_WRITER_OK;
}

slipc_io_writer_t slipc_io_array_writer_create(slipc_io_array_writer_t *self,
                                         uint8_t *buf, size_t len) {
  assert(self);
  assert(buf);
  self->buf = buf;
  self->len = len;

  slipc_io_writer_t writer = {

      .user_ctx = {self},
      .write = slipc_array_writer_write,
  };
  return writer;
}
