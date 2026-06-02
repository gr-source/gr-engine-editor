#include "filmbox/fbxAnimation.hpp"
#include "filmbox/fbxChannel.hpp"

FbxAnimation::FbxAnimation(const std::string &name, float duration, float tick) : m_name(name), m_duration(duration), m_tick(tick) {}

const std::string &FbxAnimation::getName() const {
    return m_name;
}

void FbxAnimation::setName(const std::string &name)
{
    m_name = name;
}

const float FbxAnimation::getDuration() const {
    return m_duration;
}

const float FbxAnimation::getTick() const {
    return m_tick;
}

const std::vector<FbxChannel> &FbxAnimation::getChannels() const {
    return m_channels;
}

void FbxAnimation::setChannel(const std::vector<FbxChannel> &channels)
{
    m_channels = channels;
}

FbxChannel *FbxAnimation::getChannel(int index)
{
    return &m_channels[index];
}




