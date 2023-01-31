#pragma once

#include <functional>
#include <system_error>
#include <windows.h>
#include <iostream>

class RegEdit {
public:
    enum class Key {
        KEY_CLASSES_ROOT,
        KEY_CURRENT_CONFIG,
        KEY_CURRENT_USER,
        KEY_LOCAL_MACHINE,
        KEY_USERS
    };

private:
    HKEY keyHandle_{};
    bool isOpened_{ false };

    HKEY extractKeyHandle(Key key);

public:
    struct QueriedValue {
        std::size_t size{};
        unsigned long type{};
    };

    RegEdit() = default;
    RegEdit(Key key, const std::string& catalog);
    ~RegEdit();

    /*!
     * \brief Create, or open if exists, registry key
     * \param key Handle of registry key
     * \param catalog Name of subkey
     * \return Nothing
     * \exception std::system_error If error occurred
     */
    void createCatalog(Key key, const std::string& catalog);

    void queryValue(const std::string& key,
        const std::function<void* (const QueriedValue&)>& callback);

    /*!
     * \brief Get string value from registry
     * \param key Key in registry
     * \return Value from registry
     * \exception std::system_error If error occurred
     */
    const std::string getString(const std::string& key);

    /*!
     * \brief Get unsigned long value from registry
     * \param key Key in registry
     * \return Value from registry
     * \exception std::system_error If error occurred
     */
    const unsigned long getUlong(const std::string& key);

    template <typename _Type> const _Type getBinary(const std::string& key) {
        _Type value{};

        this->queryValue<_Type>(key, value, [&](const QueriedValue& queriedValue) {
            if (queriedValue.type != REG_BINARY) {
                throw std::system_error{ ERROR_DATATYPE_MISMATCH, std::system_category(),
                                        "Readed data type mismatch" };
            }
        return false;
            });

        return value;
    }

    /*!
     * \brief Set string value in registry
     * \param key Key in registry
     * \return Nothing
     * \exception std::system_error If error occurred
     */
    void setString(const std::string& key, const std::string& value);

    /*!
     * \brief Set unsigned long value in registry
     * \param key Key in registry
     * \return Nothing
     * \exception std::system_error If error occurred
     */
    void setUlong(const std::string& key, const unsigned long value);

    template <typename _Type>
    void setBinary(const std::string& key, const _Type& value) {
        const LPBYTE valueConv{ (const LPBYTE)&value };
        const DWORD size{ sizeof(_Type) };

        auto result{ RegSetValueExA(keyHandle_, key.c_str(), 0, REG_BINARY,
                                   valueConv, size) };
        if (result != ERROR_SUCCESS) {
            throw std::system_error{ result, std::system_category() };
        }
    }

    /*!
     * \brief Remove value from registry
     * \param key Key in registry
     * \return Nothing
     * \exception std::system_error If error occurred
     */
    void removeValue(const std::string& key);

    /*!
     * \brief Remove key from registry
     * \param key Handle of registry key
     * \param catalog Name of subkey
     * \return Nothing
     * \exception std::system_error If error occurred
     */
    void removeCatalog(Key key, const std::string& catalog);

    /*!
     * \brief Closes a handle to registry key
     * \return Nothing
     * \exception std::system_error If error occurred
     */
    void closeCatalog();
};