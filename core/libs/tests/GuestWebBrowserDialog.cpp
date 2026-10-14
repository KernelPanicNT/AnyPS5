#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

extern "C" {
int APS5_VABI sceWebBrowserDialogInitialize(void);
int APS5_VABI sceWebBrowserDialogTerminate(void);
int APS5_VABI sceWebBrowserDialogOpen(const void* param);
int APS5_VABI sceWebBrowserDialogClose(void);
int APS5_VABI sceWebBrowserDialogGetStatus(void);
int APS5_VABI sceWebBrowserDialogUpdateStatus(void);
int APS5_VABI sceWebBrowserDialogGetResult(void* result);
int APS5_VABI sceWebBrowserDialogOpenForPredeterminedContent(const void* param, const void* contentParam);
}

namespace {

constexpr int COMMON_DIALOG_STATUS_NONE = 0;
constexpr int COMMON_DIALOG_STATUS_INITIALIZED = 1;
constexpr int COMMON_DIALOG_STATUS_FINISHED = 3;
constexpr int COMMON_DIALOG_RESULT_USER_CANCELED = 1;
constexpr int COMMON_DIALOG_ERROR_NOT_INITIALIZED = static_cast<int>(0x80B80003u);
constexpr int COMMON_DIALOG_ERROR_NOT_FINISHED = static_cast<int>(0x80B80005u);
constexpr int COMMON_DIALOG_ERROR_ARG_NULL = static_cast<int>(0x80B8000Du);

void Require(bool value) { if (!value) std::abort(); }

struct PredeterminedContentParam {
    std::uint64_t size;
    const char* domain[20];
    std::uint8_t reserved[256];
};
static_assert(sizeof(PredeterminedContentParam) == 424);

template <typename TAction>
bool ThrowsInvalidArgument(TAction action) {
    try {
        action();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

void CheckPredeterminedContent(std::uint8_t* param) {
    PredeterminedContentParam content{};
    content.size = sizeof(content);
    content.domain[0] = "https://example.com";
    std::int32_t result = 0x5a5a5a5a;

    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &content) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(nullptr, &content) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(ThrowsInvalidArgument([&] { sceWebBrowserDialogOpenForPredeterminedContent(param, nullptr); }));
    content.size = sizeof(content) - 8;
    Require(ThrowsInvalidArgument([&] { sceWebBrowserDialogOpenForPredeterminedContent(param, &content); }));
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    content.size = sizeof(content);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &content) == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogGetResult(&result) == 0 && result == COMMON_DIALOG_RESULT_USER_CANCELED);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &content) == 0);
    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);
}

}

int main() {
    std::uint8_t param[64] = {};
    std::int32_t result = 0x5a5a5a5a;

    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);
    Require(sceWebBrowserDialogClose() == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogOpen(param) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogGetResult(&result) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(result == 0x5a5a5a5a);

    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogUpdateStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogGetResult(&result) == COMMON_DIALOG_ERROR_NOT_FINISHED);
    Require(result == 0x5a5a5a5a);

    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);

    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogOpen(nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogOpen(param) == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogUpdateStatus() == COMMON_DIALOG_STATUS_FINISHED);

    Require(sceWebBrowserDialogGetResult(nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogGetResult(&result) == 0);
    Require(result == COMMON_DIALOG_RESULT_USER_CANCELED);

    Require(sceWebBrowserDialogClose() == 0);
    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);

    CheckPredeterminedContent(param);
    return 0;
}
