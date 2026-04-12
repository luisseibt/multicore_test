/*
 * Custom Virtio MMIO Transport Driver for VCML
 * SPDX-License-Identifier: Apache-2.0
 */
#define DT_DRV_COMPAT custom_virtio_mmio

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/logging/log.h>
#include <zephyr/irq.h>

extern void virtio_input_isr_handler(const struct device *dev);
LOG_MODULE_REGISTER(virtio_mmio, LOG_LEVEL_DBG);

/* Virtio MMIO Register Offsets (Matching VCML Model) */
#define VIRTIO_MMIO_MAGIC_VALUE         0x000
#define VIRTIO_MMIO_VERSION             0x004
#define VIRTIO_MMIO_DEVICE_ID           0x008
#define VIRTIO_MMIO_VENDOR_ID           0x00c
#define VIRTIO_MMIO_DEVICE_FEATURES     0x010
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL 0x014
#define VIRTIO_MMIO_DRIVER_FEATURES     0x020
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL 0x024
#define VIRTIO_MMIO_QUEUE_SEL           0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX       0x034
#define VIRTIO_MMIO_QUEUE_NUM           0x038
#define VIRTIO_MMIO_QUEUE_READY         0x044
#define VIRTIO_MMIO_QUEUE_NOTIFY        0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS    0x060
#define VIRTIO_MMIO_INTERRUPT_ACK       0x064
#define VIRTIO_MMIO_STATUS              0x070

/* Status Flags */
#define VIRTIO_STATUS_ACKNOWLEDGE       1
#define VIRTIO_STATUS_DRIVER            2
#define VIRTIO_STATUS_DRIVER_OK         4
#define VIRTIO_STATUS_FEATURES_OK       8
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 64
#define VIRTIO_STATUS_FAILED            128

struct virtio_mmio_config {
    mem_addr_t base_addr;
    void (*irq_config_func)(const struct device *dev);
};

struct virtio_mmio_data {
    uint32_t device_id;
    uint32_t vendor_id;
};

/* Interrupt Service Routine */
static void virtio_mmio_isr(const struct device *dev)
{
    const struct virtio_mmio_config *cfg = dev->config;
    uint32_t status = sys_read32(cfg->base_addr + VIRTIO_MMIO_INTERRUPT_STATUS);
    sys_write32(status, cfg->base_addr + VIRTIO_MMIO_INTERRUPT_ACK);

    if (status & 0x01) {
        /* Virtqueue Update! Find the child Input Device and wake it up. */
        const struct device *input_dev = DEVICE_DT_GET_ANY(custom_virtio_input);
        if (input_dev != NULL && device_is_ready(input_dev)) {
            virtio_input_isr_handler(input_dev);
        }
    }
    
    if (status & 0x02) {
        LOG_DBG("Configuration Change Interrupt Received!");
    }
}

static int virtio_mmio_init(const struct device *dev)
{
    const struct virtio_mmio_config *cfg = dev->config;
    struct virtio_mmio_data *data = dev->data;

    LOG_INF("Probing Virtio MMIO at 0x%lx", cfg->base_addr);

    /* 1. Check Magic Value */
    uint32_t magic = sys_read32(cfg->base_addr + VIRTIO_MMIO_MAGIC_VALUE);
    if (magic != 0x74726976) { /* "virt" */
        LOG_ERR("Invalid Magic Value: 0x%08x", magic);
        return -ENODEV;
    }

    /* 2. Check Version (VCML implements Version 2) */
    uint32_t version = sys_read32(cfg->base_addr + VIRTIO_MMIO_VERSION);
    if (version != 2) {
        LOG_ERR("Unsupported Virtio Version: %d (Expected 2)", version);
        return -ENODEV;
    }

    /* 3. Read Device ID */
    data->device_id = sys_read32(cfg->base_addr + VIRTIO_MMIO_DEVICE_ID);
    if (data->device_id == 0) {
        LOG_WRN("Dummy Virtio Device detected (ID 0). Skipping.");
        return -ENODEV;
    }

    data->vendor_id = sys_read32(cfg->base_addr + VIRTIO_MMIO_VENDOR_ID);
    LOG_INF("Found Virtio Device ID: %d, Vendor ID: 0x%x", data->device_id, data->vendor_id);

    /* 4. Reset Device */
    sys_write32(0, cfg->base_addr + VIRTIO_MMIO_STATUS);

    /* 5. Acknowledge Device and let it know a driver is present */
    uint32_t status = VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER;
    sys_write32(status, cfg->base_addr + VIRTIO_MMIO_STATUS);

    /* 6. Hook up the Interrupts */
    cfg->irq_config_func(dev);

    /* * NEXT STEPS:
     * 1. Read/Write Features (VIRTIO_MMIO_DEVICE_FEATURES)
     * 2. Set VIRTIO_STATUS_FEATURES_OK
     * 3. Allocate RAM for Virtqueues and write to VIRTIO_MMIO_QUEUE_DESC_LO/HI
     * 4. Set VIRTIO_STATUS_DRIVER_OK
     */

    LOG_INF("Virtio MMIO Transport Initialized Successfully.");
    return 0;
}

#define VIRTIO_MMIO_INIT(inst)                                                \
    static void virtio_mmio_irq_config_##inst(const struct device *dev)       \
    {                                                                         \
        IRQ_CONNECT(DT_INST_IRQN(inst), DT_INST_IRQ(inst, priority),          \
                    virtio_mmio_isr, DEVICE_DT_INST_GET(inst), 0);            \
        irq_enable(DT_INST_IRQN(inst));                                       \
    }                                                                         \
                                                                              \
    static const struct virtio_mmio_config virtio_cfg_##inst = {              \
        .base_addr = DT_INST_REG_ADDR(inst),                                  \
        .irq_config_func = virtio_mmio_irq_config_##inst,                     \
    };                                                                        \
                                                                              \
    static struct virtio_mmio_data virtio_data_##inst;                        \
                                                                              \
    DEVICE_DT_INST_DEFINE(inst, virtio_mmio_init, NULL,                       \
                          &virtio_data_##inst, &virtio_cfg_##inst,            \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,    \
                          NULL);

DT_INST_FOREACH_STATUS_OKAY(VIRTIO_MMIO_INIT)