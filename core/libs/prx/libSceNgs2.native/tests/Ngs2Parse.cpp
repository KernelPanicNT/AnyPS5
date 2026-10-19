#include "Ngs2Test.hpp"

#include "prx/libc/include/General.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

static void Put16(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>(value));
    out.push_back(static_cast<std::uint8_t>(value >> 8));
}

static void Put32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    Put16(out, value & 0xffff);
    Put16(out, value >> 16);
}

static void PutTag(std::vector<std::uint8_t>& out, const char* tag) {
    out.insert(out.end(), tag, tag + 4);
}

static std::vector<std::uint8_t> PcmFile(std::uint32_t channels, std::uint32_t sampleRate, std::uint32_t bits, std::uint32_t frames) {
    const std::uint32_t frameBytes = channels * bits / 8;
    std::vector<std::uint8_t> file;
    PutTag(file, "RIFF");
    Put32(file, 36 + frames * frameBytes);
    PutTag(file, "WAVE");
    PutTag(file, "fmt ");
    Put32(file, 16);
    Put16(file, 1);
    Put16(file, channels);
    Put32(file, sampleRate);
    Put32(file, sampleRate * frameBytes);
    Put16(file, frameBytes);
    Put16(file, bits);
    PutTag(file, "data");
    Put32(file, frames * frameBytes);
    file.resize(file.size() + frames * frameBytes, 0x11);
    return file;
}

static void TestParsePcm() {
    const auto file = PcmFile(2, 44100, 16, 100);
    Ngs2WaveformInfo info{};
    Require(sceNgs2ParseWaveformData(file.data(), file.size(), &info) == SCE_NGS2_OK);
    Require(info.format.waveform_type == SCE_NGS2_WAVEFORM_TYPE_PCM_I16L && info.format.num_channels == 2 && info.format.sample_rate == 44100);
    Require(info.data_offset == 44 && info.data_size == 400 && info.num_samples == 100);
    Require(info.num_blocks == 1 && info.block[0].data_offset == 44 && info.block[0].data_size == 400 && info.block[0].num_samples == 100);

    auto truncated = file;
    truncated.resize(44 + 202);
    Require(sceNgs2ParseWaveformData(truncated.data(), truncated.size(), &info) == SCE_NGS2_ERROR_INVALID_WAVEFORM_DATA);

    auto partialFrame = PcmFile(2, 44100, 16, 100);
    partialFrame.resize(44 + 402, 0x11);
    const std::uint32_t partialSize = 402;
    std::memcpy(partialFrame.data() + 40, &partialSize, sizeof(partialSize));
    Require(sceNgs2ParseWaveformData(partialFrame.data(), partialFrame.size(), &info) == SCE_NGS2_ERROR_INVALID_WAVEFORM_DATA);

    const auto eightBit = PcmFile(1, 44100, 8, 10);
    Require(sceNgs2ParseWaveformData(eightBit.data(), eightBit.size(), &info) == SCE_NGS2_ERROR_INVALID_WAVEFORM_FORMAT);
    const auto noChannels = PcmFile(0, 44100, 16, 10);
    Require(sceNgs2ParseWaveformData(noChannels.data(), noChannels.size(), &info) == SCE_NGS2_ERROR_INVALID_WAVEFORM_FORMAT);
}

static std::vector<std::uint8_t> LoopedPcmFile(std::uint32_t samplerBytes, std::uint32_t loops, std::uint32_t type, std::uint32_t start, std::uint32_t end) {
    auto file = PcmFile(2, 44100, 16, 100);
    PutTag(file, "smpl");
    Put32(file, samplerBytes);
    std::vector<std::uint8_t> sampler;
    for (int i = 0; i < 7; ++i) Put32(sampler, 0);
    Put32(sampler, loops);
    Put32(sampler, 0);
    for (int i = 0; i < 2; ++i) {
        Put32(sampler, 0);
        Put32(sampler, type);
        Put32(sampler, start + 10 * i);
        Put32(sampler, end + 10 * i);
        Put32(sampler, 0);
        Put32(sampler, 3);
    }
    sampler.resize(samplerBytes);
    file.insert(file.end(), sampler.begin(), sampler.end());
    const auto riffSize = static_cast<std::uint32_t>(file.size() - 8);
    std::memcpy(file.data() + 4, &riffSize, sizeof(riffSize));
    return file;
}

