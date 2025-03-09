#include "World.h"
#include <iostream>
#include <ctime>

namespace CustomMP {
    World::World()
        : m_CurrentWeather(Weather::CLEAR)
        , m_CurrentHour(12)
        , m_CurrentMinute(0)
        , m_TimeScale(1.0f)
        , m_WeatherTransition(false)
        , m_NextWeather(Weather::CLEAR)
        , m_TransitionProgress(0.0f)
    {
    }

    World::~World() {
    }

    void World::Update() {
        std::lock_guard<std::mutex> lock(m_WorldMutex);

        // Update game time
        static auto lastTimeUpdate = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimeUpdate).count();

        if (elapsed >= 1000) { // Update every second
            lastTimeUpdate = now;

            // Update time based on time scale
            float minutesToAdd = 1.0f * m_TimeScale;
            m_CurrentMinute += static_cast<int>(minutesToAdd);

            if (m_CurrentMinute >= 60) {
                m_CurrentHour += m_CurrentMinute / 60;
                m_CurrentMinute %= 60;

                if (m_CurrentHour >= 24) {
                    m_CurrentHour %= 24;
                }

                std::cout << "Game time: " << m_CurrentHour << ":" << (m_CurrentMinute < 10 ? "0" : "") << m_CurrentMinute << std::endl;
            }
        }

        // Update weather transition if active
        if (m_WeatherTransition) {
            static auto lastWeatherUpdate = std::chrono::system_clock::now();
            auto weatherElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWeatherUpdate).count();

            if (weatherElapsed >= 100) { // Update 10 times per second
                lastWeatherUpdate = now;

                m_TransitionProgress += 0.01f; // 1% progress per update

                if (m_TransitionProgress >= 1.0f) {
                    // Transition complete
                    m_CurrentWeather = m_NextWeather;
                    m_WeatherTransition = false;
                    m_TransitionProgress = 0.0f;

                    std::cout << "Weather transition complete: " << static_cast<int>(m_CurrentWeather) << std::endl;
                }
            }
        }
    }

    void World::SetWeather(Weather weather) {
        std::lock_guard<std::mutex> lock(m_WorldMutex);

        if (m_CurrentWeather != weather) {
            m_CurrentWeather = weather;
            m_WeatherTransition = false;
            m_TransitionProgress = 0.0f;

            std::cout << "Weather changed to: " << static_cast<int>(weather) << std::endl;
        }
    }

    void World::TransitionWeather(Weather targetWeather, float transitionTime) {
        std::lock_guard<std::mutex> lock(m_WorldMutex);

        if (m_CurrentWeather != targetWeather) {
            m_WeatherTransition = true;
            m_NextWeather = targetWeather;
            m_TransitionProgress = 0.0f;

            std::cout << "Starting weather transition to: " << static_cast<int>(targetWeather)
                << " (time: " << transitionTime << " seconds)" << std::endl;
        }
    }

    void World::GetTime(int& hour, int& minute) const {
        hour = m_CurrentHour;
        minute = m_CurrentMinute;
    }

    void World::SetTime(int hour, int minute) {
        std::lock_guard<std::mutex> lock(m_WorldMutex);

        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
            m_CurrentHour = hour;
            m_CurrentMinute = minute;

            std::cout << "Game time set to: " << hour << ":" << (minute < 10 ? "0" : "") << minute << std::endl;
        }
    }

    std::vector<uint8_t> World::Serialize() const {
        std::vector<uint8_t> data;

        // Add weather (1 byte)
        data.push_back(static_cast<uint8_t>(m_CurrentWeather));

        // Add time (2 bytes)
        data.push_back(static_cast<uint8_t>(m_CurrentHour));
        data.push_back(static_cast<uint8_t>(m_CurrentMinute));

        // Add time scale (4 bytes)
        float timeScale = m_TimeScale;
        uint8_t* timeScaleBytes = reinterpret_cast<uint8_t*>(&timeScale);
        data.insert(data.end(), timeScaleBytes, timeScaleBytes + sizeof(float));

        // Add weather transition state (6 bytes)
        data.push_back(m_WeatherTransition ? 1 : 0);
        data.push_back(static_cast<uint8_t>(m_NextWeather));

        // Add transition progress (4 bytes)
        float progress = m_TransitionProgress; 
        uint8_t* progressBytes = reinterpret_cast<uint8_t*>(&progress);
        data.insert(data.end(), progressBytes, progressBytes + sizeof(float));

        return data;
    }

    void World::Deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < 13) { // Minimum size for world data
            return;
        }

        size_t offset = 0;

        // Read weather (1 byte)
        m_CurrentWeather = static_cast<Weather>(data[offset++]);

        // Read time (2 bytes)
        m_CurrentHour = data[offset++];
        m_CurrentMinute = data[offset++];

        // Read time scale (4 bytes)
        memcpy(&m_TimeScale, &data[offset], sizeof(float));
        offset += sizeof(float);

        // Read weather transition state (6 bytes)
        m_WeatherTransition = data[offset++] != 0;
        m_NextWeather = static_cast<Weather>(data[offset++]);

        memcpy(&m_TransitionProgress, &data[offset], sizeof(float));
    }
}