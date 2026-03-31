/*
 * Spoofed QEMU RAMFB Driver -> Acts as VCML Simple FB
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT custom_display

#include <stdint.h>
#include <errno.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/device_mmio.h>
#include <string.h>

#define BYTES_PER_PIXEL 4

struct ramfb_config {
    uint16_t width;
    uint16_t height;
    uintptr_t fb_phys;
    size_t fb_size;
};
struct ramfb_data {
    uint8_t *fb_base;
    mm_reg_t fb_map;
};

static int ramfb_set_pixel_format(const struct device *dev, const enum display_pixel_format format)
{
    if (format == PIXEL_FORMAT_ARGB_8888) {
        return 0;
    }
    return -ENOTSUP;
}

static int ramfb_set_orientation(const struct device *dev,
                                 const enum display_orientation orientation)
{
    if (orientation == DISPLAY_ORIENTATION_NORMAL) {
        return 0;
    }
    return -ENOTSUP;
}

static void ramfb_get_capabilities(const struct device *dev, struct display_capabilities *caps)
{
    const struct ramfb_config *config = dev->config;

    caps->x_resolution = config->width;
    caps->y_resolution = config->height;
    caps->supported_pixel_formats = PIXEL_FORMAT_ARGB_8888;
    caps->screen_info = 0;
    caps->current_pixel_format = PIXEL_FORMAT_ARGB_8888;
    caps->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int ramfb_write(const struct device *dev, uint16_t x, uint16_t y,
                       const struct display_buffer_descriptor *desc, const void *buf)
{
    const struct ramfb_config *cfg = dev->config;
    struct ramfb_data *data = dev->data;
    
    if ((x + desc->width > cfg->width) || (y + desc->height > cfg->height)) {
        return -EINVAL;
    }

    uint32_t stride = cfg->width * BYTES_PER_PIXEL;
    uint32_t start_offset = (y * stride) + (x * BYTES_PER_PIXEL);
    
    uint8_t *dst = data->fb_base + start_offset;
    const uint8_t *src = (const uint8_t *)buf;

    for (uint32_t row = 0; row < desc->height; row++) {
        memcpy(dst, src, desc->width * BYTES_PER_PIXEL);
        dst += stride;
        src += desc->pitch * BYTES_PER_PIXEL;
    }

    return 0;
}

static int ramfb_read(const struct device *dev, uint16_t x, uint16_t y,
                      const struct display_buffer_descriptor *desc, void *buf)
{
    return -ENOTSUP; /* Not needed for a basic test */
}

static DEVICE_API(display, ramfb_api) = {
    .write = ramfb_write,
    .read = ramfb_read,
    .get_capabilities = ramfb_get_capabilities,
    .set_pixel_format = ramfb_set_pixel_format,
    .set_orientation = ramfb_set_orientation,
};

static int ramfb_init(const struct device *dev)
{
    const struct ramfb_config *cfg = dev->config;
    struct ramfb_data *data = dev->data;

    /* * THE HACK: We completely ignore fwcfg. We just map the physical 
     * memory region to a virtual pointer so the CPU can write to it.
     */
    device_map(&data->fb_map, cfg->fb_phys, cfg->fb_size, K_MEM_CACHE_NONE);
    data->fb_base = (uint8_t *)data->fb_map;

    return 0;
}

/* * We keep the exact macro structure the YAML expects, minus the fwcfg part.
 * The YAML expects memory-region to define the address.
 */
#define RAMFB_INIT(inst)                                                                       \
    static struct ramfb_data ramfb_data_##inst;                                                \
    static const struct ramfb_config ramfb_cfg_##inst = {                                      \
        .width = DT_INST_PROP(inst, width),                                                    \
        .height = DT_INST_PROP(inst, height),                                                  \
        .fb_phys = (uintptr_t)DT_REG_ADDR(DT_INST_PHANDLE(inst, memory_region)),               \
        .fb_size = DT_REG_SIZE(DT_INST_PHANDLE(inst, memory_region))};                         \
    DEVICE_DT_INST_DEFINE(inst, ramfb_init, NULL, &ramfb_data_##inst, &ramfb_cfg_##inst,       \
                          POST_KERNEL, CONFIG_DISPLAY_INIT_PRIORITY, &ramfb_api);

DT_INST_FOREACH_STATUS_OKAY(RAMFB_INIT)