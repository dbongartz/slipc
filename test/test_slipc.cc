#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <vector>

namespace dut {
#include "slipc.h"
}

std::tuple<bool, size_t, std::vector<uint8_t>>
bytecheck(std::tuple<bool, std::vector<uint8_t>> in) {
  auto v = std::get<1>(in);
  return {std::get<bool>(in), v.size(), v};
}

TEST_CASE("Encoding single bytes", "[encode]") {
  auto [byte, exp_res,
        exp_encoded] = GENERATE(table<uint8_t, bool, std::vector<uint8_t>>({
      {0, true, {0}},
      {1, true, {1}},
      {dut::slipc_char::SLIPC_END,
       true,
       {dut::slipc_char::SLIPC_ESC, dut::slipc_char::SLIPC_ESC_END}},
      {dut::slipc_char::SLIPC_ESC,
       true,
       {dut::slipc_char::SLIPC_ESC, dut::slipc_char::SLIPC_ESC_ESC}},
      {dut::slipc_char::SLIPC_ESC_END, true, {dut::slipc_char::SLIPC_ESC_END}},
      {dut::slipc_char::SLIPC_ESC_ESC, true, {dut::slipc_char::SLIPC_ESC_ESC}},
  }));

  auto writer_return = GENERATE(dut::slipc_writer_error::SLIPC_WRITER_OK,
                                dut::slipc_writer_error::SLIPC_WRITER_ERROR);

  if (writer_return != dut::slipc_writer_error::SLIPC_WRITER_OK) {
    exp_res = false;
    exp_encoded = std::vector<uint8_t>{};
  }

  INFO(std::format("byte: {}, writer_return: {}, expected result: {}", byte,
                   (int)writer_return, exp_res));

  struct writer_ctx {
    std::vector<uint8_t> buf;
    dut::slipc_writer_error err;
  };

  writer_ctx ctx{
      .buf = std::vector<uint8_t>{},
      .err = writer_return,
  };

  dut::slipc_writer writer{
      .ctx = &ctx,
      .write = [](void *ctx, uint8_t const *buf,
                  size_t len) -> dut::slipc_writer_error {
        auto _ctx = static_cast<writer_ctx *>(ctx);
        if (_ctx->err == dut::slipc_writer_error::SLIPC_WRITER_OK) {
          _ctx->buf.insert(_ctx->buf.end(), buf, buf + len);
        }
        return _ctx->err;
      },
  };
  auto res = dut::slipc_encode_byte(&writer, byte);

  REQUIRE(res == exp_res);
  REQUIRE(ctx.buf == exp_encoded);
}

TEST_CASE("Encoding failed", "[encode]") {
  std::vector<uint8_t> buf{};

  dut::slipc_writer writer{
      .ctx = &buf,
      .write = [](void *ctx, uint8_t const *buf,
                  size_t len) -> dut::slipc_writer_error {
        return dut::slipc_writer_error::SLIPC_WRITER_ERROR;
      },
  };
  auto res = dut::slipc_encode_byte(&writer, 42);

  REQUIRE(res == false);
  REQUIRE(buf.empty());
}

TEST_CASE("Good packet should encode", "[packet]") {
  struct writer_ctx {
    std::vector<uint8_t> buf;
    dut::slipc_writer_error err;
  };

  writer_ctx ctx{
      .buf = std::vector<uint8_t>{},
      .err = dut::slipc_writer_error::SLIPC_WRITER_OK,
  };

  dut::slipc_writer writer{
      .ctx = &ctx,
      .write = [](void *ctx, uint8_t const *buf,
                  size_t len) -> dut::slipc_writer_error {
        auto _ctx = static_cast<writer_ctx *>(ctx);
        if (_ctx->err == dut::slipc_writer_error::SLIPC_WRITER_OK) {
          _ctx->buf.insert(_ctx->buf.end(), buf, buf + len);
        }
        return _ctx->err;
      },
  };

  std::array<uint8_t, 12> input{
      1,
      2,
      3,
      dut::slipc_char::SLIPC_END,
      4,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_END,
      dut::slipc_char::SLIPC_ESC,
      5,
      dut::slipc_char::SLIPC_ESC_END,
      dut::slipc_char::SLIPC_ESC_ESC,
      6,
  };

  std::vector<uint8_t> expected{
      1,
      2,
      3,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_END,
      4,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_ESC,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_END,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_ESC,
      5,
      dut::slipc_char::SLIPC_ESC_END,
      dut::slipc_char::SLIPC_ESC_ESC,
      6,
      dut::slipc_char::SLIPC_END,
  };

  auto res =
      dut::slipc_encode_packet(&writer, input.data(), input.size(), false);

  REQUIRE(res == true);
  REQUIRE(ctx.buf == expected);
}

