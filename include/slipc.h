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
#ifndef _SLIPC_H_
#define _SLIPC_H_

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Special characters used in SLIP encoding.
 */
typedef enum slipc_char {
  SLIPC_END = 0xC0,     /**< Start / End of packet */
  SLIPC_ESC = 0xDB,     /**< Escape character */
  SLIPC_ESC_END = 0xDC, /**< Escaped END character */
  SLIPC_ESC_ESC = 0xDD, /**< Escaped ESC character */
} slipc_char_t;

/**
 * \brief User context structure.
 */
typedef struct slipc_user_ctx {
  void *ctx; /**< User-defined context */
} slipc_user_ctx_t;

/**
 * \brief Result codes for writer operations.
 */
typedef enum slipc_writer_result {
  SLIPC_WRITER_OK,    /**< Operation successful */
  SLIPC_WRITER_EOF,   /**< End of file */
  SLIPC_WRITER_ERROR, /**< Error occurred */
} slipc_writer_result_t;

/**
 * \brief Callback function type for writing data.
 *
 * \note This function is expected to always write the full length of data.
 *
 * \param user_ctx User context
 * \param data Pointer to data to be written
 * \param len Length of data to be written
 *
 * \retval SLIPC_WRITER_OK Operation successful
 * \retval SLIPC_WRITER_EOF Can't write more data
 * \retval SLIPC_WRITER_ERROR Error occurred
 */
typedef slipc_writer_result_t (*slipc_write_cb)(slipc_user_ctx_t user_ctx,
                                                const uint8_t *data,
                                                size_t len);

/**
 * \brief Writer structure.
 */
typedef struct slipc_writer {
  struct slipc_user_ctx user_ctx; /**< User context */
  slipc_write_cb write;           /**< Write callback function */
} slipc_writer_t;

/**
 * \brief Result codes for reader operations.
 */
typedef enum slipc_reader_result {
  SLIPC_READER_EOF,   /**< End of file */
  SLIPC_READER_MORE,  /**< More data available */
  SLIPC_READER_ERROR, /**< Error occurred */
} slipc_reader_result_t;

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
typedef slipc_reader_result_t (*slipc_read_cb)(slipc_user_ctx_t user_ctx,
                                               uint8_t *data, size_t *len);

/**
 * \struct slipc_reader
 * \brief Reader structure.
 */
typedef struct slipc_reader {
  struct slipc_user_ctx user_ctx; /**< User context */
  slipc_read_cb read;             /**< Read callback function */
} slipc_reader_t;

/**
 * \struct slipc_encoder
 * \brief Encoder structure.
 */
typedef struct slipc_encoder {
  bool startbyte; /**< Indicates if the start byte should be used */
} slipc_encoder_t;

/**
 * \enum slipc_encoder_result
 * \brief Result codes for encoder operations.
 */
typedef enum slipc_encoder_result {
  SLIPC_ENCODER_OK,       /**< Operation successful */
  SLIPC_ENCODER_IO_ERROR, /**< I/O error occurred */
} slipc_encoder_result_t;

/**
 * \brief Initialize an encoder.
 * \param self Pointer to the encoder structure
 * \param startbyte Indicates if the start byte should be used
 */
void slipc_encoder_init(slipc_encoder_t *self, bool startbyte);

/**
 * \brief Create a new encoder.
 * \param startbyte Indicates if the start byte should be used
 * \return Initialized encoder structure
 */
slipc_encoder_t slipc_encoder_new(bool startbyte);

/**
 * \brief Encode a single byte into a writer.
 * \param writer Pointer to the writer structure
 * \param byte Byte to be encoded
 *
 * \retval SLIPC_ENCODER_OK Operation successful
 * \retval SLIPC_ENCODER_IO_ERROR I/O error occurred
 */
slipc_encoder_result_t slipc_encode_byte(slipc_writer_t *writer, uint8_t byte);

/**
 * \brief Transfer data from reader to writer.
 *
 * This function encodes until reader returns SLIPC_READER_EOF or an error
 * occurs while reading or writing.
 *
 * \param self Pointer to the encoder structure
 * \param reader Pointer to the reader structure
 * \param writer Pointer to the writer structure
 *
 * \retval SLIPC_ENCODER_OK Operation successful
 * \retval SLIPC_ENCODER_IO_ERROR I/O error occurred
 */
