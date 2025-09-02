float distance_sq(float4 a, float4 b) {
    float x = a.x - b.x;
    float y = a.y - b.y;
    float z = a.z - b.z;
    float w = a.w - b.w;
    return x * x + y * y + z * z + w * w;
}

void kernel grayscale(read_only image2d_t src, write_only image2d_t dest, read_only image2d_t prev) {
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
                              CLK_ADDRESS_CLAMP |
                              CLK_FILTER_NEAREST;

    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    float4 pixel = read_imagef(src, sampler, coord);
    float4 pixel_prev = read_imagef(prev, sampler, coord);
    float d = distance_sq(pixel, pixel_prev);
    const float threshold = 0.15;

    // if motion
    if (d > threshold * threshold) {
        write_imagef(dest, coord, (float4)(0.0f, 0.0f, 1.0f, 1.0f));
    } else {
        write_imagef(dest, coord, pixel);
    }
}
