#include "fault_injection.h"

/*
 * FaultConfig:
 * Lưu cấu hình và trạng thái của một fault point.
 *
 * enabled:
 *   true  -> Fault injection đang được bật.
 *   false -> Fault injection đang bị tắt.
 *
 * call_count:
 *   Đếm số lần chương trình kiểm tra fault point này
 *   thông qua fi_should_fail().
 *
 * fail_on_call:
 *   0 -> Khi enabled, mọi lần gọi đều tạo lỗi.
 *   N -> Chỉ tạo lỗi ở lần gọi thứ N.
 */
typedef struct
{
    bool enabled;
    unsigned int call_count;
    unsigned int fail_on_call;
} FaultConfig;

static FaultConfig faults[FI_COUNT];

static bool fi_valid_id(FaultId id)
{
    return id >= FI_ACCOUNT_READ && id < FI_COUNT;
}

bool fi_set(FaultId id, bool enable)
{
    if (!fi_valid_id(id))
        return false;

    faults[id].enabled = enable;

    if (!enable)
    {
        /* reset counter when fault disable */
        faults[id].call_count = 0;

        /* clear fault happen when nth call. */
        faults[id].fail_on_call = 0;
    }

    return true;
}

bool fi_fail_on_nth_call(FaultId id, unsigned int n)
{
    if (!fi_valid_id(id) || n == 0)
        return false;

    faults[id].enabled = true;
    faults[id].call_count = 0;
    faults[id].fail_on_call = n;

    return true;
}

bool fi_should_fail(FaultId id)
{
    if (!fi_valid_id(id))
        return false;

    FaultConfig *cfg = &faults[id];

    /* Fault point đang tắt: không tạo lỗi. */
    if (!cfg->enabled)
        return false;

    /* Ghi nhận một lần kiểm tra fault point. */
    cfg->call_count++;

    /*
     * Chế độ lỗi tại lần gọi thứ N.
     * Chỉ lần gọi bằng fail_on_call mới tạo lỗi.
     */
    if (cfg->fail_on_call != 0 &&
        cfg->call_count == cfg->fail_on_call)
    {
        return true;
    }

    /*
     * Chế độ fail_on_call == 0:
     * Tạo lỗi ở mọi lần kiểm tra.
     */
    if (cfg->fail_on_call == 0)
        return true;

    return false;
}

void fi_reset(void)
{
    for (int i = 0; i < FI_COUNT; i++)
    {
        faults[i].enabled = false;
        faults[i].call_count = 0;
        faults[i].fail_on_call = 0;
    }
}