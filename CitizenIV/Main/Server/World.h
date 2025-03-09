#pragma once

#include "../../sdk/SDK.h"
#include <vector>
#include <mutex>

namespace CustomMP {
    // Weather types
    enum class Weather {
        CLEAR,
        CLOUDY,
        RAINY,
        THUNDERSTORM,
        FOGGY,
        SNOWY
    };

    class World {
    private:
        Weather m_CurrentWeather;
        int m_CurrentHour;
        int m_CurrentMinute;
        float m_TimeScale;
        bool m_WeatherTransition;
        Weather m_NextWeather;
        float m_TransitionProgress;

        std::mutex m_WorldMutex;

    public:
        World();
        ~World();

        void Update();

        // Weather control
        Weather GetWeather() const { return m_CurrentWeather; }
        void SetWeather(Weather weather);
        void TransitionWeather(Weather targetWeather, float transitionTime);

        // Time control
        void GetTime(int& hour, int& minute) const;
        void SetTime(int hour, int minute);

        float GetTimeScale() const { return m_TimeScale; }
        void SetTimeScale(float scale) { m_TimeScale = scale; }

        // Serialize world data for network transmission
        std::vector<uint8_t> Serialize() const;

        // Deserialize world data from network transmission
        void Deserialize(const std::vector<uint8_t>& data);
    };
}