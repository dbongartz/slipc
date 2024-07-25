/**
 * \file
 * \brief SLIPC I/O interface.
 *
 * This provides the I/O interface for SLIP-C.
 * It defines the reader and writer structures and the callback functions.
 *
 * It is not tied to SLIPC and can be used for other purposes.
 *
 * LICENSE: This library is released under the MIT License.
 *
 */
#ifndef __SLIPC_IO_H__
#define __SLIPC_IO_H__

#include <stddef.h>
#include <stdint.h>

/**
 * \brief User context structure.
 */
typedef struct slipc_io_user_ctx {
  void *ctx; /**< User-defined context */
} slipc_io_user_ctx_t;

/**
 * \brief Result codes for writer operations.
 */
typedef enum slipc_io_writer_result {
  SLIPC_IO_WRITER_OK,    /**< Operation successful */
  SLIPC_IO_WRITER_EOF,   /**< End of file */
  SLIPC_IO_WRITER_ERROR, /**< Error occurred */
} slipc_io_writer_result_t;

/**
 * \brief Callback function type for writing data.
 *
 * \note This expected to always set len to the number of bytes written.
 *
 * \param user_ctx User context
 * \param data Pointer to data to be written
 * \param len Pointer to the length of data to be written
 *
 * \retval SLIPC_WRITER_OK Operation successful
 * \retval SLIPC_WRITER_EOF Can't write more data
 * \retval SLIPC_WRITER_ERROR Error occurred
 */
typedef slipc_io_writer_result_t (*slipc_io_write_cb)(
    slipc_io_user_ctx_t user_ctx, const uint8_t *data, size_t *len);

/**
 * \brief Writer structure.
 */
typedef struct slipc_writer {
  struct slipc_io_user_ctx user_ctx; /**< User context */
  slipc_io_write_cb write;           /**< Write callback function */
} slipc_io_writer_t;

/**
 * \brief Writer for writing to an array.
 */
typedef struct slipc_io_array_writer {
  uint8_t *buf;
  size_t len;
} slipc_io_array_writer_t;

/**
 * \brief Create a writer for writing to an array.
 *
 * \param self Pointer to the array writer structure
 * \param buf Buffer needs to be alive as long as the array writer is used
 * \param len Length of the buffer
 *
 * \return Initialized writer structure
 */
slipc_io_writer_t slipc_io_array_writer_create(slipc_io_array_writer_t *self,
                                               uint8_t *buf, size_t len);

/**
 * \brief Result codes for reader operations.
 */
typedef enum slipc_io_reader_result {
  SLIPC_IO_READER_EOF,   /**< End of file */
  SLIPC_IO_READER_MORE,  /**< More data available */
  SLIPC_IO_READER_ERROR, /**< Error occurred */
} slipc_io_reader_result_t;

/**
 * \brief Callback function type for reading data.
 *
 * This expected to always set len to the number of bytes read.
 *
 * If no (more) data is available, the function should return SLIPC_READER_EOF
 * otherwise set SLIPC_READER_MORE.
 *
 * If an error occurs, the function should return SLIPC_READER_ERROR.
 *
 * \param user_ctx User context
 * \param data Pointer to buffer for reading data
 * \param len Pointer to length of data to be read
 * \retval SLIPC_READER_EOF No more data available
 * \retval SLIPC_READER_MORE More data available
 * \retval SLIPC_READER_ERROR Error occurred
 */
typedef slipc_io_reader_result_t (*slipc_io_read_cb)(
    slipc_io_user_ctx_t user_ctx, uint8_t *data, size_t *len);

/**
 * \struct slipc_reader
 * \brief Reader structure.
 */
typedef struct slipc_io_reader {
  struct slipc_io_user_ctx user_ctx; /**< User context */
  slipc_io_read_cb read;             /**< Read callback function */
} slipc_io_reader_t;

/**
 * \brief Reader for reading from an array.
 */
typedef struct slipc_io_array_reader {
  uint8_t const *buf;
  size_t len;
} slipc_io_array_reader_t;

/**
 * \brief Create a reader for reading from an array.
 *
 * \param self Pointer to the array reader structure
 * \param buf Buffer needs to be alive as long as the array reader is used
 * \param len Length of the buffer
 *
 * \return Initialized reader structure
 */
slipc_io_reader_t slipc_io_array_reader_create(slipc_io_array_reader_t *self,
                                               uint8_t const *buf, size_t len);

#endif // __SLIPC_IO_H__