slipc_encoder_result_t slipc_encoder_transfer(slipc_encoder_t *self,
                                              slipc_reader_t *reader,
                                              slipc_writer_t *writer);

/**
 * \brief Encode a packet of data into a writer.
 *
 * This function encodes the data buffer and sends it to the writer.
 *
 * \param writer Pointer to the writer structure
 * \param buf Pointer to the data buffer
 * \param len Length of the data buffer
 * \param startbyte Indicates if the start byte should be used
 *
 * \retval SLIPC_ENCODER_OK Operation successful
 * \retval SLIPC_ENCODER_IO_ERROR I/O error occurred
 */
slipc_encoder_result_t slipc_encode_packet(slipc_writer_t *writer,
                                           const uint8_t *buf, size_t len,
                                           bool startbyte);

/**
 * \brief Decoder structure.
 */
typedef struct slipc_decoder {
  bool startbyte; /**< Indicates if the start byte should be used */
  uint8_t prev;   /**< Previous byte processed */
} slipc_decoder_t;

/**
 * \brief Result codes for decoder operations.
 */
typedef enum slipc_decoder_result {
  SLIPC_DECODER_EOF,       /**< End of file */
  SLIPC_DECODER_MORE,      /**< More data available */
  SLIPC_DECODER_NOT_FOUND, /**< Data not found */
  SLIPC_DECODER_IO_ERROR,  /**< I/O error occurred */
} slipc_decoder_result_t;

/**
 * \brief Initialize a decoder.
 * \param self Pointer to the decoder structure
 * \param startbyte Indicates if we should expect a start byte
 */
void slipc_decoder_init(slipc_decoder_t *self, bool startbyte);

/**
 * \brief Create a new decoder.
 * \param startbyte Indicates if we should expect a start byte
 * \return Initialized decoder structure
 */
slipc_decoder_t slipc_decoder_new(bool startbyte);

/**
 * \brief Decode a single byte into a writer.
 *
 * \note This ignores the startbyte option.
 *
 * This function is stateful as it needs information about the previous byte to
 * handle ESC bytes correctly.
 *
 * \param self Pointer to the decoder structure
 * \param writer Pointer to the writer structure
 * \param byte Byte to be decoded
 *
 * \retval SLIPC_DECODER_EOF SLIP_END byte found
 * \retval SLIPC_DECODER_MORE SLIP_ESC byte found
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
slipc_decoder_result_t slipc_decode_byte(slipc_decoder_t *self,
                                         slipc_writer_t *writer, uint8_t byte);

/**
 * \brief Transfer data from reader to writer using the decoder.
 *
 * This function decodes until reader returns SLIPC_READER_EOF or an error
 * occurs while reading or writing.
 *
 * \param self Pointer to the decoder structure
 * \param reader Pointer to the reader structure
 * \param writer Pointer to the writer structure
 *
 * \retval SLIPC_DECODER_EOF Packet transfer complete
 * \retval SLIPC_DECODER_MORE Packet incomplete
 * \retval SLIPC_DECODER_NOT_FOUND No Start byte found (if startbyte is true)
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
slipc_decoder_result_t slipc_decoder_transfer(slipc_decoder_t *self,
                                              slipc_reader_t *reader,
                                              slipc_writer_t *writer);

/**
 * \brief Decode a packet of data into a writer.
 *
 * This function decodes the data buffer and sends it to the writer.
 *
 * \param writer Pointer to the writer structure
 * \param buf Pointer to the data buffer
 * \param len Length of the data buffer
 * \param startbyte Indicates if the start byte should be used
 *
 * \retval SLIPC_DECODER_EOF Packet transfer complete
 * \retval SLIPC_DECODER_MORE Packet incomplete
 * \retval SLIPC_DECODER_NOT_FOUND No Start byte found (if startbyte is true)
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
slipc_decoder_result_t slipc_decode_packet(slipc_writer_t *writer,
                                           const uint8_t *buf, size_t len,
                                           bool startbyte);

#ifdef __cplusplus
}
#endif
#endif /* _SLIPC_H_ */
