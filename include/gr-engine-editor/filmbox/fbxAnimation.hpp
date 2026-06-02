#pragma once

#include "fbxChannel.hpp"

class FbxAnimation {
public:
    FbxAnimation(const std::string &name = "default", float duration = 0.0f, float tick = 0.0f);

    const std::string &getName() const;

    void setName(const std::string &name);

    const float getDuration() const;

    const float getTick() const;

    template <typename ...Args>
    FbxChannel &addChannel(Args &&...args);

    const std::vector<FbxChannel> &getChannels() const;

    void setChannel(const std::vector<FbxChannel> &channels);

    FbxChannel *getChannel(int index);

private:
    std::string m_name;

    float m_duration;

    float m_tick;

    std::vector<FbxChannel> m_channels;
};

template <typename... Args>
inline FbxChannel &FbxAnimation::addChannel(Args &&...args) {
    return m_channels.emplace_back(std::forward<Args>(args)...);
}
