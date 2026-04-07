/*
 * Custom Virtio Input Driver - Vring Allocation
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT custom_virtio_input

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(virtio_input, LOG_LEVEL_INF);

/* --- MMIO Register Offsets (Virtio v2) --- */
#define VIRTIO_MMIO_QUEUE_SEL       0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX   0x034
#define VIRTIO_MMIO_QUEUE_NUM       0x038
#define VIRTIO_MMIO_QUEUE_READY     0x044
#define VIRTIO_MMIO_QUEUE_NOTIFY    0x050
#define VIRTIO_MMIO_STATUS          0x070
#define VIRTIO_MMIO_QUEUE_DESC_LOW  0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH 0x084
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW 0x090
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH 0x094
#define VIRTIO_MMIO_QUEUE_USED_LOW  0x0a0
#define VIRTIO_MMIO_QUEUE_USED_HIGH 0x0a4

#define QUEUE_SIZE 8
#define VRING_DESC_F_WRITE 2 /* Flag telling the device it can write to this buffer */

/* --- Virtio Data Structures --- */
struct virtio_input_event {
    uint16_t type;
    uint16_t code;
    uint32_t value;
};

/* 1. Descriptor Table */
struct vq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

/* 2. Available Ring (Driver -> Device) */
struct vq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[QUEUE_SIZE];
    uint16_t used_event;
};

/* 3. Used Ring (Device -> Driver) */
struct vq_used_elem {
    uint32_t id;
    uint32_t len;
};

struct vq_used {
    uint16_t flags;
    uint16_t idx;
    struct vq_used_elem ring[QUEUE_SIZE];
    uint16_t avail_event;
};

/* The Driver Data containing the actual RAM buffers */
struct virtio_input_data {
    mem_addr_t mmio_base;

    /* ALIGNED Vring Structures in RAM */
    struct vq_desc desc[QUEUE_SIZE] __aligned(16);
    struct vq_avail avail __aligned(2);
    struct vq_used used __aligned(4);
    
    /* The actual buffers where VCML writes keystrokes */
    struct virtio_input_event events[QUEUE_SIZE];
};

static int virtio_input_init(const struct device *dev)
{
    struct virtio_input_data *data = dev->data;
    /* Force the base address to be treated as pure math, not a C pointer */
    uintptr_t base = (uintptr_t)data->mmio_base;

    LOG_INF("Virtio Input initializing Vrings at MMIO: 0x%lx", base);

    /* --- STEP 1: Select Queue 0 (Event Queue) --- */
    sys_write32(0, base + VIRTIO_MMIO_QUEUE_SEL);

    /* Verify VCML supports at least 8 items in the queue */
    uint32_t q_max = sys_read32(base + VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (q_max < QUEUE_SIZE) {
        LOG_ERR("Queue max size too small! %d", q_max);
        return -ENODEV;
    }

    /* Set our requested Queue Size */
    sys_write32(QUEUE_SIZE, base + VIRTIO_MMIO_QUEUE_NUM);

    /* --- STEP 2: Configure the Memory --- */
    /* Link each descriptor to its corresponding event buffer */
    for (int i = 0; i < QUEUE_SIZE; i++) {
        data->desc[i].addr = (uint32_t)(uintptr_t)&data->events[i]; 
        data->desc[i].len = sizeof(struct virtio_input_event);
        data->desc[i].flags = VRING_DESC_F_WRITE;
        data->desc[i].next = 0;
        
        data->avail.ring[i] = i; 
    }
    
    data->avail.idx = QUEUE_SIZE;
    data->used.idx = 0;

    /* --- STEP 3: Pass Addresses to VCML --- */
    sys_write32((uint32_t)(uintptr_t)&data->desc, base + VIRTIO_MMIO_QUEUE_DESC_LOW);
    sys_write32(0, base + VIRTIO_MMIO_QUEUE_DESC_HIGH);

    sys_write32((uint32_t)(uintptr_t)&data->avail, base + VIRTIO_MMIO_QUEUE_AVAIL_LOW);
    sys_write32(0, base + VIRTIO_MMIO_QUEUE_AVAIL_HIGH);

    sys_write32((uint32_t)(uintptr_t)&data->used, base + VIRTIO_MMIO_QUEUE_USED_LOW);
    sys_write32(0, base + VIRTIO_MMIO_QUEUE_USED_HIGH);

    sys_write32(1, base + VIRTIO_MMIO_QUEUE_READY);

    /* --- STEP 4: Finish Handshake & Notify --- */
    uint32_t status = sys_read32(base + VIRTIO_MMIO_STATUS);
    sys_write32(status | 4, base + VIRTIO_MMIO_STATUS); 

    LOG_INF("Vring configured. Ringing VCML doorbell...");
    
    sys_write32(0, base + VIRTIO_MMIO_QUEUE_NOTIFY);

    return 0;
}

#define VIRTIO_INPUT_INIT(inst)                                               \
    static struct virtio_input_data input_data_##inst = {                     \
        .mmio_base = DT_REG_ADDR(DT_INST_PARENT(inst)),                       \
    };                                                                        \
    DEVICE_DT_INST_DEFINE(inst, virtio_input_init, NULL,                      \
                          &input_data_##inst, NULL,                           \
                          POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY,            \
                          NULL);

DT_INST_FOREACH_STATUS_OKAY(VIRTIO_INPUT_INIT)