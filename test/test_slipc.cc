#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

namespace dut {
#include "slipc.h"
}

struct InputDecode {
  std::vector<uint8_t> encoded;
  std::vector<uint8_t> decoded;
};

static const InputDecode GOOD_PACKET = {
    {
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        7,
        dut::slipc_char_t::SLIPC_END,
    },
    {
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        7,
    },
};

static const InputDecode GOOD_PACKET_WITH_START = {
    {
        dut::slipc_char_t::SLIPC_END,
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        7,
        dut::slipc_char_t::SLIPC_END,
    },
    GOOD_PACKET.decoded,
};

static const InputDecode NOISY_PACKET = {
    {
        // Noise
        44,
        dut::slipc_char_t::SLIPC_ESC,
        22,

        // Start of packet
        dut::slipc_char_t::SLIPC_END,
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        7,
        dut::slipc_char_t::SLIPC_END,
        // End of packet

        // Noise
        42,
        dut::slipc_char_t::SLIPC_ESC,
    },
    {
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        7,
    },
};

static const InputDecode NO_DATA_PACKET = {
    {},
    {},
};

static const InputDecode EMPTY_PACKET = {
    {
        dut::slipc_char_t::SLIPC_END,
    },
    {},
};

static const InputDecode EMPTY_PACKET_WITH_START = {
    {
        dut::slipc_char_t::SLIPC_END,
        dut::slipc_char_t::SLIPC_END,
    },
    EMPTY_PACKET.decoded,
};

static const InputDecode EMPTY_PACKET_WITH_NOISE = {
    {
        // Noise
        44,
        dut::slipc_char_t::SLIPC_ESC,
        22,

        // Start of packet
        dut::slipc_char_t::SLIPC_END,
        dut::slipc_char_t::SLIPC_END,
        // End of packet

        // Noise
        42,
        dut::slipc_char_t::SLIPC_ESC,
    },
    EMPTY_PACKET.decoded,
};

static const InputDecode MALFORMED_PACKET = {
    {
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        7,
        8,
        // Spurious ESC
        dut::slipc_char_t::SLIPC_ESC,
        9,
        // Repeated Spurios ESC
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC,
        10,
        // ESC with END
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_END,
    },
    {
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        7,
        8,
        // Malformed
        dut::slipc_char_t::SLIPC_ESC,
        9,
        // Malformed
        dut::slipc_char_t::SLIPC_ESC,
        // Malformed
        dut::slipc_char_t::SLIPC_ESC,
        10,
        // Malformed
        dut::slipc_char_t::SLIPC_ESC,
    },
};

static const InputDecode MALFORMED_PACKET_WITH_START = {
    {
        dut::slipc_char_t::SLIPC_END,
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        7,
        8,
        // Spurious ESC
        dut::slipc_char_t::SLIPC_ESC,
        9,
        // Repeated Spurios ESC
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC,
        10,
        // ESC with END
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_END,
    },
    MALFORMED_PACKET.decoded,
};

static const InputDecode MALFORMED_NOISY_PACKET = {
    {
        // Noise
        44,
        dut::slipc_char_t::SLIPC_ESC,
        22,

        // Start of packet
        dut::slipc_char_t::SLIPC_END,
        1,
        2,
        dut::slipc_char_t::SLIPC_ESC_END,
        4,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        5,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_END,
        6,
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC_ESC,
        7,
        8,
        // Spurious ESC
        dut::slipc_char_t::SLIPC_ESC,
        9,
        // Repeated Spurios ESC
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_ESC,
        10,
        // ESC with END
        dut::slipc_char_t::SLIPC_ESC,
        dut::slipc_char_t::SLIPC_END,
        // End of packet

        // Noise
        42,
        dut::slipc_char_t::SLIPC_ESC,
    },
    MALFORMED_PACKET.decoded};

struct VecWriter : dut::slipc_writer_t {
  std::vector<uint8_t> buf;
  dut::slipc_writer_result_t result;

  VecWriter(dut::slipc_writer_result_t result =
                dut::slipc_writer_result_t::SLIPC_WRITER_OK)
      : result(result), dut::slipc_writer_t{this, writer_cb} {}

  static dut::slipc_writer_result_t writer_cb(dut::slipc_user_ctx_t uctx,
                                              uint8_t const *buf, size_t *len) {
    auto &ctx = *static_cast<VecWriter *>(uctx.ctx);
    std::span src(buf, *len);

    if (ctx.result == dut::slipc_writer_result_t::SLIPC_WRITER_OK) {
      ctx.buf.insert(ctx.buf.end(), src.begin(), src.end());
      *len = src.size_bytes();
    } else {
      *len = 0;
    }

    return ctx.result;
  }
};