static void RequireBlock(const Ngs2WaveformBlock& block, std::uint64_t offset, std::uint64_t size, std::uint32_t samples, std::uint32_t repeats) {
    Require(block.data_offset == offset && block.data_size == size && block.num_samples == samples && block.num_repeats == repeats && block.num_skip_samples == 0);
}

static bool ParseThrows(const std::vector<std::uint8_t>& file) {
    Ngs2WaveformInfo info{};
    try {
        static_cast<void>(sceNgs2ParseWaveformData(file.data(), file.size(), &info));
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

static void TestParsePcmLoop() {
    Ngs2WaveformInfo info{};
    const auto looped = LoopedPcmFile(60, 1, 0, 20, 59);
    Require(sceNgs2ParseWaveformData(looped.data(), looped.size(), &info) == SCE_NGS2_OK);
    Require(info.format.waveform_type == SCE_NGS2_WAVEFORM_TYPE_PCM_I16L && info.data_offset == 44 && info.data_size == 400 && info.num_samples == 100);
    Require(info.loop_begin_position == 20 && info.loop_end_position == 60 && info.num_blocks == 3);
    RequireBlock(info.block[0], 44, 80, 20, 0);
    RequireBlock(info.block[1], 124, 160, 40, UINT32_MAX);
    RequireBlock(info.block[2], 284, 160, 40, 0);

    for (const auto& file : {LoopedPcmFile(84, 2, 1, 20, 59), LoopedPcmFile(52, 1, 0, 20, 59)}) {
        Require(sceNgs2ParseWaveformData(file.data(), file.size(), &info) == SCE_NGS2_OK);
        Require(info.loop_begin_position == 20 && info.loop_end_position == 60 && info.num_blocks == 3);
        RequireBlock(info.block[1], 124, 160, 40, UINT32_MAX);
    }

    const auto whole = LoopedPcmFile(60, 1, 0, 0, 99);
    Require(sceNgs2ParseWaveformData(whole.data(), whole.size(), &info) == SCE_NGS2_OK);
    Require(info.loop_begin_position == 0 && info.loop_end_position == 100 && info.num_blocks == 1);
    RequireBlock(info.block[0], 44, 400, 100, UINT32_MAX);

    const auto tail = LoopedPcmFile(60, 1, 0, 0, 49);
    Require(sceNgs2ParseWaveformData(tail.data(), tail.size(), &info) == SCE_NGS2_OK);
    Require(info.num_blocks == 2);
    RequireBlock(info.block[0], 44, 200, 50, UINT32_MAX);
    RequireBlock(info.block[1], 244, 200, 50, 0);

    const auto backwards = LoopedPcmFile(60, 1, 0, 50, 10);
    Require(sceNgs2ParseWaveformData(backwards.data(), backwards.size(), &info) == SCE_NGS2_OK);
    Require(info.loop_begin_position == 50 && info.loop_end_position == 11 && info.num_blocks == 1);
    RequireBlock(info.block[0], 44, 400, 100, 0);

    const auto wrapped = LoopedPcmFile(60, 1, 0, 50, 0xffffffffu);
    Require(sceNgs2ParseWaveformData(wrapped.data(), wrapped.size(), &info) == SCE_NGS2_OK);
    Require(info.loop_begin_position == 50 && info.loop_end_position == 0 && info.num_blocks == 1);
    RequireBlock(info.block[0], 44, 400, 100, 0);

    const auto none = LoopedPcmFile(60, 0, 0, 20, 59);
    Require(sceNgs2ParseWaveformData(none.data(), none.size(), &info) == SCE_NGS2_OK);
    Require(info.loop_begin_position == 0 && info.loop_end_position == 0 && info.num_blocks == 1);
    RequireBlock(info.block[0], 44, 400, 100, 0);

    Require(ParseThrows(LoopedPcmFile(60, 1, 0, 20, 100)));
    Require(ParseThrows(LoopedPcmFile(48, 1, 0, 20, 59)));
    Require(ParseThrows(LoopedPcmFile(28, 1, 0, 20, 59)));
}

static void TestParseFile() {
    const auto wave = PcmFile(1, 22050, 16, 64);
    const char* const guestPath = "/aps5_ngs2_parse_file.wav";
    const auto path = ResolvePath_nid_no_patch(guestPath);
    {
        std::ofstream out(path, std::ios::binary);
        const char pad[3]{};
        out.write(pad, sizeof(pad));
        out.write(reinterpret_cast<const char*>(wave.data()), static_cast<std::streamsize>(wave.size()));
    }
    Ngs2WaveformInfo info{};
    Require(sceNgs2ParseWaveformFile(guestPath, 3, nullptr) == SCE_NGS2_ERROR_INVALID_OUT_ADDRESS);
    Require(sceNgs2ParseWaveformFile(nullptr, 0, &info) == SCE_NGS2_ERROR_INVALID_WAVEFORM_DATA);
    Require(sceNgs2ParseWaveformFile(guestPath, 3, &info) == SCE_NGS2_OK);
    Require(info.format.num_channels == 1 && info.format.sample_rate == 22050 && info.num_samples == 64);
    Require(info.data_offset == 3 + 44 && info.block[0].data_offset == 3 + 44 && info.data_size == 128);
    Require(sceNgs2ParseWaveformFile(guestPath, static_cast<std::uint32_t>(wave.size()) + 4, &info) == SCE_NGS2_ERROR_INVALID_WAVEFORM_DATA);
    Require(sceNgs2ParseWaveformFile(guestPath, 0, &info) == SCE_NGS2_ERROR_UNKNOWN_WAVEFORM_FORMAT);
    std::filesystem::remove(path);
}

static void TestParseFileBank() {
    const auto first = PcmFile(2, 48000, 16, 300);
    auto second = PcmFile(1, 32000, 16, 50);
    PutTag(second, "LIST");
    Put32(second, 4);
    PutTag(second, "INFO");
    const std::uint32_t riffSize = static_cast<std::uint32_t>(second.size() - 8);
    std::memcpy(second.data() + 4, &riffSize, sizeof(riffSize));
    const char* const guestPath = "/aps5_ngs2_parse_bank.bin";
    const auto path = ResolvePath_nid_no_patch(guestPath);
    {
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(first.data()), static_cast<std::streamsize>(first.size()));
        out.write(reinterpret_cast<const char*>(second.data()), static_cast<std::streamsize>(second.size()));
    }
    Ngs2WaveformInfo info{};
    Require(sceNgs2ParseWaveformFile(guestPath, 0, &info) == SCE_NGS2_OK);
    Require(info.format.num_channels == 2 && info.format.sample_rate == 48000 && info.num_samples == 300);
    Require(info.data_offset == 44 && info.data_size == 1200 && info.block[0].data_offset == 44);
    const auto secondOffset = static_cast<std::uint32_t>(first.size());
    Require(sceNgs2ParseWaveformFile(guestPath, secondOffset, &info) == SCE_NGS2_OK);
    Require(info.format.num_channels == 1 && info.format.sample_rate == 32000 && info.num_samples == 50);
    Require(info.data_offset == secondOffset + 44 && info.data_size == 100 && info.block[0].data_offset == secondOffset + 44);
    std::filesystem::remove(path);
}

int main() {
    TestParsePcm();
    TestParsePcmLoop();
    TestParseFile();
    TestParseFileBank();
    return 0;
}
