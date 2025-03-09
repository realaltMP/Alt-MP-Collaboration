#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace CustomMP {
    // Version information
    constexpr const char* SDK_VERSION = "1.0.0";

    // Forward declarations
    class Player;
    class Vehicle;
    class World;
    class NetworkManager;

    // Vector3 structure for positions
    struct Vector3 {
        float x, y, z;

        Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    };

    // Quaternion structure for rotations
    struct Quaternion {
        float x, y, z, w;

        Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
        Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    };

    // Entity base class
    class Entity {
    protected:
        uint32_t m_ID;
        Vector3 m_Position;
        Quaternion m_Rotation;
        uint32_t m_Model;

    public:
        Entity(uint32_t id, uint32_t model) : m_ID(id), m_Model(model) {}
        virtual ~Entity() = default;

        uint32_t GetID() const { return m_ID; }
        uint32_t GetModel() const { return m_Model; }

        const Vector3& GetPosition() const { return m_Position; }
        void SetPosition(const Vector3& position) { m_Position = position; }

        const Quaternion& GetRotation() const { return m_Rotation; }
        void SetRotation(const Quaternion& rotation) { m_Rotation = rotation; }
    };
}