struct VecReader : dut::slipc_reader_t {
  std::vector<uint8_t> buf;
  dut::slipc_reader_result_t result;

  VecReader(std::vector<uint8_t> buf,
            dut::slipc_reader_result_t result =
                dut::slipc_reader_result_t::SLIPC_READER_MORE)
      : buf(buf), result(result), dut::slipc_reader_t{this, reader_cb} {}

  static dut::slipc_reader_result_t reader_cb(dut::slipc_user_ctx_t uctx,
                                              uint8_t *buf, size_t *len) {
    auto &ctx = *static_cast<VecReader *>(uctx.ctx);
    std::span dst(buf, *len);
    if (ctx.result == dut::slipc_reader_result_t::SLIPC_READER_MORE) {
      *len = std::min(ctx.buf.size(), dst.size_bytes());
      std::copy_n(ctx.buf.begin(), *len, buf);
      ctx.buf.erase(ctx.buf.begin(), ctx.buf.begin() + *len);

      if (ctx.buf.empty()) {
        return dut::slipc_reader_result_t::SLIPC_READER_EOF;
      }
    }

    return ctx.result;
  }
};

TEST_CASE("Encoding single bytes", "[encode]") {
  auto [byte, exp_encoded] = GENERATE(table<uint8_t, std::vector<uint8_t>>({
      {0, {0}},
      {1, {1}},
      {
          dut::slipc_char_t::SLIPC_END,
          {dut::slipc_char_t::SLIPC_ESC, dut::slipc_char_t::SLIPC_ESC_END},
      },
      {
          dut::slipc_char_t::SLIPC_ESC,
          {dut::slipc_char_t::SLIPC_ESC, dut::slipc_char_t::SLIPC_ESC_ESC},
      },
      {
          dut::slipc_char_t::SLIPC_ESC_END,
          {dut::slipc_char_t::SLIPC_ESC_END},
      },
      {
          dut::slipc_char_t::SLIPC_ESC_ESC,
          {dut::slipc_char_t::SLIPC_ESC_ESC},
      },
  }));

  INFO("Encoding byte " << byte);

  SECTION("Should succeed") {
    VecWriter writer;
    auto res = dut::slipc_encode_byte(&writer, byte);
    REQUIRE(res == dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
    REQUIRE(writer.buf == exp_encoded);
  }

  SECTION("Should fail") {
    VecWriter writer(dut::slipc_writer_result_t::SLIPC_WRITER_ERROR);
    auto res = dut::slipc_encode_byte(&writer, byte);
    REQUIRE(res != dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
    REQUIRE(writer.buf.empty());
  }
}

TEST_CASE("Packet encode", "[encode]") {

  SECTION("Good Packet") {
    auto const packet = GOOD_PACKET;

    SECTION("Writer ok") {
      VecWriter writer(dut::slipc_writer_result_t::SLIPC_WRITER_OK);
      auto res = dut::slipc_encode_packet(&writer, packet.decoded.data(),
                                          packet.decoded.size(), false);
      REQUIRE(res == dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
      REQUIRE(writer.buf == packet.encoded);
    }

    SECTION("Writer error") {
      VecWriter writer(dut::slipc_writer_result_t::SLIPC_WRITER_ERROR);
      auto res = dut::slipc_encode_packet(&writer, packet.decoded.data(),
                                          packet.decoded.size(), false);
      REQUIRE(res != dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
      REQUIRE(writer.buf.empty());
    }
  }

  SECTION("Good Packet With Start") {
    auto const packet = GOOD_PACKET_WITH_START;

    SECTION("Writer ok") {
      VecWriter writer(dut::slipc_writer_result_t::SLIPC_WRITER_OK);
      auto res = dut::slipc_encode_packet(&writer, packet.decoded.data(),
                                          packet.decoded.size(), true);
      REQUIRE(res == dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
      REQUIRE(writer.buf == packet.encoded);
    }
  }
}

TEST_CASE("Transfer encode", "[transfer]") {
  VecWriter writer(dut::slipc_writer_result_t::SLIPC_WRITER_OK);

  SECTION("Good Packet") {

    SECTION("Without startbyte") {
      auto const packet = GOOD_PACKET;
      VecReader reader(packet.decoded);

      auto slip = dut::slipc_encoder_new(false);
      auto res = dut::slipc_encoder_transfer(&slip, &reader, &writer);

      REQUIRE(res == dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
      REQUIRE(writer.buf == packet.encoded);
    }

    SECTION("With startbyte") {
      auto const packet = GOOD_PACKET_WITH_START;

      VecReader reader(packet.decoded);

      auto slip = dut::slipc_encoder_new(true);
      auto res = dut::slipc_encoder_transfer(&slip, &reader, &writer);

      REQUIRE(res == dut::slipc_encoder_result_t::SLIPC_ENCODER_OK);
      REQUIRE(writer.buf == packet.encoded);
    }
  }
}

TEST_CASE("Transfer decode", "[decode]") {
  VecWriter writer(dut::slipc_writer_result_t::SLIPC_WRITER_OK);

  SECTION("Good Packet") {

    SECTION("Without startbyte") {
      auto const packet = GOOD_PACKET;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(false);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("With startbyte") {
      auto const packet = GOOD_PACKET_WITH_START;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }
  }

  SECTION("Noisy Packet") {
    auto const packet = NOISY_PACKET;
    VecReader reader(packet.encoded);

    auto decoder = dut::slipc_decoder_new(true);
    auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

    REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
    REQUIRE(writer.buf == packet.decoded);
  }

  SECTION("No Data Packet") {
    auto const packet = NO_DATA_PACKET;
    VecReader reader(packet.encoded);

    SECTION("Without startbyte") {
      auto decoder = dut::slipc_decoder_new(false);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_NOT_FOUND);
      REQUIRE(writer.buf.empty());
    }

    SECTION("With startbyte") {
      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_NOT_FOUND);
      REQUIRE(writer.buf == packet.decoded);
    }
  }

  SECTION("Empty Packet") {
    SECTION("Without startbyte") {
      auto const packet = EMPTY_PACKET;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(false);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf.empty());
    }

    SECTION("With startbyte") {
      auto const packet = EMPTY_PACKET_WITH_START;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("With noise") {
      auto const packet = EMPTY_PACKET_WITH_NOISE;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf.empty());
    }
  }

  SECTION("Malformed Packet") {
    SECTION("Without startbyte") {
      auto const packet = MALFORMED_PACKET;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(false);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("With startbyte") {
      auto const packet = MALFORMED_PACKET_WITH_START;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("Noisy Packet") {
      auto const packet = MALFORMED_NOISY_PACKET;
      VecReader reader(packet.encoded);

      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_transfer(&decoder, &reader, &writer);

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }
  }
}

TEST_CASE("Packet decode", "[decode]") {
  SECTION("Good Packet") {

    SECTION("Without startbyte") {
      auto const packet = GOOD_PACKET;
      auto writer = VecWriter();
      auto decoder = dut::slipc_decoder_new(false);
      auto res = dut::slipc_decoder_decode_packet(
          &decoder, &writer, packet.encoded.data(), packet.encoded.size());

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("With startbyte") {
      auto const packet = GOOD_PACKET_WITH_START;
      auto writer = VecWriter();
      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_decode_packet(
          &decoder, &writer, packet.encoded.data(), packet.encoded.size());

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }
  }

  SECTION("Noisy Packet") {
    auto const packet = NOISY_PACKET;
    auto writer = VecWriter();
    auto decoder = dut::slipc_decoder_new(true);
    auto res = dut::slipc_decoder_decode_packet(
        &decoder, &writer, packet.encoded.data(), packet.encoded.size());

    REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
    REQUIRE(writer.buf == packet.decoded);
  }

  SECTION("Malformed Packet") {

    SECTION("Without startbyte") {
      auto const packet = MALFORMED_PACKET;
      auto writer = VecWriter();
      auto decoder = dut::slipc_decoder_new(false);
      auto res = dut::slipc_decoder_decode_packet(
          &decoder, &writer, packet.encoded.data(), packet.encoded.size());

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("With startbyte") {
      auto const packet = MALFORMED_PACKET_WITH_START;
      auto writer = VecWriter();
      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_decode_packet(
          &decoder, &writer, packet.encoded.data(), packet.encoded.size());

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }

    SECTION("Noisy Packet") {
      auto const packet = MALFORMED_NOISY_PACKET;
      auto writer = VecWriter();
      auto decoder = dut::slipc_decoder_new(true);
      auto res = dut::slipc_decoder_decode_packet(
          &decoder, &writer, packet.encoded.data(), packet.encoded.size());

      REQUIRE(res == dut::slipc_decoder_result_t::SLIPC_DECODER_EOF);
      REQUIRE(writer.buf == packet.decoded);
    }
  }
}