TEST_CASE("Transfer should encode", "[transfer]") {
  struct rw_ctx {
    std::vector<uint8_t> buf;
    int err;
  };

  rw_ctx wctx{
      .buf = std::vector<uint8_t>{},
      .err = dut::slipc_writer_error::SLIPC_WRITER_OK,
  };

  dut::slipc_writer writer{
      .ctx = &wctx,
      .write = [](void *ctx, uint8_t const *buf,
                  size_t len) -> dut::slipc_writer_error {
        auto _ctx = static_cast<rw_ctx *>(ctx);
        if (_ctx->err == dut::slipc_writer_error::SLIPC_WRITER_OK) {
          _ctx->buf.insert(_ctx->buf.end(), buf, buf + len);
        }
        return static_cast<dut::slipc_writer_error>(_ctx->err);
      },
  };

  std::vector<uint8_t> input{
      1,
      2,
      3,
      dut::slipc_char::SLIPC_END,
      4,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_END,
      dut::slipc_char::SLIPC_ESC,
      5,
      dut::slipc_char::SLIPC_ESC_END,
      dut::slipc_char::SLIPC_ESC_ESC,
      6,
  };

  rw_ctx rctx{
      .buf = input,
      .err = dut::slipc_reader_error::SLIPC_READER_EOF,
  };

  dut::slipc_reader reader{
      .ctx = &rctx,
      .read = [](void *ctx, uint8_t *buf,
                 size_t *len) -> dut::slipc_reader_error {
        auto _ctx = static_cast<rw_ctx *>(ctx);
        if (_ctx->buf.empty()) {
          *len = 0;
          return dut::slipc_reader_error::SLIPC_READER_EOF;
        }
        auto size = std::min(_ctx->buf.size(), *len);
        std::copy(_ctx->buf.begin(), _ctx->buf.begin() + size, buf);
        _ctx->buf.erase(_ctx->buf.begin(), _ctx->buf.begin() + size);
        *len = size;

        INFO(std::format("left: {}", _ctx->buf.size()));
        return dut::slipc_reader_error::SLIPC_READER_OK;
        // return dut::slipc_reader_error::SLIPC_READER_ERROR;
      },
  };

  dut::slipc_encoder slip{};
  dut::slipc_encoder_init(&slip, writer, reader, false);

  auto res = dut::slipc_encode_transfer(&slip);

  std::vector<uint8_t> expected{
      1,
      2,
      3,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_END,
      4,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_ESC,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_END,
      dut::slipc_char::SLIPC_ESC,
      dut::slipc_char::SLIPC_ESC_ESC,
      5,
      dut::slipc_char::SLIPC_ESC_END,
      dut::slipc_char::SLIPC_ESC_ESC,
      6,
      dut::slipc_char::SLIPC_END,
  };

  REQUIRE(res == true);
  REQUIRE(wctx.buf == expected);
}

TEST_CASE("Decode byte ok", "[decode]") {
  std::vector<uint8_t> buf{};

  dut::slipc_writer writer{
      .ctx = &buf,
      .write = [](void *ctx, uint8_t const *buf,
                  size_t len) -> dut::slipc_writer_error {
        auto dst = static_cast<std::vector<uint8_t> *>(ctx);
        dst->insert(dst->end(), buf, buf + len);
        return dut::slipc_writer_error::SLIPC_WRITER_OK;
      },
  };
  auto res = dut::slipc_encode_byte(&writer, 42);

  std::vector<uint8_t> exp{
      42,
  };

  REQUIRE(res == true);
  REQUIRE(buf == exp);
}
