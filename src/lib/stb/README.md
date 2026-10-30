# stb image writer

`include/stb_image_write.h` is the upstream single-header image writer by
Sean Barrett and contributors, from <https://github.com/nothings/stb>.

BStone 3DS uses its PNG callback encoder to save screenshots. The BStone
allocator callbacks and `STBI_WRITE_NO_STDIO` are configured in
`bstone_stb_image_encoder.cpp`; SD file access stays in the engine's streams.

The upstream header is retained unchanged. See `LICENSE` for its MIT/public
domain license choices and the header for the original notices.
