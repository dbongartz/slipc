/**
 * \file slipc.h
 * \brief SLIPC (Serial Line Internet Protocol in C) encoder and decoder.
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

#include "slipc_io.h"

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
 * This is identical to transfer, just from a buffer.
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
  bool malformed; /**< Malformed packet */
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
 * \brief Check if the current packet is malformed.
 */
bool slipc_decoder_is_malformed(slipc_decoder_t *self);

/**
 * \brief Decode a single byte into a writer.
 *
 * \note This ignores the startbyte option.
 *
 * This function is stateful as it needs information about the previous byte to
 * handle ESC bytes correctly.
 *
 * A malformed byte will just be passed through.
 * (E.g. ESC not followed bye ESC_END or ESC_ESC)
 *
 * The state can be checked with slipc_decoder_is_malformed().
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
 * This is identical to transfer, just from a buffer.
 *
 * \param self Pointer to the decoder structure
 * \param writer Pointer to the writer structure
 * \param buf Pointer to the data buffer
 * \param len Length of the data buffer
 *
 * \retval SLIPC_DECODER_EOF Packet transfer complete
 * \retval SLIPC_DECODER_MORE Packet incomplete
 * \retval SLIPC_DECODER_NOT_FOUND No Start byte found (if startbyte is true)
 * \retval SLIPC_DECODER_IO_ERROR I/O error occurred
 */
slipc_decoder_result_t slipc_decoder_decode_packet(slipc_decoder_t *self,
                                                   slipc_writer_t *writer,
                                                   const uint8_t *buf,
                                                   size_t len);

#ifdef __cplusplus
}
#endif
#endif /* _SLIPC_H_ */
